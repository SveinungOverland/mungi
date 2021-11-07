/****************************************************************************
 *
 *      $Id: dupes.c,v 1.3 2002/05/31 07:43:52 danielp Exp $
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

/* implement dup(), dup2() and the dup subset of fcntl() */

#include "syscalls.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <stdarg.h>

#include "oftab.h"


int 
dup(int fildes)
{
	return fcntl(fildes, F_DUPFD, 0);
}

int 
dup2(int fildes, int fildes2)
{
	int r;

	r = close(fildes2);

	/* don't fcntl unless the close worked */
	if (r == 0)
		return fcntl(fildes, F_DUPFD, fildes2);
	else
		return r;
}

int
fcntl(int fildes, int cmd, ... /* int arg | struct flock *arg */)
{
		
	va_list va;
	int fildes2;
	_open_file *file;

	/* we only support F_DUPFD at the moment */
	if (cmd != F_DUPFD)
		return -1;

	/* lookup the file */
	file = _oft_lookup(fildes);	

	if (file == NULL) {
		/* FIXME: set errno */
		return -1;
	}
	
	/* find the 2nd file descriptor */
	va_start(va, cmd);
	fildes2 = va_arg(va, int);
	va_end(va);

	/* now we can do the dup */
	return _oft_dup(fildes, fildes2);
}
