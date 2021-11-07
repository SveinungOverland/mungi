/****************************************************************************
 *
 *      $Id: nstree.c,v 1.4 2002/07/22 10:18:02 cgray Exp $
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

/* interface functions to a NameSpace tree! */

/*
 * FIXME: Add checking for . or .. somewhere!
 * FIXME: Check for duplicate inserts!!!
 * FIXME: does find parent return the name in the new or the old string?!?
 * FIXME: does find_r free up the string it makes?!
 *
 */

#include <mungi.h>
#include <assert.h>
#include <string.h>

#include "btree.h"
#include "bt_types.h"
#include "nstree.h"

#include "hmalloc.h"


/* Local static functions */
static void *ns_alloc( BTree tree, size_t size );
static void *ns_realloc( BTree tree, void *ptr, size_t size );
static void ns_free( BTree tree, void* mem );
static BTKey hashpjw(const char *s);

/* so we can print */
void UserPrint( char *fmt, ... );

/* initialise a tree */
BTree db_init ( int max_parts, cap_t *cap );

#define FIND_NOTFOUND 0
#define FIND_FIRST    1
#define FIND_OTHER    2

#define ENUM_SIZE     100

#define ITYPE_ITEM    0
#define ITYPE_PARENT  1

/* kinda stupid :) */
#define SLASH '/'

static void *ns_alloc( BTree tree, size_t size )
{
	return hmalloc( tree->pool->heap, size );
}

static void *ns_realloc( BTree tree, void *ptr, size_t size )
{
	return hrealloc( tree->pool->heap, ptr, size );
}

static void ns_free( BTree tree, void* mem )
{
	hfree( tree->pool->heap, mem );
}


/* A decent hash algorithm. All I need now is a decent HASHTABLE_SIZE
 *
 * hashpjw() -- hash function from P. J. Weinberger's C compiler
 *              call with NUL-terminated string s.
 *              returns value in range 0 .. HASHTABLE_SIZE-1
 *
 * Reference: Alfred V. Aho, Ravi Sethi, Jeffrey D. Ullman
 *            _Compilers: Principles, Techniques, and Tools_
 *            Addison-Wesley, 1986.
 */

/* #define HASHTABLE_SIZE 5001 */
/* big prime */
#define HASHTABLE_SIZE 18446744073709551557UL
/* #define HASHTABLE_SIZE 500 */
/* #define HASHTABLE_SIZE 50 */

static BTKey
hashpjw(const char *s)
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
	return h % HASHTABLE_SIZE;
}

/* find the entry for a name in a single tree */
static int ns_find_item( BTree tree, char *name, 
			 nsent_t **data, nsent_t **dprev )
{
	long hash;
	int r, found = FIND_NOTFOUND;
	nsent_t *ent, *prev = NULL;
	
	assert( tree );
	assert( name );
	assert( data );

	/* find the hash */
	hash = hashpjw( name );

	/* lookup the hash */
	r = BTSearch( tree, hash, &ent );

	if( r != BT_OK )
		return FIND_NOTFOUND;

	/* check the first item */
	if( strcmp( ent->name, name ) == 0 )
	{
		found = FIND_FIRST;
	}

	/* find it in the chain */
	while( ent->next && found == FIND_NOTFOUND )
	{
		prev = ent;
		ent = ent->next;

		if( strcmp( ent->name, name ) == 0 )
			found = FIND_OTHER;
	}

	if( found != FIND_NOTFOUND )
	{
		*data = ent;
		if( dprev != NULL )
			*dprev = prev;
	}

	return found;
}

