/****************************************************************************
 *
 *      $Id: posix.c,v 1.3 2002/05/31 07:56:36 danielp Exp $
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

/* These are the POSIX-compliant naming functions */

#include <mungi.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Naming includes */
#include "namehash.h"
#include "namemap.h"
#include "nstree.h"

/* FIXME: needed for serialisation... clean up */
#include "lib/paxlib.h"
#include "tree_cmpt/INameTree_io.h"

/* POSIX includes */
#include "dirent.h"

/* to access internal file stuff */
/* FIXME: clean up?? */
#include "../libc/posix/oftab.h"

/* for open() etc */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Info needed for namespace stuff */
static namehash_t *namespace = NULL;
static cap_t       param_buf;  /* FIXME: make this a pool of some kind */
static char       *cwd = NULL; /* current working directory */

/* 'p9' - or POSIX/9 functions.
   mix of POSIX and Plan-9 type things for bootstrap */

extern INameTree_t *nm_get_root_tree( void *map );

/* FIXME: extras - find somewhere better to put these */
extern int p9_init( namehash_t *nh, cap_t pbuf );
extern cicap_t p9_get_default_cicap(void);
extern int p9_mount(char* mount_pt, void *tree, int access_mode);


cicap_t
p9_get_default_cicap(void)
{
	return int_get_cicap( nm_get_root_tree( namespace ) );
}

/* p9_init - initialise the POSIX std. namespace for this binary */
int
p9_init( namehash_t *nh, cap_t pbuf )
{
	if( namespace != NULL )
	{
		printf( "ERROR: p9 namespace already initialised!\n" );
		return 1;
	}

	namespace = nh;
	param_buf = pbuf;
	cwd = strdup( "/" );  /* initialise working directory */

	return 0;
}


/* p9_mount - mount a tree into the namespace */
int 
p9_mount( char* mount_pt, void *tree, int access_mode )
{
	int r;

	/* FIXME: add some checks */
	r = nm_mount(namespace, mount_pt, tree, (uintptr_t)access_mode, cwd);

	return r;
}


/* POSIX Function! */
/* FIXME: I use icky casts here to get around POSIX using const
 * and me being too lazy to. I must fix naming to use const and drop the
 * cast. ceg
 */

/* mkdir: the 2nd param is meaningless for now, so ignore it */
int
mkdir( const char *path, mode_t ignored )
{
	int r;

	
	r = nm_mkdir( namespace, (char*) path, param_buf, cwd );

	/* FIXME: set errno? */
	if( r != NM_OK )
		return -1;

	return 0;
}

/* mkdir: the 2nd param is meaningless for now, so ignore it */
int
rmdir( const char *path )
{
	int r;

	r = nm_unlink( namespace, (char*) path, param_buf, cwd );

	/* FIXME: set errno? */
	if( r != NM_OK )
		return -1;

	return 0;
}

/* rename: kinda tricky? Impl. later */
int rename(const char *oldpath, const char *newpath)
{
    assert( !"rename() called!" );
    return -1;
}

/* FIXME: Check return values on these suckers!!! Make POSIX like! */
DIR *
opendir ( const char *dir_name )
{
	/* FIXME: this doesn't verify existence of directory */
	return nm_opendir( namespace, (char*) dir_name, cwd );
}


struct dirent *
readdir ( DIR *dir_pointer )
{
	return nm_readdir( namespace, dir_pointer, param_buf, cwd );
}


int 
closedir ( DIR *dir_pointer )
{
	return nm_closedir( namespace, dir_pointer, cwd );
}


int
chdir( const char *path )
{
	char *old;
	int r;
	nm_stat_t ret;

	assert( path != NULL );

	/* stat the target dir!! */
	r = nm_stat( namespace, (char*) path, &ret, param_buf, cwd );

	if( r != NM_OK )
		return -1; /* FIXME: set errno */

	/* copy it */
	old = cwd;
	cwd = nh_absolute( (char*) path, old );
	free( old );

	return 0;
}

char *
getwd( char *buffer )
{
	if( strlen( cwd ) >= PATH_MAX )
	{
		strcpy( buffer, "cwd > PATH_MAX" );
		return NULL;
	}

	strcpy( buffer, cwd );
	return buffer;
}

/* use this one for longer name compatability */
char *
getcwd( char *buffer, size_t size )
{
	if( strlen( cwd ) >= (size+1) )
		return NULL;

	strcpy( buffer, cwd );
	return buffer;
}


