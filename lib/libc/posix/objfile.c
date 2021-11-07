/****************************************************************************
 *
 *      $Id: objfile.c,v 1.4 2002/07/22 10:17:54 cgray Exp $
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
 * Implement object files
 */

#include <mungi.h>
#include <assert.h>
#include <string.h>

#include <sys/stat.h>  /* for mode_t */
#include <mlib.h>
#include <fcntl.h>

#include "oftab.h"
#include "objfile.h"

/* FIXME! */
#include "../../../apps/init/include/filedesc.h"


/* to hold file caps */
static clist_t* file_clist = NULL;

/* local statics */
static int is_objfile( void *addr );
static int check_file_cap( cap_t file_cap );

/* see if an address has file magic  (making sure we have access) */
static int
is_objfile( void *addr )
{
	struct objfile *file;
	cap_t *cap_addr, new_cap;
	
	/* ApdLookup() the address */
	cap_addr = ApdLookup( addr, M_READ | M_WRITE );

	/* add to our clist, otherwise */
	if( cap_addr == NULL )
	{
		/* make the cap */
		new_cap.address = addr;
		new_cap.passwd = FILE_PASSWORD;

		/* add it */
		check_file_cap( new_cap );

		/* re-check it */
		cap_addr = ApdLookup( addr, M_READ | M_WRITE );

		/* bad address? */
		if( cap_addr == NULL )
			return 0;
	}

	file = (struct objfile *) addr;

	if( memcmp( file->magic, FILE_MAGIC, strlen( FILE_MAGIC ) ) != 0 )
		return 0;

	/* passed all tests */
	return 1;
}


/* Use ObjInfo to find an object size */
static uintptr_t
find_object_size( void* addr )
{
	objinfo_t info;
	int r;

	assert( addr != NULL );
	
	r = ObjInfo( addr, O_SET_NONE, &info );
	if( r != 0 )
		return 0;

	return info.length;
}

/* make sure we have the cap for a file*/
/* FIXME: this could be implemented much better! (overflow etc) */
static int
check_file_cap( cap_t file_cap )
{

	/* first check the clist */
	if( file_clist == NULL )
	{
		cap_t clist_cap;
		if( create_clist_and_add( PAGESIZE, &clist_cap ) != 0 )
			return -1;

		file_clist = (clist_t*) clist_cap.address;
	}
	
	/* add the cap */
	assert( file_clist != NULL );
	add_to_clist( file_cap, file_clist );

	return 0;
}

/* create an object file - start_size 0 == default */
void*
_objfile_create( size_t start_size )
{
	cap_t file_cap;
	struct objfile *file;

	if( start_size == 0 )
		start_size = DEFAULT_FILE_SIZE;
	
	start_size += PAGESIZE; /* we want PAGESIZE at the start for
				   meta-data */
	
	/* create the object */
	file_cap.passwd = FILE_PASSWORD;

	/* FIXME: prolly want an ObjInfo with persistent set? */
	file_cap.address = ObjCreate( start_size, FILE_PASSWORD, NULL );

	/* failed!! */
	if( file_cap.address == NULL )
		return NULL;

	/* FIXME: add cap to the mirror list, this is dodgey :) */
	if( check_file_cap( file_cap ) != 0 )
		return NULL;

	/* fill out some header info */
	file = (struct objfile*) file_cap.address;

	/* copy in magic */
	memcpy( file->magic, FILE_MAGIC, strlen( FILE_MAGIC ) );
	
	/* ... now the rest */
	file->st_mode  = 0777;
	file->st_ino   = ((uintptr_t) file) + PAGESIZE; /* start of data */
	file->st_dev   = _FT_UFILE;
	file->st_nlink = 1;
	file->st_uid   = 0;
	file->st_gid   = 0;
	file->st_size  = 0;
	
	file->st_atime = 0;  /* take these from ObjInfo() instead?? */
	file->st_mtime = 0;
	file->st_ctime = 0;
	
	/* FIXME: create some semaphores */
	
	return file_cap.address;
}

/* delete an objfile - name are taken care of elsewhere */
int 
_objfile_delete( void* file_addr )
{
	int r;

	/* check it's valid */
	if( !is_objfile( file_addr ) )
		return -1;

	/* delete it! */
	r = ObjDelete( file_addr );

	return r;
}

