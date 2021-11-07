/****************************************************************************
 *
 *      $Id: init.c,v 1.9 2002/08/23 08:24:11 cgray Exp $
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
 *   New fandangled PDX-tty enabled version
 */


#include <mungi.h>
#include <assert.h>
#include <mlib.h>


/* FIXME: fix these headers */
#include "include/filedesc.h"
#include "../lib/libc/posix/oftab.h"
#include "../lib/libc/posix/env.h"

#include <stdio.h>
#include <string.h>       

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>  /* for open() and write() */

#include "dit.h"


/* do we have tar?? */
#define HAVE_PORTS 0

/* Naming includes */
/* FIXME: Clean these up! */
#include "../../lib/naming/tree_cmpt/INameTree_io.h"
#include "../../lib/naming/tree_cmpt/tree.h"
#include "../../lib/naming/nstree.h"
#include "../../lib/naming/hmalloc.h"
#include "../../lib/naming/namehash.h"
#include "../../lib/naming/namemap.h"

#include <dirent.h>
#include <stdlib.h>

extern int p9_init(namehash_t *nh, cap_t pbuf);
extern int  p9_mount(char *mount_pt, void *tree, int access_mode);
extern void naming_set_cmpt(cap_t *caplist, int num_caps);
extern void _mungi_lib_init(void);
extern int  serial_main(void *);
static void create_namespace(void* dit);
static void copy_dit_to_file(void* dit, char* name);

static pdx_fdesc_t serial_tty;  /* the serial TTY entry points */
static cap_t pdx_buffer;        /* buffer to talk to the PDX on */
static cap_t pdx_buffer_rw;     /* read/write buffer to pass to the TTY */

/* Create the Serial TTY driver */
static int create_serial(void);
int write_pdx_stream(pdx_fdesc_t *tty, char *msg, size_t size);
int auth_pdx_stream(pdx_fdesc_t *tty);
void set_default_files(pdx_fdesc_t *pdx);

#define TAR "tar"

extern int UserPrint(const char *s, ...);

/********************************************************************
 *
 *   SERIAL SETUP FUNCTIONS
 *
 ********************************************************************/

static int
create_serial(void)
{
	cap_t serial_pd, clist_obj, *dit_cap;
	clist_t *serial_clist;
	apddesc_t *serial_apd;
	int r;

	/* create a clist for the DIT - use this fn as a template! */
	dit_cap = ApdLookup(create_serial, M_READ);
	assert(dit_cap != NULL);
	r = create_clist_and_add(PAGESIZE, &clist_obj);
	assert(r == 0);

	/* let the serial access its code (dit) and its clist */
	serial_clist = (clist_t*) clist_obj.address;
	serial_clist->caps[serial_clist->n_caps++] = *dit_cap;
	serial_clist->caps[serial_clist->n_caps++] = clist_obj;


	/* make an obj for the apd */
	r = create_simple_object(PAGESIZE, &serial_pd);
	assert(r == 0);

	serial_apd = (apddesc_t*)(serial_pd.address);
	serial_apd->n_locked = 0;
	serial_apd->n_apd = 2;
	serial_apd->clist[0].address = NULL;
	serial_apd->clist[1] = clist_obj;
	

	/* create a buffer to transfer */
	r = create_simple_object(PAGESIZE, &pdx_buffer);
	assert(r == 0);
	assert(pdx_buffer.address != NULL);

	/* create a new password */
	pdx_buffer_rw.address = pdx_buffer.address;
	pdx_buffer_rw.passwd  = 12;  /* FIXME: make more random? */
	r = ObjPasswd(pdx_buffer_rw, M_READ | M_WRITE);
	assert(r == 0);
	UserPrint("Init: pdx buffer is: 0x%llx \n",pdx_buffer_rw.address);

	/* create a semaphore so we know when it's done! */
	r = SemCreate(pdx_buffer.address, 0, 0);
	assert(r == 0);

	/* add the cap for the pdx buffer to the serial APD */
	/* FIXME: would could be smarter (eg. R/O buffer)   */
	serial_clist->caps[serial_clist->n_caps++] = pdx_buffer_rw;

	/* create a thread in the APD */
	r = ThreadCreate(serial_main, pdx_buffer.address, NULL, serial_apd);
	assert(r != THREAD_NULL);

	/* block until the other thread is ready... */
	UserPrint("init: Going to sleep...\n");
	SemWait(pdx_buffer.address);

	/* Get the entry point info */
	UserPrint("init: I'm awake again!\n");

	/* FIXME: Do we *really* need *double* copy?? -ceg */
	memcpy(&serial_tty, pdx_buffer.address, sizeof(serial_tty));

	/* set the talk buffer */
	serial_tty.buffer = pdx_buffer_rw;

	if (serial_tty.read.address == NULL)
		return 1;  /* ERROR! */
	else
		return 0;
}


