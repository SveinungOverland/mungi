/****************************************************************************
 *
 *      $Id: bthelp.c,v 1.2 2002/05/31 07:56:34 danielp Exp $
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

/* implement the 'helper' functions for the B+Tree for printing, allocation
   etc */

#include <mungi.h>
#include <assert.h>
#include "hmalloc.h"
#include "btree.h"


extern void UserPrint(char *x, ...);
void BTPrintObj(BTObject const obj);
BTPage *alloc_page(PagePool *pool);
void free_page(PagePool *pool, BTPage *p);
BTree db_init ( int max_parts, cap_t* cap );

void BTPrintObj(BTObject const obj)
{
	UserPrint( "obj: '%.25s' (%lld)\n", obj->name, obj->hash );
/*	UserPrint( "obj: '%.30s'\n", obj->name ); */
}

BTPage *alloc_page(PagePool *pool) 
{
	void *ret;

	assert( pool );

	ret = hmalloc( pool->heap, sizeof( BTPage ) );
	pool->n_allocs++;
	return (BTPage *) ret;
}


void free_page(PagePool *pool, BTPage *p) 
{
	assert( pool );
	assert( p );

	hfree( pool->heap, p );
	pool->n_frees++;
}

/* alloc/free helper functions */
/*
static cap_t heap = { 0, 0 };
static int pos = 0;
*/


/* initialise a B-Tree */

BTree db_init ( int max_parts, cap_t *cap ) 
{
    BTree db;
    void *heap;

    /* first allocate us a big chunk of memory for the object */
/*    heap = hmalloc_init( (2 << 10), cap ); */
    heap = hmalloc_init( max_parts, cap );
    assert( heap != NULL );

    db = (BTree) hmalloc(heap, sizeof(BTree_S));

    if (!db)
	return NULL;

    db->pool = (PagePool *) hmalloc(heap, sizeof(PagePool));

    if (!db->pool)
	return NULL;
        
    /* set the heap in the pool */
    db->pool->heap = heap;
    db->pool->n_allocs = db->pool->n_frees = 0;
    db->root = NULL;
    db->depth = 0;


    return db;
}
