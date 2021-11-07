/****************************************************************************
 *
 *      $Id: main.c,v 1.7 2002/07/22 10:17:33 cgray Exp $
 *      Copyright (C) 2002 Operating Systems Research Group, UNSW, Australia.
 *
 *      This file is part of the Mungi operating system distribution.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *	version 2 as published by the Free Software Foundation.
 *	A copy of this license is included in the top level directory of 
 *	the Mungi distribution.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 ****************************************************************************/

/*
 *  Serial TTY/stream PDX driver module
 */

#include <mungi.h>


/* so we can talk L4 */
#include "syscalls.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallbits.h"

#include <mlib.h>

/* FIXME: mmm... seedy. Perhaps we should
 *        fix the make files? :)
 */
#include "../include/filedesc.h"

extern void _mungi_lib_init(void);


//cap_t client_clist;  /* for storing client clist items */
cap_t *client_cap =NULL;


cap_t serial_pdx_read( cap_t param );
cap_t serial_pdx_write( cap_t param );
cap_t serial_pdx_activate( cap_t param );


/* serial/pdx buffer & semaphores */
/**/

//#define SERIAL_BUF_SIZE 19        /* MS-DOS compatable */
#define SERIAL_BUF_SIZE 42        /* what more magic do you need ? */
static char serial_buf[SERIAL_BUF_SIZE];
static int  write_pos = 0;
static int  read_pos  = 0;
static int  serial_blocked = 0;   /* is PDX blocked? */
static char serial_lock;          /* sem. lock for these vars */
static char serial_block;         /* sem. for pdx to block on */
static void serial_loop(void);    /* main loop */

/* FIXME: this should be in a header */
int serial_main( void* param );
void UserPrint( char *fmt, ... );

cap_t 
serial_pdx_write( cap_t param )
{
	cap_t ret;
	pdx_buf_t *buf;

	/* FIXME: check descriptor?? */

	/* insert the cap into a clist */
//	add_to_clist_pos( param, client_clist.address, 0 );
	assert( client_cap != NULL );
	*client_cap = param ;

	/* FIXME: check address */
	assert( param.address != NULL );
	buf = (pdx_buf_t*) param.address;

	/* print it */
	assert( buf->buf != NULL );
	buf->buf[buf->size] = '\0';    /* terminate the string */
	SeedyPrint( &buf->buf[0] );
	/* FIXME: sane return value */
	ret = (cap_t){0,0};
	return ret;
}

cap_t 
serial_pdx_read( cap_t param )
{
	cap_t ret;
	pdx_buf_t *buf;
	int r, start, len, total_len = 0;

	/* FIXME: check descriptor?? */

	/* insert the cap into a clist */
//	add_to_clist_pos( param, client_clist.address, 0 );
	*client_cap = param ;
	/* FIXME: check address */
	buf = (pdx_buf_t*) param.address;

	/* get the lock */
	r = SemWait( &serial_lock );
	assert( r == 0 );

	/* if there's nothing to read, go to sleep */
	if( read_pos == write_pos )
	{
		/* mark us as blocked */
		serial_blocked = 1;

		/* release the old lock */
		r = SemSignal( &serial_lock );
		assert( r == 0 );

		/* wait on the 'block' */
		r = SemWait( &serial_block );
		assert( r == 0 );

		/* re-get the lock */
		r = SemWait( &serial_lock );
		assert( r == 0 );

		/* sanity */
		assert( read_pos != write_pos );
	}

	/** do the copy in two parts! **/

	/* (1) copy from the read point to the end of the buffer */
	start = read_pos;
	len = SERIAL_BUF_SIZE - read_pos;

	/* account for write pos */
	if( write_pos > read_pos )
		len = write_pos - read_pos;
		
	/* account for small buffer */
	if( len > buf->size )
		len = buf->size;

	memcpy( &buf->buf[0], &serial_buf[start], len );
	read_pos += len;
	total_len = len;


	/* BUGFIX: if we've hit the end of the buffer, readpos = 0! */
	if( read_pos >= SERIAL_BUF_SIZE )
		read_pos = 0;



	/* (2) copy from the start point to the write point */
	if( write_pos < read_pos
	    && buf->size > total_len )
	{
		start = 0;
		len = write_pos;

		if( len + total_len > buf->size )
			len = buf->size - total_len;

		memcpy( &buf->buf[total_len], &serial_buf[0], len );
		read_pos += len;
		total_len += len;
	}

	r = SemSignal( &serial_lock );
	assert( r == 0 );

	/* return size */
	buf->size = total_len;


	/* FIXME: sane return value? */
	ret = (cap_t){0,0};
	return ret;
}

cap_t 
serial_pdx_activate( cap_t param )
{
	cap_t ret;
	ret = (cap_t){0,0};

	return ret;
}