/* search all trees recursively looking for a name or a parent */
static int ns_find_r( BTree *tree, char **path_name, 
			 nsent_t **data, nsent_t **dprev, int type )
{
	int r, len, exit_loop = 0;
	char *name;
	char *cur, *end;
	nsent_t *ent = NULL;
	BTree mem_tree = *tree;

	assert( tree );
	assert( path_name );
	assert( *path_name );

	/* first, duplicate the string */
	len = strlen( *path_name ) + 1;

	name = ns_alloc( mem_tree, len );

	if( name == NULL )
		return FIND_NOTFOUND;

	strcpy( name, *path_name );

	/* if we shortcut out on the root item */
	r = FIND_FIRST;

	/* now we can iteratively resolve names! :) */
	cur = name;
	for(;;)
	{
		/* munch slashes */
		while( *cur == SLASH )
			cur++;

		/* find the end */
		end = strchr( cur, SLASH );

		/* null if it's the last item! */
		if( end == NULL )
		{
			if( type == ITYPE_PARENT )
				break;
			else
			{
				assert( type == ITYPE_ITEM );
				exit_loop = 1;
			}

		}
		else
			*end = NULL;

		/* this happens if they spec. a dir with trailing
		   slash or it's the root dir! */
		if( strlen( cur ) == 0 )
			break;

		/* look it up */
		r = ns_find_item( *tree, cur, &ent, dprev );

		if( r == FIND_NOTFOUND )
		{
			ns_free( mem_tree,  name );
			return FIND_NOTFOUND;
		}

		/* if that was the last thing for an item lookup */
		if( exit_loop )
		{
			/* we leave through here if there's no 
			 * trailing slash 
			 */
			if( ent != NULL )
				*tree = ent->child;

			break;
		}


		/* setup the child dir & away we go */
		assert( ent );
		if( ent->flags.dat.treedat.dir == 0 )
		{
			ns_free( mem_tree, name );
			return FIND_NOTFOUND;
		}

		assert( ent->child );
		*tree = (BTree) ent->child;

		/* move the pointer in the name - skipping the null we 
		 * inserted. We can't jump past the end of the buffer
		 * because their null terminator would have been dealt
		 * with above
		 */
		cur = end + 1;
	}

	/* if we got to here then ent should have something useful in it,
	 * otherwise they specified some garbage such as an empty name */
	*data = ent;

	/* tell them the name of the base-part */
	*path_name += (cur - name);

	ns_free( mem_tree, name );

	/* r may not be relevant if we are looking up the parent, 
	   but carefactor? */
	return r;
}

/* Given a full path name 'name' (eg. blah/blah/blah.jpg)
 * this will find the tree reference for the parent directory
 * or return an error. Leading and double slashes will be 
 * ignored
 */
static int ns_findparent( BTree *tree, char **name )
{
	nsent_t *data;

	return ns_find_r( tree, name, &data, NULL, ITYPE_PARENT );
}


/* lookup the item 'name' and return entry info */
static int ns_find( BTree *tree, char *name, 
		    nsent_t **data, nsent_t **dprev )
{
	return ns_find_r( tree, &name, data, dprev, ITYPE_ITEM );
}

static int ns_insert_ent( BTree tree, nsent_t *ent )
{
	int r;
	nsent_t *existing;
	
	assert( tree );
	assert( ent );
	ent->next = NULL;

	/* try the insertion */
	r = BTIns( tree, ent );

	if( r == BT_OK )
		return NST_OK;
	else if( r != BT_DUPLICATE )
		return NST_FAIL;

	/* otherwise, lookup & add to the chain */
	r = BTSearch( tree, ent->hash, &existing );
	
	if( r != BT_FOUND )
		assert( !"Shouldn't get here!" );
	assert( existing );
	
	/* Check it's not already in there first! */
	while( existing->next != NULL )
	{
		/* duplicate */
		if( strcmp( existing->name, ent->name ) ==  0 )
			return NST_FAIL;

		/* ent->next = existing->next; */
		existing = existing->next;
	}

	/* add it to the chain */
	existing->next = ent;

	/* now we're done! */
	return NST_OK;


}

static int ns_delete_ent( BTree tree, char *name, nsent_t **pent )
{
	nsent_t *prev, *ent, *del = NULL, **mod;
	int r;

	assert( tree );
	assert( name );
	assert( ent );

	/* first find it! */
	r = ns_find_item( tree, name, &ent, &prev );

	if( r == FIND_NOTFOUND )
		return NST_FAIL;

	assert( ent );

	if( r == FIND_FIRST )
	{
		/* see if we should just update the
		 * chain or delete it
		 */
		if( ent->next != NULL )
		{
			/* we need to update the tree entry */
			r = BTModify( tree, ent->hash, &mod );
			assert( r == BT_OK );

			*mod = ent->next;
				
		}
		else
		{
			/* just delete it! */
			r = BTDel( tree, ent->hash, &del );
			if( r == BT_OK )
				assert( del == ent );
		}
	}
	else
	{
		assert( r == FIND_OTHER );
		assert( prev );

		/* shorten the chain */
		prev->next = ent->next;
	}
	
	*pent = ent;
	return NST_OK;
}