/* creat - just call open() */
int creat ( const char *path, mode_t mode )
{
	return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}


/* look up names & pass off onto obj* file functions */
/* Valid flags in oflag are:
   O_RONLY, O_WRONLY, O_RDWR - Read only, write only, read & write
   O_CREAT  - Create file if it doesn't exist
   O_EXCL   - Fail if file exists (if with O_CREAT)
   O_TRUNC  - Change the length of the file to 0 on opening
   O_APPEND - file pointer set to end of file for EVERY write
*/
int open ( const char *path, int oflag, ... /* mode_t mode */ )
{
 	int r;
	nm_stat_t ret;
	int o_creat, o_excl;
	void* file_addr;

	assert( path != NULL );

	/* creat - 
	         look up name
		 if name not there, 
		     create file object
		     create name for object
		 if name is there (or create failed) then fail if O_EXCL
		 
		 open the file ptr & return the oftab number
	*/

	/* open - 
	         look up name
		 if not there, fail
		 
		 open file ptr & return oftab number
	*/


	/* for readability */
	o_creat = (oflag & O_CREAT) == O_CREAT;
	o_excl  = (oflag & O_EXCL)  == O_EXCL;

	/* look up the name */
	r = nm_stat( namespace, (char*) path, &ret, param_buf, cwd );

	/* failed & not trying to open */
	if( r == NM_FAIL && o_creat == 0 )
	{
		/* FIXME: set errno */
		return -1;
	}

	/* if the name is there & we want creat & excl, then fail */
	/* FIXME: what should be behaviour if they are creating on an
	 * existing directory? it's POSIX so fail?
	 */
	if( r == NM_FAIL && o_creat != 0 && o_excl != 0 )
	{
		/* FIXME: set errno */
		return -1;
	}

	/* create it if it does not exist! */
	if( r == NM_FAIL && o_creat != 0 )
	{
		/* 1. create a file object */
		file_addr = _objfile_create( 0 );

		/* 2. create a name for it */
		/* FIXME: set flags to mark it as a file? */
		r = nm_addname( namespace, (char*) path, (uintptr_t)file_addr, 
				0, param_buf, cwd );
		
		/* FIXME: is there anything in 'r' to check here? */

		/* 3. re-stat the object (get rid of race conditions?) */
		r = nm_stat( namespace, (char*) path, &ret, param_buf, cwd );

		/* 4. delete object if not the same */
		if( ret.st_ino != ((uintptr_t)file_addr) )
		{
			_objfile_delete( file_addr );

			/* 5. abort if O_EXCL */
			if( o_excl != 0 )
			{
				/* FIXME: set errno */
				return -1;
			}
		}
	}

	/* OK, at this stage we should have the info from a stat
	   and be able to open the file! */

	/* make sure it's not an invalid name */
	if( ret.st_ino == NULL )
	{
		/* FIXME: set errno */
		return -1;
	}

	/* FIXME: check flags to make sure it's an objfile (eg. impl. flags) */
	return _objfile_open( (void*) ret.st_ino, oflag );
}

/* since we don't have symlinks, stat and lstat are the same */
int 
lstat(const char *file_name, struct stat *buf)
{
    return stat( file_name, buf );
}

int 
stat(const char *file_name, struct stat *buf)
{
    int r;
    nm_stat_t ret;
    void* file_addr;
    nsentflag_t flags;

    /* look up the name */
    r = nm_stat( namespace, (char*) file_name, &ret, param_buf, cwd );

    /* failed */
    if( r == NM_FAIL ) 
    {
	    /* FIXME: set errno */
	    return -1;
    }

    /* get the addr out */
    file_addr = (void *)ret.st_ino; 

    /* clear the return struct */
    memset( buf, 0, sizeof( *buf ) );

    /* should we set the dir bit? */
    flags.flags.all = ret.st_flags;
    if( flags.dat.treedat.dir != 0 )
    {
	buf->st_mode |= S_IFDIR;
    }
    else if( file_addr == NULL )  /*error to stat a non-dir with ptr == NULL*/
	return -1;

    /* should we set the file bit? */
    /* FIXME: check flags, too */
    if( file_addr != NULL )
	r = _objfile_stat( file_addr, buf );
    else 
	r = 0;

    return r;
}


