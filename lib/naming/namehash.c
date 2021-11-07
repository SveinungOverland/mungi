/****************************************************************************
 *
 *      $Id: namehash.c,v 1.3 2002/05/31 07:56:35 danielp Exp $
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

/* name hashing - provides resolution of relative
 * names and hashing them for plan-9 stuff
 */

#include <mungi.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hmalloc.h"
#include "namehash.h"

#define SLASH     '/'
#define SLASH_STR "/"

extern void UserPrint( char *fmt, ... );

void *nh_alloc( namehash_t *nhash, size_t size )
{
	return hmalloc( nhash->hmalloc_dat, size );
}

void *nh_realloc( namehash_t *nhash, void *ptr, size_t size )
{
	return hrealloc( nhash->hmalloc_dat, ptr, size );
}

void nh_free( namehash_t *nhash, void* mem )
{
	hfree( nhash->hmalloc_dat, mem );
}


static long nh_hash( const char *s, long size ) 
{
	const char *p;
	unsigned long h, g;
	
	h = 0;
	
	for (p = s; *p != '\0'; p++) {
		h = (h << 4) + *p;
		g = h & 0xf000000000000000;
		if (g != 0) {
			h ^= g >> 24;
			h ^= g;
		}
	}
	return 0;
	return h % size;
}


/* given a path, and then len, firstly count the number
 * of separated items in it, then tokenise it all
 * into an array
 */
static char** nh_pathtok( char *path, int len ) 
{
	int i;
	int inName = 0;
	int toks = 0;
	char **list, *next;
	int pos = 0, newlen;

	for( i = 0; i < len; i++ )
	{
		/* slash after name */
		if( path[i] == SLASH && inName == 1 )
			inName = 0;

		/* start of a name */
		if( path[i] != SLASH && inName == 0 )
		{
			inName = 1;
			toks++;
		}
	}

	/* add one for final null */
	list = (char**) malloc( (toks + 1) * sizeof( char* ) );
	if( list == NULL )
	{
/*		UserPrint( "Could not allocate array!\n "); */
		return NULL;
	}

	/* now add 'em */
	inName = 0;
	i = 0;
	while( pos < toks )
	{
		/* skip slashes */
		while( path[i] == SLASH && i < len )
			i++;

		/* trailing slash? */
		/* FIXME: given toks will this ever fail? */
		if( i == len )
			break;

		/* find the next slash */
		next = strchr( &path[i], (int) SLASH );

		/* next slash or end of string */
		if( next == NULL )
			newlen = len - i;
		else
			newlen = next - path - i;

		/* create the copy */
		list[pos] = (char*) malloc( newlen + 1 );

		if( list[pos] ==  NULL )
			return NULL;

		assert( list[pos] );
		assert( path + i );
		strncpy( list[pos], path + i, newlen );
		list[pos][newlen] = '\0';

		i = next - path + 1;
		pos++;
	}
	

	/* add the null */
	list[pos] = NULL;

	return list;
}

/* squash down a char** array
 * delete 'count' items starting at start and moving *backwards*
 * given the array is bounded is [0..total-1]
 */
static void nh_squash( char **names, int start, int total, int count )
{
	int m1, m2, mcount;

	m1 = start - (count - 1);
	if( m1 < 0 )
		m1 = 0;

	m2 = start + 1;
	if( m2 > total )
		m2 = total;

	mcount = total - m2 + 1;

	/* actually move it */
	memmove( &names[m1], &names[m2], mcount * sizeof( char * ) );
}

/* given a list of name-parts, compress out . and .. */
static void nh_compress( char **names )
{
	int cur = 0;
	int num = 0;

	while( names[cur] != NULL )
	{
		num++;
		cur++;
	}

	cur = 0;
	while( names[cur] != NULL )
	{
		if( strcmp( names[cur], "." ) == 0 )
		{
			/* a '.' item */
			free( names[cur] );
			nh_squash( names, cur, num, 1 );
			num--;
		}
		else if( strcmp( names[cur], ".." ) == 0 )
		{
			/* a '..' */
			if( cur > 0 )
				free( names[cur-1] );
			free( names[cur] );

			nh_squash( names, cur, num, 2 );

			if( cur > 0 )
			{
				
				num -= 2;
				cur--;
			}
			else
				num -= 1;
		}
		else
		{
			/* normal - just keep going */
			cur++;
		}
	}
}