/* create a name tree */
int ns_create( void **tree, cap_t* cap )
{
	BTree bt;
	bt = db_init( 500, cap );

	if( bt != NULL )
	{
		*tree = (void*) bt;
		return NST_OK;
	}
	else
	{
		*tree = NULL;
		return NST_FAIL;
	}
}

/* add an item to the tree */
int ns_add( void *tree, char *org_name, nsentflag_t flags, void* value )
{
	int r;
	nsent_t *ent;
        char *copy, *name;

	assert( tree );
	assert( org_name );

	/* fistly find the parent of this, make sure it's valid */
	name = org_name;
	r = ns_findparent( (BTree*) &tree, &name );

	if( r == FIND_NOTFOUND )
		return NST_FAIL;

	/* now allocate space */
	ent = (nsent_t*) ns_alloc( (BTree) tree, sizeof( nsent_t ) );
	
	if( ent == NULL )
 		return NST_FAIL; 

        copy = (char*) ns_alloc( (BTree) tree, strlen( name ) + 1) ;

	/* FIXME: Free other stuff? */
	if( copy == NULL )
		return 1;

	strcpy( copy, name );

	/* fill it out */
	ent->value = value;
	ent->flags = flags;
	ent->child = NULL;
	ent->name  = copy;
	ent->hash  = hashpjw( copy );
	ent->next  = NULL;

	/* try to insert it */
	return ns_insert_ent( tree, ent );
}


/* remove an item from the tree */
int ns_del( void *tree, char *name )
{
	int r;
	nsent_t *ent;
	
	assert( tree );
	assert( name );

	/* FIXME: find parent! */
	r = ns_findparent( (BTree*) &tree, &name );

	if( r == FIND_NOTFOUND )
		return NST_FAIL;

	r = ns_delete_ent( tree, name, &ent );

	if( r != NST_OK )
		return NST_FAIL;

	ns_free( (BTree) tree, ent->name );
	ns_free( (BTree) tree, ent );

	return NST_OK;
}

/* lookup an item */
int ns_lookup( void *tree, char * name, nsent_t **data )
{
	int r;

	assert( tree );
	assert( name );
	assert( data );

	r = ns_find( (BTree*) &tree, name, data, NULL );

	if( r != FIND_NOTFOUND )
		return NST_OK;
	else
		return NST_FAIL;

}

