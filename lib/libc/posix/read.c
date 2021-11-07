/****************************************************************************
 *
 *      $Id: read.c,v 1.4 2002/05/31 07:43:53 danielp Exp $
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

#include "syscalls.h"
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "oftab.h"

/* read:
 *
 * Get the file descriptor info from the open file table. Filter the message
 * off to the appropriate read function.
 */

ssize_t 
read(int fildes, void *buf, size_t nbyte)
{
	ssize_t ret = 0;
	_open_file *file;

	file = _oft_lookup( fildes );

	/* not a real file handle */
	if( file == NULL )
	{
		assert( !"EBADF!" );
		return -1;
	}

	switch( file->type )
	{
		case _FT_PDX:
			ret = _pdxfile_read( file, buf, nbyte );
			break;

		case _FT_UFILE:
			ret = _objfile_read( file, buf, nbyte );
			break;

		case _FT_USTREAM:
			assert( !"NYI!" );

		default:
			assert( !"Bad file type!\n" );
	}

	return ret;
}
