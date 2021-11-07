/****************************************************************************
 *
 *      $Id: objfile.h,v 1.2 2002/05/31 07:43:52 danielp Exp $
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

/* structure of an object-file 
 *
 * An object file is a file implemented as a single Mungi object
 *
 * A page is reserved at the start for meta-data and a semaphore
 * for locking
 *
 */

#define FILE_MAGIC "FILE"
#define DEFAULT_FILE_SIZE (1*1024*1024) /* 1MB of VM */
#define FILE_PASSWORD     37            /* all files use same passwd (ick!) */

struct objfile
{
	char      magic[4];  /* should say 'FILE' (no null!) */

	mode_t    st_mode;
	ino_t     st_ino;    /* == object address */
	dev_t     st_dev;    /* == FT_UFILE */
	nlink_t   st_nlink;  /* always 1 */
	uid_t     st_uid;    /* 0 */
	gid_t     st_gid;    /* 0 */
	off_t     st_size;   /* actual size */
	
	time_t    st_atime;  /* take these from ObjInfo()?? */
	time_t    st_mtime;
	time_t    st_ctime;
};