/* modify an item */
int ns_modify( void *tree, char *name, nsent_t *newdata, char *newname )
{
	int r, bRename = 0, bMakeDir = 0, bClearDir = 0;
	nsent_t *ent, *check;
	char *file_bit = NULL;
	char *name_buf;
	BTree dest_tree = (BTree) tree, tree_src = (BTree) tree;

	assert( tree );
	assert( name );
	assert( newdata );
	
	/* first lookup the item they want to modify */
	r = ns_find( &tree_src, name, &ent, NULL );

	if( r == FIND_NOTFOUND )
		return NST_FAIL;
	assert( ent );

	/* if they want to rename, check they can */
	if( newname != NULL )
	{
		if( strcmp( name, newname ) != 0 )
			bRename = 1;
	}

	if( bRename )
	{
		/* check what they want to rename it to doesn't
		 *  exist (butt the parent does!)
		 */
		BTree bad_tree = (BTree) tree;
		r = ns_find( &bad_tree, newname, &check, NULL );

		if( r != FIND_NOTFOUND )
		{
			UserPrint( "Already exists!\n" );
			return NST_FAIL;
		}

		file_bit = newname;
		r = ns_findparent( (BTree*) &dest_tree, &file_bit );

		if( r == FIND_NOTFOUND )
		{
			UserPrint( "Parent doesn't exist!\n" );
			return NST_FAIL;
		}

		/* make the name long enough */
		name_buf = (char*) ns_realloc( tree, ent->name, 
					       strlen( file_bit ) + 1   );
		
		if( name_buf == NULL )
		{
			UserPrint( "Out of memory!\n" );
			return NST_FAIL;
		}

		ent->name = name_buf;
	}

	/* if they want to clear dir flag, check they can */
	if( ent->flags.dat.treedat.dir != newdata->flags.dat.treedat.dir )
	{
		if( newdata->flags.dat.treedat.dir == 1 )
			bMakeDir = 1;  /* we can always turn something
					  into a dir */
		else
			bClearDir = 1;
	}

	if( bClearDir )
	{
		/* Check the subdir is empty! */
		if( BTEmpty( (BTree) ent->child ) != BT_NOT_FOUND )
			return NST_FAIL;
	}

	/* if we're making it a dir */
	if( bMakeDir )
	{
		BTree db;
		void *heap = ((BTree)tree)->pool->heap;

		assert( ent->flags.dat.treedat.dir == 0 );

		db = (BTree) hmalloc(heap, sizeof(BTree_S));

		if (!db)
			return NST_FAIL;

		db->pool = (PagePool *) hmalloc(heap, sizeof(PagePool));

		if (!db->pool)
			return NULL;
        
		/* set the heap in the pool */
		db->pool->heap = heap;
		db->pool->n_allocs = db->pool->n_frees = 0;
		db->root = NULL;
		db->depth = 0;

		ent->child = (BTree) db;
		ent->flags.dat.treedat.dir = 1;

	}

	/* if we're clearing the dir bit */
	if( bClearDir )
	{
		assert( ent->flags.dat.treedat.dir == 1 );

		/* free the memory for the child */
		hfree( ((BTree)ent->child)->pool->heap, (void*) (ent->child) );

		ent->child = NULL;
		ent->flags.dat.treedat.dir = 0;
	}

	/* set the other (boring) flags */
	ent->flags.dat.userdat = newdata->flags.dat.userdat;
	ent->value = newdata->value;

	/* now do a rename */
	if( bRename )
	{
		BTree stree;
		char *old_file;
		nsent_t *dummy;

		/* lookup the parent of the source */
		stree = (BTree) tree;
		old_file = name;
		r = ns_findparent( &stree, &old_file );
		assert( r != FIND_NOTFOUND );

		/* delete the source from the tree */
		r = ns_delete_ent( tree, ent->name, &dummy );
		assert( r == NST_OK );

		/* update the name etc. */
		ent->hash = hashpjw( file_bit );
		assert( ent->name );

		strcpy( ent->name, file_bit );

		/* finally add it to the dest */
		r = ns_insert_ent( dest_tree, ent );
		assert( r == NST_OK );
	}

	return NST_OK;
}

/* take the list of matching B+Tree items and unpack what fits into
 * the buffers to return to the app
 */
static int ns_decode_enum( int r, nsent_t **eout, unsigned long start, 
			   unsigned long overflow, void * outdat, int outlen,
			   int max_num, int bValue, void* value )
{
	int i, len, coff, bSpace = 0;
	int num_found = 0, chain_pos = 0;
	long last_hash = 0, last_overflow = 0;
	nsent_t *cur;
	nsenum_t *dat;


	coff = 1;
	dat = (nsenum_t*) outdat;
	for( i = 0; i < r ; i++ )
	{
		cur = eout[i];

		last_hash = cur->hash;
		last_overflow = 0;

		/* index into overflow chain */
		while( cur )
		{
			/* only start at the offset they specify */
			if( cur->hash == start 
			    && chain_pos++ < overflow )
			{
				cur = cur->next;
				last_overflow++;
				continue;
			}

			if( bValue )
			{
				if( value != cur->value )
				{
					cur = cur->next;
					last_overflow++;
					continue;
				}
			}

			/* copy in the stuff */
			dat[coff].other.value  = cur->value;
			dat[coff].other.flags  = cur->flags;

			/* setup the next array - must inc by one (at least) */
			len = strlen( cur->name ) + 1;
			dat[coff].other.arrnext = 1 + coff + len / 
				sizeof( nsenum_t );
			if( len % sizeof( nsenum_t ) )
				dat[coff].other.arrnext++;

			/* Check we have the space! */
			if( outlen <= coff * sizeof( nsenum_t ) + len  )
			{
				bSpace = 1;
				goto decode_exit;
			}

			/* copy in the string */
			strcpy( (char*) &dat[coff+1], cur->name );

			/* move along the chain */
			coff = dat[coff].other.arrnext;
			num_found++;
			cur = cur->next;
			last_overflow++;

			/* if they have all they want */
			if( num_found >= max_num )
			{
				goto decode_exit;
			}
		}
	}

 decode_exit:

	/* now setup the return info */
	dat[0].first.num = num_found;
	dat[0].first.next_start = last_hash;
	dat[0].first.next_overflow = last_overflow;

	/* set the return value */
	if( num_found == 0 && bSpace != 0 )
		return NST_MOREMEM;
	else
		return NST_OK;
}

