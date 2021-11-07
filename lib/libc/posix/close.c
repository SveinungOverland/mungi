/****************************************************************************
 *
 *      $Id: close.c,v 1.3 2002/05/31 07:43:51 danielp Exp $
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

#include <mungi.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "oftab.h"



/* close - as usual. */
int
close(int fildes)
{
	int ret = 0;
	_open_file *file;

	file = _oft_lookup(fildes);
	
	/* not a real file handle */
	if (file == NULL) {
		assert(!"EBADF!");
		return -1;
	}

	switch (file->type) {
	case _FT_PDX:
                /* FIXME: prolly want to do something else here? */
		ret = 0;
		break;

	case _FT_UFILE:
		ret = _objfile_close(file);
		break;

	case _FT_USTREAM:
		assert(!"Not Implemented!");
                break; /* for what it's worth */
	default:
		assert(!"Bad file type!\n");
	}

	/* now close the actual file table entry */
	if (ret == 0)
		ret = _oft_remove(fildes);

	return ret;
}