/* FIXME: Move this elsewhere! */
int
write_pdx_stream(pdx_fdesc_t *tty, char *msg, size_t size)
{
	cap_t dummy;
	int r;
	pdx_buf_t *buf;
	
	/* setup the buffer */
	buf = pdx_buffer_rw.address;

	/* setup the buffer */
	buf->descriptor = tty->write_desc;
	buf->size = size;
	memcpy(&buf->buf[0], msg, size);

   
	/* make the call */
	r = PdxCall(tty->write.address, pdx_buffer_rw, &dummy, PD_EMPTY);
	assert(r == 0);

	return 0;  /* FIXME: return something better */
}

/* add caps for the descriptor into our clists */
int
auth_pdx_stream(pdx_fdesc_t *tty)
{
	apddesc_t myapd;
	clist_t *clist;
	int r;

	r = ApdGet(&myapd);
	assert(r == 0);
	assert(myapd.n_apd > 1);

	clist = myapd.clist[1].address;
	assert(clist != NULL);

	if (tty->read.address != NULL)
		add_to_clist(tty->read, clist);

	if (tty->write.address != NULL)
		add_to_clist(tty->write, clist);

	if (tty->activate.address != NULL)
		add_to_clist(tty->activate, clist);

	return 0;
}

/* given the PDX data, make stdin, stdout and stderror */
void
set_default_files(pdx_fdesc_t *pdx)
{
	_open_file arg;

	/* FIXME: clear read/write bits of stdout/in */

	/* stdin */
	arg.type = _FT_PDX;
	memcpy(&arg.data.pdx, pdx, sizeof(*pdx));
	_oft_set(0, &arg);

	/* stdout */
	arg.type = _FT_PDX;
	memcpy(&arg.data.pdx, pdx, sizeof(*pdx));
	_oft_set(1, &arg);

	/* stderr */
	arg.type = _FT_PDX;
	memcpy(&arg.data.pdx, pdx, sizeof(*pdx));
	_oft_set(2, &arg);
}

/********************************************************************
 *
 *   NAMESPACE SETUP FUNCTIONS
 *
 ********************************************************************/
/* copy a whole DIT segment into a file */
static void
copy_dit_to_file(void* dit, char* name)
{
        uintptr_t start, copied;
        unsigned int len, tmp;
	int fd, r;
	
	/* find the dit info */
	r = dit_find_region(dit, name, &start, &len);
	
	if (r != 0){
		printf("WARNING: could not find '%s' in dit!\n", name);
		return;
	}

	/* write it out */
	fd = open(name, O_CREAT | O_WRONLY);

	if (fd == -1) {
		printf("WARNING: could not open file '%s' for writing\n", name);
		return;
	}

	printf("Copying %d bytes of '%s'\n", len, name);

	copied = 0;
	while (copied < len) {	
		tmp = write(fd, (void*)(start+copied), len-copied);
		
		if (tmp <= 0){
			printf("ERROR: write returned %d\n", tmp);
			break;
		}

		copied += tmp;
	}

	close(fd);
}

