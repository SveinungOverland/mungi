/****************************************************************************
 *
 *      $Id: namemap.c,v 1.3 2002/05/31 07:56:35 danielp Exp $
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

/* Name mappings implementation */


#include <mungi.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tree_cmpt/INameTree_io.h"
#include "tree_cmpt/tree.h"
#include "nstree.h"
#include "hmalloc.h"

#include "namehash.h"
#include "namemap.h"

#include "abort_test.h"

#define SLASH     '/'
#define SLASH_STR "/"

extern void UserPrint( char *fmt, ... );

INameTree_t *nm_get_root_tree(void *map);

typedef struct nment_s
{
	void *tree;
	char *path;
	int   flags;  /* read/write flag */

	struct nment_s *next;
} nment_t;


/* given a path, find the list of trees it could be
 * in and the relative name it will be in them
 */
static int nm_find_trees( void *map, char *org_path, nment_t **trees, 
			  char **relative, char* cwd )
{
	int r, alen;
	void *tree_tmp;
	char *path;
	char *ch, *absolute;

	/* first duplicate the path so we can work with it */
	assert( org_path );
	absolute = nh_absolute( org_path, cwd );
	if( absolute == NULL )
	{
/*		UserPrint( "nh_absolute returned null\n" ); */
		return NM_FAIL;
	}
	alen = strlen( absolute );
	path = strdup( absolute ); /* take a working copy */
	if( path == NULL )
	{
		free( absolute );
		return NM_MEM;
	}

	/* now, starting at the end, work backwards chopping path segments
	   off until we find a match */
	while( 1 )
	{
		/* lookup the current thing */
		r = nh_find( map, path, &tree_tmp, cwd );

		if( r == NH_MEM )
			return NH_MEM;

		if( r == NH_OK )
		{
			*trees = tree_tmp;
			/* FIXME: Check this is right */
			assert( path );
			*relative = strdup( absolute +
					    strlen( path ) );
			free( absolute );
			free( path );
			return NM_OK;
		}

		assert( r == NH_FAIL );
		
		/* shorten the name by one item */
		ch = strrchr( path, (int) SLASH );

		/* nothing left to check & not found */
		if( ch == NULL )
			break;

		/* preserve leading / */
		/* problem with /foo - we need to check the root */
		if( ch == path )
		{
			/* check the last bit, or just the slash */
			if( strlen( path ) == 1 )
				*ch = NULL;
			else
				*(ch + 1) = NULL;
		}
		else
			*ch = NULL;
	}

	free( absolute );
	free( path );

	return NM_FAIL;
}


/* add a map entry - new is what it goes over,
 * path is the path in the tree, type says before, after
 * or delete what's already there
 */
static int nm_map( void *map, char *new, char *path, 
		   void *tree, int type_p, char* cwd )
{
	nment_t *ent, *cur, *prev, *newent;
	int r, type, flags;

	assert( map );
	assert( new );
	assert( path );
	assert( tree );

	cur = NULL;
	type  = type_p & NMT_TYPES;
	flags = type_p & NMF_FLAGS;
	
	/* look up new */
	r = nh_find( map, new, (void**) &ent, cwd );

	/* out of memory is bad */
	if( r == NH_MEM )
		return NM_MEM;

	/* make the entry */
	newent = (nment_t*) nh_alloc( map, sizeof( nment_t ) );
	if( newent == NULL )
		return NM_MEM;

	newent->tree  = tree;
	newent->path  = path;
	newent->flags = flags;
	newent->next  = NULL;

	/* if it's not there - add it & return */
	if( r == NH_FAIL )
	{
		/* FIXME: if not replace, work out current and add */
		/* assert( type == NMT_REPLACE ); */

		/* add the new entry */
		r = nh_insert( map, new, newent, cwd );

		return r;
	}

	/* else it's there */
	assert( r == NH_OK );
	assert( cur != NULL );

	/* if it's replace */
	if( type == NMT_REPLACE )
	{
		/* write the new data into the first entry - 
		 * save updating the table
		 */
		cur = ent->next;
		*ent = *newent;
		
		/* delete the old stuff */
		while( cur )
		{
			prev = cur;
			cur = cur->next;
			nh_free( map, prev );
		}

		nh_free( map, newent );

		return NM_OK;
	}

	assert( type == NMT_BEFORE || type == NMT_AFTER );
	cur = ent;

	/* if it's add before, mod the chain */
	if( type == NMT_BEFORE )
	{
		newent->next = cur;
		nh_modify( map, new, newent, cwd );

		if( r == NH_MEM )
		{
			nh_free( map, newent );
			return NM_MEM;
		}

		return r == NM_OK ? NM_OK : NM_FAIL;
	}

	assert( type == NMT_AFTER );

	/* if it's end, go to the end of the chain */
	while( cur != NULL )
	{
		prev = cur;
		cur = cur->next;
	}

	/* add it */
	cur->next = newent;

	return NM_OK;
	
}

