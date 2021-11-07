/****************************************************************************
 *
 *      $Id: dirent.h,v 1.4 2002/08/01 08:10:09 cgray Exp $
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

/* POSIX compliant naming functions. Wrappers for the naming service */

/* FIXME: clean these up! */
#include <../lib/naming/namehash.h>
#include <../lib/naming/namemap.h>

/* to get mode_t */
#include <compat.h>

#define PATH_MAX 256

/* struct dirent is provided in namemap.h */
typedef nm_dir_t  DIR;

DIR           *opendir ( const char *dir_name );
struct dirent *readdir ( DIR *dir_pointer );
int            closedir( DIR *dir_pointer );


/* For the environment */
size_t _naming_serialise(void* dest);
int    _naming_deserialise(void * env_data, size_t size);