static void
create_namespace(void* dit)
{
	void *tree;
	cap_t pbuf;
	namehash_t *nh;
	int r;
	environment_t ev;

	/* creat the param buffer */
	r = create_simple_object(2 * PAGESIZE, &pbuf);
	assert(r == 0);

	/* init our namespace */
	nh = nh_init(100, 100);
	assert(nh != NULL);

	r = p9_init(nh, pbuf);
	assert(r == 0);

	/* FIXME: do this in the nm_ library?? */
	printf("init: calling constructor...\n");
	tree = int_create_constructor(CID_CNAMETREE, pbuf, PAGESIZE, &ev);
	printf("init: new tree is 0x%p\n", tree);
	assert(tree != NULL);
	
	r = p9_mount( "/", tree, NMF_READWRITE );

	if (r != NM_OK)
		printf("Error on mount\n");

	/******* NOW WE CAN ADD NAMES! :) *********/

	r = mkdir("dir_1", 0);
	r = mkdir("dir_2", 0);
	r = mkdir("dir_3", 0);	
	
	/* FIXME: use tar! */
        UserPrint( "copying 1...\n" );
	copy_dit_to_file( dit, TAR );
        UserPrint( "copying 2...\n" );
	copy_dit_to_file( dit, "filesystem" );
        UserPrint( "copying 3...\n" );

	/******* LIST NAMES *******/
	printf( "init: Doing an ls!!!\n" );
	{
		DIR *dir;
		struct dirent *ent;
		
		dir = opendir( "." );
		
		if( dir == NULL )
		{
			printf( "error on opendir()\n" );
			return;
		}
		
		ent = readdir( dir );
		while( ent != NULL )
		{
			printf( "%.25s\n", ent->d_name );
			ent = readdir( dir );
		}		
		
		closedir( dir );
	}

}

/********************************************************************
 *
 *   MAIN
 *
 ********************************************************************/

int naming_register_cmpt( cap_t* pdx_objects, int pdx_obj_count );

cap_t elf_load( void* );