/* mount 'tree' over the spot of new */
int nm_mount( void *map, char *new, void *tree, int type, char* cwd )
{
	char *absolute;

	assert( map );
	assert( new );
	assert( tree );

	/* setup the new data */
	absolute = nh_absolute( new, cwd );
	if( absolute == NULL )
		return NH_MEM;

	/* map with new data & a replace type*/
	return nm_map( map, absolute, SLASH_STR, tree, 
		       (type & NMF_FLAGS) |  NMT_REPLACE, cwd );
}


/* bind 'old' over the top of 'new' */
int nm_bind( void *map, char *new, char *old, int type_p, char* cwd )
{
	int r, type, flags;
	nment_t *trees;
	char *rel;

	assert( map );
	assert( new );
	assert( old );

	type  = type_p & NMT_TYPES;
	flags = type_p & NMF_FLAGS;

	/* find the source (old) information */
	r = nm_find_trees( map, old, &trees, &rel, cwd );

	if( r != NM_OK )
		return r;

	/* We need to insert all of these with the new relative name
	 * into the new hash 
	 */
	if( type == NMT_REPLACE || type == NMT_BEFORE )
	{
		assert( !"NYI!\n" );

	}
	else
	{
		/* append all to the end, one by one */
		while( trees != NULL )
		{
			r = nm_map( map, new, rel, trees->tree, 
				    NMT_AFTER | flags, cwd );
			
			if( r != NM_OK )
				return NM_FAIL;

			trees = trees->next;
		}
	}

	return NM_OK;
}

/* unmount entry 'dir' */
int nm_unmount( void *map, char *dir, char *cwd )
{
	assert( map );
	assert( dir );

	/* FIXME: Implement! */

	return NH_FAIL;
}


/* some posix-esque functions. These should be augmented/wrapped
 * with a full file system 
 */

nm_dir_t *nm_opendir( void *map, char *path, char *cwd )
{
	nm_dir_t *dir;
	int r;
	nment_t *trees;
	char *relative;

	dir = (nm_dir_t*) malloc( sizeof( nm_dir_t ) );
	if( dir == NULL )
		return NULL;

	/* make sure path references a valid tree */
	r = nm_find_trees( map, path, &trees, &relative, cwd );

	if( r != NM_OK )
		return NULL;

	/* setup the values */
	dir->dir = strdup( path );
/*	UserPrint( "dir is 0x%llx\n", dir->dir ); */
	assert( dir->dir != NULL );

	dir->start = 0;
	dir->overflow = 0;
	dir->bind_num = 0;
	dir->data.d_name = NULL;

	return dir;
}

/* FIXME: remove the cap! */
nm_dirent_t *nm_readdir( void *map, nm_dir_t* dir, cap_t pb, char *cwd )
{
	int r, i, found = 0;
	nment_t *trees;
	char *rel, *str;
	environment_t ev;
	nsenum_t *buf;

	assert( dir );
	r = nm_find_trees( map, dir->dir, &trees, &rel, cwd );

	if( r != NM_OK )
		return NULL;

	/* look up the correct number item in the dir - could be a union */
	i = 0;
	while( i < dir->bind_num )
	{
		i++;
		trees = trees->next;
		if( trees == NULL )
		{
			free( rel );
			return NULL;
		}
	}

	if( strcmp( rel, "" ) == 0 )
	{
		free( rel );
		rel = strdup( "/" );
	}

	while( 1 )
	{
		/* FIXME: magic numbers! */
		/* copy data into the param buffer */
		strcpy( pb.address + 1024, rel );

		/* ABORT?? */
#if ABORT_LEVEL == ABORT_NMAP
		dir->start++;
		free( rel );
		if( dir->start < ABORT_MAX_COUNT )
			return &dir->data;
		else
			return NULL;		
#endif		

		/* make the actual call */
		r = int_fenum( trees->tree, pb, 1024, dir->start, 
			       dir->overflow, 2048, 255, 1, 0, &ev );
		
		if( r != BTC_OK )
			return NULL;

		buf = (nsenum_t*) (pb.address + 2048);

		found = buf[0].first.num;

		if( found != 0 )
		{
			dir->start = buf[0].first.next_start;
			dir->overflow = buf[0].first.next_overflow;
			break;
		}

		/* move thru trees if nothing left here */
		dir->start = 0;
		dir->overflow = 0;
		dir->bind_num++;
		trees = trees->next;

		if( trees == NULL )
		{
			free( rel );
			return NULL;
		}
	}
	
	/* if we got here then it's all OK! */
	str = strdup( (char*) &buf[2] );

	if( dir->data.d_name != NULL )
		free( dir->data.d_name );

	assert( str );
	dir->data.d_name = str;
	dir->data.d_namelen = strlen( str );
	dir->data.flags = 0;  /* FIXME: read these! */

	/* clean up */
	free( rel );

	return &dir->data;
}

