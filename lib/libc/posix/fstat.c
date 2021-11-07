/****************************************************************************
 *
 *      $Id: fstat.c,v 1.2 2002/05/31 07:43:52 danielp Exp $
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
#include <sys/stat.h>
#include <sys/types.h>
#include <compat.h>
#include <assert.h>
#include <string.h>

#include "oftab.h"


/* fstat:
 * 
 * fstat system call on a file. look up table entry & return stuff
 * really dodgey impl for now!
 */

int 
fstat( int fildes, struct stat *buf )
{
	memset( buf, 0, sizeof( *buf ) );

	/* FIXME: do properly! */
	buf->st_mode = 0777;
	buf->st_ino = 666;
	buf->st_dev = 1;
	buf->st_nlink = 1;
	buf->st_uid = 0;
	buf->st_gid = 0;
	buf->st_size = 37;
	buf->st_atime = 0;
	buf->st_mtime = 0;
	buf->st_ctime = 0;
	
	return 0;
}