int
main(void *param)
{
	int r;
	mthreadid_t thread;
	char *test_msg;
	void *dit = NULL, *entry = NULL;

	/* do library initialisations */
	_mungi_lib_init();

	UserPrint( "init: Welcome to init, creating serial\n" );

	/*********************************
	 *     
	 *             SERIAL
	 *
	 *********************************/

	/* step 1 - create the tty PDX object */
	r = create_serial();
	assert( r == 0 );

	UserPrint( "init: Serial Created, r = %d\n", r );

	/* step 2 - Add the entry points to our clist */
	auth_pdx_stream( &serial_tty );

	/* step 3 - Write something to the serial */
	UserPrint( "init: about to write to TTY!\n" );
	test_msg = "PDX Stream test!\n";
	write_pdx_stream( &serial_tty, test_msg, strlen(test_msg) );

	/* step 4 - setup our stdout */
	set_default_files( &serial_tty );

	/* step 5 - write to it */
	UserPrint( "About to printf\n" );
	printf( "Now I'm doing a printf! (1)\n" );
	printf( "Now I'm doing a printf! (2)\n" );
	printf( "Now I'm doing a printf! (3)\n" );
	printf( "Now I'm doing a printf! (4)\n" );

#if 0
	/* step 5.5 - read something */ 
	printf( "Now reading from the stream...\n" );
	while(1)
	{
		char buf[100];

		fgets( buf, 90, stdin );
		UserPrint( "%s", buf );
	}
#endif

	/***********************************
	 *     
	 *              DIT
	 *
	 ***********************************/
	printf( "Finding DIT from KIP...\n" );
	dit = dit_find_dit( param );

	/***********************************
	 *     
	 *              NAMING
	 *
	 ***********************************/

	{
		cap_t *caplist;
		int caplist_size = 100;
		int r2;

		caplist = malloc(caplist_size * sizeof(cap_t));
		assert(caplist != NULL);

		/* step 6 - create the naming component */
		printf("Creating naming component\n");
		r2 = naming_register_cmpt(caplist, caplist_size);
		assert(r2 != NULL);

		/* step 7 - setup access to naming */
		printf("Setting naming component (%d)\n", r2);
		naming_set_cmpt(caplist, r2);
		free(caplist);

		/* step 8 - create the initial namespace */
		printf("Creating initial namespace\n");
		create_namespace(dit);
		
	}

	/***********************************
	 *     
	 *          OTHER PROCESSES
	 *
	 ***********************************/

	/* step 9 - create the shell */ 
	printf( "init: Mungi Init start\n");

#if HAVE_PORTS == 0

	/* if we don't have ports then we expect to find mishell
	   in the dit (as an executable), so we'll run it */

	/* check for testing code */
	printf( "init: Finding entry point for test...\n" );
	entry = dit_find_entry_pt( dit, "test" );

	if( entry  == NULL )
		printf( "init: POST not found\n" );
	else
	{
		fexec( entry );
		while( 1 )
			ThreadSleep( THREAD_MYSELF, 1000000000 );
	}

	printf( "init: Finding entry point for mishell...\n" );
	entry = dit_find_entry_pt( dit, "mishell" );
	printf( "init: mishell's entry point is %p\n", entry );

	if( entry  == NULL )
		printf( "init: Could not get entry point for mishell!\n" );
	else
		fexec( entry );

#else

	/* if we have the ports, run tar and then run mishell */

	{
		int fd, r;
		struct stat st;
		void *addr;
		cap_t cap;

		printf( "init: trying to run from filesystem\n" );
		fd = open( TAR, O_RDONLY );
		if( fd != -1 )
		{
			/* FIXME: use fstat() when implemented */
			r = stat( TAR, &st );
			assert( r == 0 );
			
			addr = st.st_ino;
			printf( "init: addr is 0x%llx\n" );

			cap = elf_load( addr );
			printf( "init: about to fexec()\n" );
			fexec( cap.address );
			printf( "init: back from fexec()\n" );

			close( fd );
		}
		else
			printf( "init: error opening '%s'\n", TAR );

		/* sleep for a while */
		/* FIXME: fix ThreadSleep */
		printf( "init: going to sleep for tar\n" );
		ThreadSleep( THREAD_MYSELF, 10000000000 );

		{
			DIR *dir;
			struct dirent *ent;
		
			dir = opendir( "." );
		
			if( dir == NULL )
			{
				printf( "error on opendir()\n" );
				return;
			}
		
			ent = readdir( dir );
			while( ent != NULL )
			{
				printf( "%.25s\n", ent->d_name );
				ent = readdir( dir );
			}		
		
			closedir( dir );
		}


		fd = open( "mishell", O_RDONLY );
		if( fd != -1 )
		{
			/* FIXME: use fstat() when implemented */
			r = stat( "mishell", &st );
			assert( r == 0 );
			
			addr = st.st_ino;
			printf( "init: addr is 0x%llx\n" );

			cap = elf_load( addr );
			printf( "init: about to fexec()\n" );
			fexec( cap.address );

			close( fd );
		}
		else
			assert( !"died!" );
	}
#endif
	UserPrint( "init: Going into sleepy loop...\n" );

	while( 1 )
	{ 
		thread = ThreadWait( THREAD_ANY, &r );
		
		UserPrint( "init: thread 0x%llx died (%d)\n", thread, r );
		if( thread == THREAD_ANY )
		{
			UserPrint( "init: Hmmm... nothing to wait for...\n" );
			UserPrint( "init: ... Sleeping\n" ); 
			
			while( 1 )
				ThreadSleep( THREAD_MYSELF, 1000000000 );
		}
	}

        return 0;
}
