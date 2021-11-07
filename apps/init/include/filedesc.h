/****************************************************************************
 *
 *      $Id: filedesc.h,v 1.3 2002/05/31 04:57:13 danielp Exp $
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
 * $Id: filedesc.h,v 1.3 2002/05/31 04:57:13 danielp Exp $
 *
 *  File descriptor definition
 */

#ifndef _FILEDESC_H
#define _FILEDESC_H


/* PDX entry points for operations */
typedef struct pdx_desc_s
{
	cap_t read;      /* PDX capability */
	long read_desc;  /* PDX internal descriptor (== a cap??) */

	cap_t write;
	long write_desc;

	cap_t activate;
	long activate_desc;

	/* FIXME: this could be done better! */
	cap_t buffer;    /* buffer to talk thru */

} pdx_fdesc_t;



/* FIXME: should this be moved?? */
typedef struct pdx_buf_s
{
	long descriptor;  /* internal stream desc. */
	size_t size;      /* # bytes written/read  */
	char buf[1];      /* make as long as you want.... */
} pdx_buf_t;

struct obj_fdesc
{
	struct objfile *file;
	
	uintptr_t offset;       /* location in object */
	                        /* NOTE: POSIX says offset should still be
				 * shared after a fork/exec. This is not
				 * simple to copy here, so we dont!
				 * After fexec() reads & writes are independent
				 */
	uintptr_t objsize;      /* size of object at opening time
				 * get at opening time so we know when
				 * to grow files.
				 */

	struct
	{
		unsigned read   : 1;
		unsigned write  : 1;
		unsigned append : 1;
	} flags;
};


#endif /* _FILEDESC_H */