/* open an objfile by pointer - deal with caps etc. return oftab entry */
int
_objfile_open( void* file_addr, int oflags )
{
	_open_file desc;
	int flag_set = 0;
	int ret;

	assert( file_addr != NULL );

	/* check it's valid */
	if( !is_objfile( file_addr ) )
		return -1;

	/* create a struct for the oftab */
	desc.type = _FT_UFILE;
	
	/* fill out the obj data */
	desc.data.obj.file = (struct objfile *) file_addr;
	desc.data.obj.offset = 0;    /* always start at beginning */
	desc.data.obj.objsize = 0;

	/* work out read/write flags */
	if( (oflags & O_WRONLY) == O_WRONLY )
	{
		desc.data.obj.flags.read = 0;
		desc.data.obj.flags.write = 1;
		flag_set = 1;
	}

	if( (oflags & O_RDWR) == O_RDWR )
	{
		desc.data.obj.flags.read = 0;
		desc.data.obj.flags.write = 1;
		flag_set = 0;
	}

	if( (oflags & O_RDONLY) == O_RDONLY
	    || flag_set == 0 )
	{
		desc.data.obj.flags.read = 1;
		desc.data.obj.flags.write = 0;
	}

	/* append flag */	
	if( (oflags & O_APPEND) == O_APPEND )
		desc.data.obj.flags.append = 1;

	/* if they specify O_TRUNC then modify the file to 0 len */
	if( (oflags & O_TRUNC) == O_TRUNC )
		desc.data.obj.file->st_size = 0;

	/* find the object size */
	desc.data.obj.objsize = find_object_size( desc.data.obj.file );
	if( desc.data.obj.objsize == 0 )
	{
		/* FIXME: set errno */
		return -1;
	}

	/* now add it to the open file table */
	ret = _oft_add( &desc );

	/* FIXME: errno? */
	return ret;
}

int
_objfile_write(_open_file *file, const void *buf, size_t nbyte)
{
	struct obj_fdesc* desc;
	void* dest;

	assert( file != NULL );
	assert( file->type == _FT_UFILE );

	desc = &file->data.obj;

	/* FIXME: lock the file */

	/* if appending, move the current offset */
	if( desc->flags.append != 0 )
		desc->offset = desc->file->st_size;

	/* check if we need to resize the object */
	if( desc->offset + nbyte + PAGESIZE > desc->objsize )
	{
		/* FIXME: implement growing & returning errors here */
		assert( !"file growing NYI" );
	}

	/* work out the dest. pointer */
	dest = ((void*)desc->file) + desc->offset + PAGESIZE;

	/* copy the data over */
	memcpy( dest, buf, nbyte );

	/* check the file length */
	if( desc->offset + nbyte > desc->file->st_size )
		desc->file->st_size = desc->offset + nbyte;

	/* set the file pos */
	desc->offset += nbyte;

	/* FIXME: unlock file */

	return nbyte;
}

int
_objfile_read  ( _open_file *file, void *buf, size_t nbyte )
{
	struct obj_fdesc* desc;
	void* src;

	assert( file != NULL );
	assert( file->type == _FT_UFILE );

	desc = &file->data.obj;

	/* FIXME: lock the file */

	assert( desc->offset <= desc->file->st_size );

	/* are they trying to read past EOF? */
	if( desc->offset + nbyte > desc->file->st_size )
		nbyte = desc->file->st_size - desc->offset;

	/* if no more can be read (or they specified 0 nbyte) */
	if( nbyte == 0 )
		return 0;

	/* work out where to copy from */
	src = ((void*)desc->file) + desc->offset + PAGESIZE;

	/* do the copy */
	memcpy( buf, src, nbyte );

	/* set the file pos */
	desc->offset += nbyte;

	return nbyte;
}

int
_objfile_close( _open_file *file )
{
	/* we don't really need to do anything in this current impl. */
	return 0;
}

int
_objfile_stat( void *addr, struct stat *buf )
{
    struct objfile *file;

    /* check it's valid */
    if( !is_objfile( addr ) )
	return -1;
    
    /* make it a nice type */
    file = (struct objfile*) addr;

    /* FIXME: lock here */
    
    buf->st_mode |= file->st_mode;
    buf->st_ino   = file->st_ino;
    buf->st_dev   = file->st_dev;
    buf->st_nlink = file->st_nlink;
    buf->st_uid   = file->st_uid;
    buf->st_gid   = file->st_gid;
    buf->st_size  = file->st_size;
    
    /* FIXME: prolly easiest if we get these from Mungi with ObjInfo() */
    buf->st_atime = file->st_atime;
    buf->st_mtime = file->st_mtime;
    buf->st_ctime = file->st_ctime;

    return 0;
}
