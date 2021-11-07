/****************************************************************************
 *
 *      $Id: namemap.h,v 1.2 2002/05/31 07:56:35 danielp Exp $
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

/* Header file for 'name mappings'. This does the actual
 * Plan-9 style work, calling on the name hash for stuff
 */

#ifndef NAMEMAP_H
#define NAMEMAP_H

typedef struct dirent  /* make this a dirent for POSIX */
{
	int   d_namelen;
	char *d_name;
	long flags;
} nm_dirent_t;

/* FIXME: we might want to put other stuff in here
 */
typedef struct
{
	void *st_dev;   /* device == name tree */
	long  st_flags; /* flags */
	long  st_ino;   /* 'inode' == value */
} nm_stat_t;

typedef struct
{
	char *dir;
	long start;
	long overflow;
	long bind_num;

	nm_dirent_t data;
} nm_dir_t;



/* mount 'tree' over the spot of new */
int nm_mount( void *map, char *new, void *tree, int type, char* cwd );

/* bind 'new' over the top of 'old */
int nm_bind( void *map, char *new, char *old, int type, char* cwd );

/* unmount entry 'dir' */
int nm_unmount( void *map, char *dir, char* cwd );

/* name & dir stuff */
nm_dir_t    *nm_opendir( void *map, char *path, char* cwd );
nm_dirent_t *nm_readdir( void *map, nm_dir_t* dir, cap_t pb, char* cwd );
int nm_closedir( void *map, nm_dir_t *dir, char* cwd );
int nm_addname( void *map, char *name, long value, 
		long flags, cap_t pb, char* cwd ); 
int nm_unlink( void *map, char *text, cap_t pb, char* cwd );
int nm_mkdir( void *map, char *name, cap_t pb, char* cwd );
int nm_stat( void *map, char *name, nm_stat_t* buf, cap_t pb, char *cwd );

/* types of binds - replace, before or after! ;) */
#define NMT_REPLACE 0
#define NMT_BEFORE  1
#define NMT_AFTER   2

/* flags for read/write */
#define NMF_NOREAD    32
#define NMF_NOWRITE   64
#define NMF_READWRITE  0

#define NMT_TYPES   (NMT_REPLACE | NMT_BEFORE | NMT_AFTER)
#define NMF_FLAGS   (NMF_NOREAD | NMF_NOWRITE)

/* return values */
#define NM_OK     0
#define NM_FAIL   1
#define NM_MEM    2


#endif /* NAMEMAP_H */
