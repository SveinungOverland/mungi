/****************************************************************************
 *
 *      $Id: oftab.h,v 1.4 2002/05/31 07:43:53 danielp Exp $
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
 * Structures for a user-land open file table
 */

/* FIXME: Seedy! */
#include "../../apps/init/include/filedesc.h"
#include <sys/stat.h>

/* file types */
#define _FT_PDX     0   /* PDX stream       */
#define _FT_USTREAM 1   /* user-land stream */
#define _FT_UFILE   2   /* user-land file   */


typedef struct _open_file_S {
	int ref_count;  /* no. file numbers pointing to this descriptor */
	int type;

	union {
		pdx_fdesc_t pdx;
		struct obj_fdesc obj;
	} data;

} _open_file;


/* exported functions... should be used by open() read() dup() etc. */
extern int _oft_set( int desc, _open_file *file );
extern int _oft_remove( int desc );
extern int _oft_add( _open_file *file );
extern int _oft_dup( int desc, int desc2 );
extern _open_file *_oft_lookup( int desc );

extern size_t _oft_serialise  (void * dest);
extern int _oft_deserialise(void * env_data,size_t size );


/* functions to access files */
int _pdxfile_write(_open_file *file, const void *buf, size_t nbyte);
int _pdxfile_read (_open_file *file, void *buf, size_t nbyte);

void* _objfile_create( size_t start_size );
int   _objfile_delete( void* file_addr );
int   _objfile_open  ( void* file_addr, int oflags );
int   _objfile_write ( _open_file *file, const void *buf, size_t nbyte );
int   _objfile_read  ( _open_file *file, void *buf, size_t nbyte );
int   _objfile_close ( _open_file *file );
int   _objfile_stat  ( void *addr, struct stat *buf );