/* given a token list, makes a full & absolute path
 * name with a leading slash and no trailing slash
 */
static void nh_tok2str( char *buf, char **tokens )
{
	int i = 0;

	/* start with leading / */
	strcpy( buf, SLASH_STR );

	while( tokens[i] != NULL )
	{
		if( i !=  0 )
			strcat( buf, SLASH_STR );

		strcat( buf, tokens[i] );
		free( tokens[i] );
		i++;
	}
}

/* given an ugly/relative/whatever name 'name'
 * this will resolve it to an absolute name, taking
 * into account the current directory etc.
 */

char *nh_absolute( char *name, char *cwd )
{
	char **tokens;
	char *full_name;
	char *absolute;
	int len;

	assert( name );
	assert( cwd != NULL );

	/* get the length */
	len = strlen( name );

	/* absolute path - don't care about curdir */
	if( *name == SLASH )
	{
		full_name = strdup( name );
	}
	else
	{
		assert( cwd != NULL );
		len += strlen( cwd ) + 1;  /* len cwd + a slash */

		/* FIXME; check this!!! */
		/* full_name = (char*) malloc( len + 1 );*/ /* +1 for null */
		full_name = (char*) malloc( len + 100 ); /* +1 for null */

		if( full_name == NULL )
		{
/*			UserPrint( "fullname null (%d)\n", len + 1 ); */
			return NULL;
		}
		
		sprintf( full_name, "%s/%s", cwd, name );
	}

	tokens = nh_pathtok( full_name, len );

	free( full_name );
	assert( !(full_name=0) );  /* null it in debug */

	/* failed :( */
	if( tokens == NULL )
	{
/*		UserPrint( "PathTok null\n" ); */
		return NULL;
	}

	/* compress down . and .. */
	nh_compress( tokens );

	/* make some space for the final name */
	absolute = (char*) malloc( len + 2 ); /* NULL + leading / */
	if( absolute == NULL )
	{
/*		UserPrint( "absolute null, returning\n" ); */
		return NULL;
	}
	
	/* make them nice */
	nh_tok2str( absolute, tokens );

/*
	if( !absolute )
		UserPrint( "absolute now null!\n" );
*/

	free( tokens );

	/* now make the full string */
	return absolute;
}


/* insert the (relative) name 'name' into nhash, pointing to value */
int nh_insert( namehash_t *nhash, char *name, void *value, char *cwd )
{
	char *absolute, *abs2;
	long hash;
	hashent_t *ent, *prev = NULL, *enew;

	assert( name  );
	assert( nhash );

	/* make it an absolute name */
	absolute = nh_absolute( name, cwd );
	if( absolute == NULL )
		return NH_MEM;

	/* hash it */
	hash = nh_hash( absolute, nhash->size );

	/* find the chain */
	ent = nhash->table[ hash ];

	/* look for it in the chain */
	while( ent != NULL )
	{
		/* already there! */
		if( strcmp( ent->name, absolute ) == 0 )
		{
			free( absolute );
			return NH_FAIL;
		}

		prev = ent;
		ent = ent->next;
	}

	/* copy it into the tree */
	assert( absolute );
	abs2 = (char*) nh_alloc( nhash, strlen( absolute ) + 1 );
	if( abs2 == NULL )
		return NH_MEM;

	strcpy( abs2, absolute );
	free( absolute );
	
	/* create the new entry */
	enew = (hashent_t*) nh_alloc( nhash, sizeof( hashent_t ) );
	if( enew == NULL )
		return NH_MEM;
	enew->next = NULL;
	enew->name = abs2;
	enew->child = value;

	/* add it to the list */
	if( prev == NULL )
		nhash->table[ hash ] = enew;
	else
		prev->next = enew;

	return NH_OK;
}