/* function to register as a character reciever */
extern l4_threadid_t mm_tid; /* same as used by SeedyPrint */
static void
serial_register(void)
{
  syscall_t sysmsg;
  l4_msgdope_t result;
  int r;

  sysmsg.msg.reg[0] = 0xfeedb17e;   /* magic number */
  sysmsg.syscall.number = 0;
  
  r = l4_ipc_call(mm_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		  &sysmsg.msg, L4_IPC_NEVER, &result);
  assert( r == 0 );

}

/* wait for a character */
static int
serial_waitchar(void)
{
	syscall_t sysmsg;
	l4_msgdope_t result;
	l4_threadid_t thread;
	int r;
	int c;

	/* FIXME: no open wait! */
	r = l4_ipc_wait( &thread, L4_IPC_SHORT_MSG, &sysmsg.msg, 
			 L4_IPC_NEVER, &result);
	assert( r == 0 );	
	
	c = (char)sysmsg.msg.reg[0];

	/* pre-process \r (enter) into \n (newline) */
	if( c == '\r' )
		c = '\n';
	return c;
}

static void 
serial_loop(void)
{
	int c, r;
	char cbuf[2] = { '\0', '\0' };

	/* register ourself with console in mungi */
	serial_register();

	SeedyPrint( "Serial registered, starting\n" );
	while( 1 )
	{
		/* read a char */
		c = serial_waitchar();

		/* get the lock */
		r = SemWait( &serial_lock );
		assert( r == 0 );

		/* echo the character to the screen */
		cbuf[0] = (char) c;
		SeedyPrint( (char*) &cbuf[0] );


		/* can we add it? */
		if( (write_pos + 1) == read_pos
		    || (read_pos == 0 && write_pos >= (SERIAL_BUF_SIZE-1)) )
		{

			
			/* buffer full - abort! :) */
			SeedyPrint( "\a" ); 

			r = SemSignal( &serial_lock );
			assert( r == 0 );

			continue;
		}

		/* add the char to the buffer */
		serial_buf[write_pos] = (char) c;
		write_pos++;
		if( write_pos >= SERIAL_BUF_SIZE )
			write_pos = 0;
		
		/* is PDX blocked? */
		if( serial_blocked != 0 )
		{
			/* clear it */
			serial_blocked = 0;

			/* signal it */
			r = SemSignal( &serial_block );
			assert( r == 0 );
		}

		/* release the lock */
		r = SemSignal( &serial_lock );
	}

}

static long
mkrandom(void)
{
	static int last_password = 0x37;  /* FIXME FIXME FIXME */
	return last_password++;
}

/* Generate a random password & ObjCrePdx it!*/
static int 
set_pdx(cap_t *cap, clist_t *clist, pdx_t *ept)
{
	int r;

	cap->address = ept;
	cap->passwd  = mkrandom();

        /* FIXME: Are these security labels correct? */        
	r = ObjCrePdx(*cap,clist,0,0,0,1,(pdx_t*)&cap->address);

	return 0;
}

/* entry point for serial driver/TTY.
 * First setup the PDX entry points, then tell that back to init
 */
int 
serial_main( void* param )
{
	int r;
	apddesc_t myapd;
	clist_t *clist;
	cap_t pdx_cap;
	pdx_fdesc_t fdesc;

	/* do library initialisations */
	_mungi_lib_init();

	UserPrint( "tty: Welcome to serial main!\n" );
	UserPrint( "tty: Param: 0x%llx\n", param );

	/* fistly let's setup some PDX entry points */

	/* make a cap */
	pdx_cap = *(ApdLookup( serial_main, M_OWNER )); /* Seedy deref */
	
	/* find the clist */
	r = ApdGet( &myapd );
	assert( r == 0 );
	assert( myapd.n_apd > 1 );

	clist = myapd.clist[1].address;
	assert( clist != NULL );
	
	/* setup the entry points - random passwords & descriptors */
	fdesc.read_desc = mkrandom();
	r = set_pdx(&fdesc.read, clist, (pdx_t *)serial_pdx_read);
	assert(r == 0);

	fdesc.write_desc = mkrandom();
	r = set_pdx(&fdesc.write, clist, (pdx_t *)serial_pdx_write);
	assert(r == 0);

	fdesc.activate_desc = mkrandom();
	r = set_pdx(&fdesc.activate, clist, (pdx_t *)serial_pdx_activate);
	assert(r == 0);


	/* make a clist for user caps */
	client_cap = (cap_t *)&clist[clist->n_caps];

	/* setup the semaphores */
	UserPrint( "tty: Creating semaphores\n" );
	r = SemCreate( &serial_lock, 1, 0 );
	assert( r == 0 );

	r = SemCreate( &serial_block, 0, 0 );
	assert( r == 0 );

	UserPrint( "tty: We seem to have setup PDX ok!\n" );

        /* copy data into other end's buffer */
	memcpy( param, &fdesc, sizeof( fdesc ) );

	/* signal init that we're done */
	SemSignal( param );

	/* start buffering input characters! */
	serial_loop();

	assert( !"shouldn't get here!\n" );

	/* we prolly never want to quit! */
	return -1;
}