/* enumerate entries */
int ns_enum( void *tree, char *dir, unsigned long start, 
	     unsigned long overflow, void *outdat, unsigned long outlen, 
	     unsigned int num )
{
	int r, max = ENUM_SIZE;
	nsent_t *eout[ENUM_SIZE], *dir_ent, find;

	assert( tree );
	assert( outdat );
	assert( num != 0 );
	
	/* locate the parent */
	r = ns_find( (BTree*) &tree, dir, &dir_ent, NULL );

	if( r == FIND_NOTFOUND )
	{
		UserPrint( "dir '%s' not found!\n", dir );
		return NST_FAIL;
	}

	/* make sure it is actually a dir, but not the root dir */
	if( dir_ent )
		if( dir_ent->flags.dat.treedat.dir != 1 )
			return NST_FAIL;

	/* the tree we get back should be it! */
	assert( tree );
	
	/* so we don't retreive 100 when they only want 1 or 2 
	 * we need num + 1 because of the way we handle
	 * subsequent overflow items if they only want one
	 */
	max = min( max, num + 1 );

	find.name = NULL;
	find.hash = start;

	r = BTEnum( tree, eout, max, &find, BTE_ENUM );

	return ns_decode_enum( r, eout, start, overflow, 
			       outdat, outlen, num, 0, 0 );
}

/* enumerate entries */
int ns_rlookup( void *tree, char *dir, unsigned long start, 
		unsigned long overflow, void *outdat, unsigned long outlen, 
		unsigned int num, void *value )
{
	int r, max = ENUM_SIZE, count;
	nsent_t *eout[ENUM_SIZE], *dir_ent, find;
	nsenum_t *dat = (nsenum_t*) outdat;

	assert( tree );
	assert( outdat );
	assert( num != 0 );

	/* locate the parent */
	r = ns_find( (BTree*) &tree, dir, &dir_ent, NULL );

	if( r == FIND_NOTFOUND )
		return NST_FAIL;

	/* the tree we get back should be it! */
	assert( tree );
	
	/* so we don't retreive 100 when they only want 1 or 2 
	 * we need num + 1 because of the way we handle
	 * subsequent overflow items if they only want one
	 */
	max = min( max, num + 1 );

	find.name = NULL;
	find.hash = start;
	find.value = value;

	/* make sure we always return at least one item if there
	 *  is one in the tree 
	 */
	while( 1 )
	{

		/* get a bunch of things from the tree */
		count = BTEnum( tree, eout, max, &find, BTE_RLOOK );
		
		/* decode the ones that match the value */
		r = ns_decode_enum( count, eout, start, overflow, 
				    outdat, outlen, num, 1, value );

		/* error or out of memory! */
		if( r != NST_OK )
			return r;

		/* if we didn't get anything decoded and the B-Tree
		 * ran out of items then exit
		 */
		if( dat[0].first.num != 0 || count < max )
			break;

		start = find.hash = dat[0].first.next_start;
		overflow = dat[0].first.next_overflow;
	}

	return r;
}
