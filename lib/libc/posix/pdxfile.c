/****************************************************************************
 *
 *      $Id: pdxfile.c,v 1.4 2002/05/31 07:43:53 danielp Exp $
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
 * $Id: pdxfile.c,v 1.4 2002/05/31 07:43:53 danielp Exp $
 *
 * Implement write to PDX files!
 */

#include <mungi.h>
#include <assert.h>
#include <mlib.h>
#include <string.h>

#include "oftab.h"


/* FIXME! */
#include "../../../apps/init/include/filedesc.h"

#define BUF_SIZE 4096

/*FIXME:  create buffers on demand for files.
  at later stage add seprate read & write buffers not a single for both.
*/
static int
check_buffer(pdx_fdesc_t *buf){

		cap_t cap ;
		int r ;

		if (buf->buffer.address==NULL) {
				/* create a buffer to transfer */
				r = create_simple_object(BUF_SIZE, &cap);
				assert(r == 0);
				assert(cap.address != NULL);
				buf->buffer = cap;
		}
		
		return 0;
}


int
_pdxfile_write(_open_file *file, const void *buf, size_t nbyte)
{
	cap_t dummy;
	int r;
	pdx_buf_t *sendbuf;
	pdx_fdesc_t *tty;

	tty = &file->data.pdx;

   
	check_buffer(tty);

	/* setup the buffer */
	sendbuf = tty->buffer.address;

	/* setup the buffer */
	sendbuf->descriptor = tty->write_desc;
	sendbuf->size = nbyte;
	memcpy( &sendbuf->buf[0], buf, nbyte );

	/* make the call */
	r = PdxCall( tty->write.address, tty->buffer, &dummy, PD_EMPTY );
	assert( r == 0 );
	return nbyte;  /* FIXME: return something better */
}

int
_pdxfile_read( _open_file *file, void *buf, size_t nbyte )
{
	cap_t dummy;
	int r;
	pdx_buf_t *sendbuf;
	pdx_fdesc_t *tty;

	tty = &file->data.pdx;	
	
	check_buffer( tty);

	/* setup the buffer */
	sendbuf = tty->buffer.address;

	/* setup the buffer */
	sendbuf->descriptor = tty->read_desc;
	sendbuf->size = nbyte;

	/* make the call */
	r = PdxCall( tty->read.address, tty->buffer, &dummy, PD_EMPTY );
	assert( r == 0 );

	/* FIXME: check return code/size etc */
	/* Copy stuff back out */
	memcpy( buf, &sendbuf->buf[0], sendbuf->size );

	nbyte = sendbuf->size;
	return nbyte;
}