/* delete relative name 'name' from the hash */
int nh_delete( namehash_t *nhash, char *name, char *cwd )
{
	char *absolute;
	long hash;
	hashent_t *ent, *prev = NULL;

	assert( name  );
	assert( nhash );

	/* make it an absolute name */
	absolute = nh_absolute( name, cwd );
	if( absolute == NULL )
		return NH_MEM;

	/* hash it */
	hash = nh_hash( absolute, nhash->size );

	/* find the chain */
	ent = nhash->table[ hash ];

	/* look for it in the chain */
	while( ent != NULL )
	{
		/* already there! */
		if( strcmp( ent->name, absolute ) == 0 )
			break;

		prev = ent;
		ent = ent->next;
	}

	free( absolute );
	
	if( ent == NULL )
		return NH_FAIL;

	if( prev == NULL )
		nhash->table[ hash ] = ent->next;
	else
		prev->next = ent->next;

	nh_free( nhash, ent );

	return NH_OK;
}

/* Lookup the relative name 'name' in hash. stick value into *value
 * and return success/fail
 */
int nh_find( namehash_t *nhash, char *name, void **value, char *cwd )
{
	char *absolute;
	long hash;
	hashent_t *ent, *prev = NULL;

	assert( name  );
	assert( nhash );

	/* make it an absolute name */
	absolute = nh_absolute( name, cwd );
	if( absolute == NULL )
		return NH_MEM;

	/* hash it */
	hash = nh_hash( absolute, nhash->size );

	/* find the chain */
	ent = nhash->table[ hash ];

	/* look for it in the chain */
	while( ent != NULL )
	{
		/* already there! */
		if( strcmp( ent->name, absolute ) == 0 )
			break;

		prev = ent;
		ent = ent->next;
	}
	free( absolute );

	if( ent == NULL )
		return NH_FAIL;

	/* so we can just check for existence */
	if( value != NULL )
		*value = ent->child;

	return NH_OK;

}

/* lookup name and set the pointer to mod */
int nh_modify( namehash_t *nhash, char *name, void *value, char *cwd )
{
	char *absolute;
	long hash;
	hashent_t *ent, *prev = NULL;

	assert( name  );
	assert( nhash );

	/* make it an absolute name */
	absolute = nh_absolute( name, cwd );
	if( absolute == NULL )
		return NH_MEM;

	/* hash it */
	hash = nh_hash( absolute, nhash->size );

	/* find the chain */
	ent = nhash->table[ hash ];

	/* look for it in the chain */
	while( ent != NULL )
	{
		/* already there! */
		if( strcmp( ent->name, absolute ) == 0 )
			break;

		prev = ent;
		ent = ent->next;
	}
	free( absolute );

	if( ent == NULL )
		return NH_FAIL;

	/* set it */
	ent->child = value;

	return NH_OK;
}

/* initialise a new namehash. Using size pages and 'entries' entries
 * in the hash table
 */
namehash_t *nh_init( int size, int entries )
{
	void *heap;
	namehash_t *nh;
	cap_t cap;

	/* make a malloc object */
	heap = hmalloc_init( size, &cap );
	assert( heap != NULL );

	nh = (namehash_t*) hmalloc(heap, sizeof(namehash_t));

	if (!nh)
		return NULL;
	
	/* create the hash table */
	nh->table = (hashent_t**) hmalloc( heap, 
					   entries * sizeof( namehash_t ) );
		
	/* FIXME: delete the hmalloc obj here? */
	if ( !nh->table )
		return NULL;
        
	/* setup initial stuff in this object */
	nh->size = entries;
	memset( nh->table, 0, entries * sizeof( namehash_t ) );

	nh->hmalloc_dat = heap;
	nh->hmalloc_cap = cap;

	return nh;
}