int nm_closedir( void *map, nm_dir_t *dir, char* cwd )
{
	assert( dir != NULL );

	if( dir->data.d_name != NULL )
		free( dir->data.d_name );

	if( dir->dir != NULL )
		free( dir->dir );

	free( dir );

	return 0;
}

int nm_addname( void *map, char *name, long value, 
		long flags, cap_t pb, char *cwd )
{
	int r;
	nment_t *trees;
	char *rel;
	environment_t ev;

	assert( name );
	assert( map );

	r = nm_find_trees( map, name, &trees, &rel, cwd );

	if( r != BTC_OK )
		return NM_FAIL;

	while( trees )
	{
		if( (trees->flags & NMF_NOWRITE) != 0 )
		{
			trees = trees->next;
			continue;
		}

		strcpy( pb.address + 1024, rel );

		r = int_create( trees->tree, pb, 1024, 
				flags, value, &ev );

		if( r == BTC_OK )
		{
			free( rel );
			return NM_OK;
		}

		trees = trees->next;
	}

	/* couldn't add to anything */
	free( rel );
	return NM_FAIL;
}

int nm_unlink( void *map, char *name, cap_t pb, char *cwd )
{
	int r;
	nment_t *trees;
	char *rel;
	environment_t ev;

	assert( map );
	assert( name );

	r = nm_find_trees( map, name, &trees, &rel, cwd );

	if( r != BTC_OK )
		return NM_FAIL;

	strcpy( pb.address + 1024, rel );

	/* for each tree, delete from the 1st one we can find!  */
	while( trees )
	{
		if( (trees->flags & NMF_NOWRITE) != 0 )
		{
			trees = trees->next;
			continue;
		}

		r = int_delete( trees->tree, pb, 1024, &ev );

		if( r == BTC_OK )
			return NM_OK;

		trees = trees->next;
	}

	return NM_FAIL;
}

int nm_mkdir( void *map, char *name, cap_t pb, char *cwd )
{
	int r;
	nment_t *trees;
	char *rel;
	environment_t ev;
	nsentflag_t flags;

	/* first find the tree & add the name */

	assert( name );
	assert( map );

	r = nm_find_trees( map, name, &trees, &rel, cwd );

	if( r != BTC_OK )
		return NM_FAIL;

	/* find first writable */
	while( trees != NULL )
	{
		if( (trees->flags & NMF_NOWRITE) != 0 )
		{
			trees = trees->next;
			continue;
		}
		
		strcpy( pb.address + 1024, rel );
	
		r = int_create( trees->tree, pb, 1024, 	0, 0, &ev );

		if( r == BTC_OK )
		{
			/* now create the dir flag & set that */
			flags.flags.all = 0;
			flags.dat.treedat.dir = 1;
			
			r = int_modify( trees->tree, pb, 1024, -1, 
					flags.flags.all, 0, &ev );
			
			free( rel );

			/* FIXME: delete the name? */
			if( r != BTC_OK )
				return NM_FAIL;
			
			return NM_OK;
		}
		
		trees = trees->next;
	}

	return NM_OK;

}

int nm_stat( void *map, char *name, nm_stat_t* buf, cap_t pb, char *cwd )
{
	int r;
	nment_t *trees;
	char *rel;
	environment_t ev;

	assert( name );
	r = nm_find_trees( map, name, &trees, &rel, cwd );

	if( r != NM_OK )
		return NM_FAIL;

	/* find first readable */
	while( trees != NULL )
	{
		if( (trees->flags & NMF_NOREAD) != 0 )
		{
			trees = trees->next;
			continue;
		}
		
		strcpy( pb.address + 1032, rel );
	
		r = int_lookup( trees->tree, pb, 1032, 	0, 1024, &ev );

		if( r == BTC_OK )
		{
		    nsent_t *ent;

		    /* copy in the data */
		    buf->st_dev = (void*) trees->tree;
		    ent = (nsent_t*)(pb.address+1024);
		    buf->st_flags = ent->flags.flags.all;
		    buf->st_ino = (uintptr_t)ent->value;
		    
		    free( rel );
		    return NM_OK;
		}
		
		trees = trees->next;
	}

	free( rel );
	return NM_FAIL;
}

/* FIXME: impl. serialisation better */
INameTree_t *
nm_get_root_tree( void *map )
{
	int r;
	nment_t *trees;
	char *rel;

	r = nm_find_trees( map, "/", &trees, &rel, "/" );
	assert( r == BTC_OK  );
	assert( trees != NULL );

	free(rel);

	return trees->tree;
}
