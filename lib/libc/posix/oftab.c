/****************************************************************************
 *
 *      $Id: oftab.c,v 1.3 2002/05/31 07:43:53 danielp Exp $
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
 * Manage a user-land open file table
 */

#include <mungi.h>
#include <assert.h>
#include <strings.h>
#include <stdlib.h>
#include <mlib.h>
#include "oftab.h"

#include "env.h"

#define OFT_SIZE 10                 /* table size */
static _open_file *oftab[OFT_SIZE]; /* the table  */
static int _open_max = 0;           /* max open file # */

/* FIXME: This whole file suffers from concurrency probs :( */

static int find_free_handle( int start );

/* find the first free handle from start */
static int
find_free_handle( int start )
{
	int pos;

	/* find a free file handle */
	for( pos = 0; pos < _open_max; pos++ )
		if( oftab[pos] == NULL )
			return pos;

	/* FIXME: will this always work? :) */
	return _open_max;
}

/* add an open file handle - return new file handle */
int
_oft_add( _open_file *file )
{
	int pos;

	assert( file != NULL );

	/* find first free handle */
	pos = find_free_handle( 0 );

	if( pos >= _open_max )
		_open_max = pos + 1;

	/* FIXME: dynamically size it */
	if( pos >= OFT_SIZE )
		assert( !"open file table full!!!\n" );
      
	_oft_set( pos, file );

	return pos;
}

/* close a file handle */
int
_oft_remove( int desc )
{
	/* FIXME: bounds check desc */

	/* clear it out with 0 (and hence the in_use flag */
	free( oftab[desc] );
	oftab[desc] = NULL;

	/* FIXME: remove caps for it? */

	return 0;
}


/* set file desc to file */
int
_oft_set( int desc, _open_file *file ) 
{
	_open_file *new;

	/* FIXME: bounds check desc */

	if( oftab[desc] != NULL )
		assert( !"Collision in open file table!\n" );
	
	/* duplicate the data */
	new = malloc( sizeof( *new ) );
	if( new == NULL )
		return -1;
	memcpy( new, file, sizeof( _open_file ) );
	oftab[desc] = new;

	/* set the reference count */
	oftab[desc]->ref_count = 1;

	if( desc >= _open_max )
		_open_max = desc + 1;

	return desc;
}

/* find the data for an open file */
_open_file *
_oft_lookup( int desc )
{
	/* FIXME: bounds check */

	if( oftab[desc] == NULL )
		return NULL;

	return oftab[desc];
}


/* duplicate an open-file entry and inc. reference count 
 * this is much the same as POSIX dup2()
 * return the new descriptor
 */
int
_oft_dup( int desc, int desc2 )
{
	int pos;

	/* FIXME: bounds check! :) */
	
	/* we don't want to resize etc. here */
	if( desc2 > _open_max )
		return -1;

	pos = find_free_handle( desc2 );
	if( pos >= _open_max )
		_open_max = pos + 1;

	/* copy pointer, add reference */
	oftab[pos] = oftab[desc];
	oftab[pos]->ref_count++;

	return pos;
}


/* FIXME: these won't work for >1 reference counts!! */

/*serialise the open file table*/
size_t 
_oft_serialise(void * dest)
{
	struct env_filetable *eft = dest ;
	int i;

	/* copy in data, item by item */
	int size = 0;
	eft->num_files = _open_max;

	size = eft->num_files * sizeof( _open_file);
	for( i = 0; i < _open_max; i++ )
	{
		if( oftab[i] != NULL )
			memcpy( &eft->files[i], oftab[i], 
				sizeof( _open_file ) );
		else
			memset( &eft->files[i], 0, sizeof( _open_file ) );
	}

	return size;
}

/* DEserialise the open file table*/
int 
_oft_deserialise(void * env_data,size_t size)
{
	struct env_filetable *eft = env_data ;
	int i;
	
	_open_max = eft->num_files; /* next handle to open */
	size = eft->num_files * sizeof(_open_file);
	for( i = 0; i < _open_max; i++ )
	{
		if( eft->files[i].ref_count != 0 )
			_oft_set( i, &eft->files[i] );
		else
			oftab[i] = NULL;
	}

	/*clean up pdx caps*/
	for( i=0; i < eft->num_files; i++ )
	{
		if( oftab[i] == NULL )
			continue;

		if( oftab[i]->type == _FT_PDX  )
		{
			/*clear the buffer*/
			oftab[i]->data.pdx.buffer.address=NULL ;
			oftab[i]->data.pdx.buffer.passwd=0 ;

			/*add cap to our lists*/
			apd_insert_cap(oftab[i]->data.pdx.read);
			apd_insert_cap(oftab[i]->data.pdx.write);
			apd_insert_cap(oftab[i]->data.pdx.activate);
		}
	}
	
	return 0 ;
}


