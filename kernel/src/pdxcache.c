/****************************************************************************
 *
 *      $Id: pdxcache.c,v 1.3 2002/08/23 08:24:24 cgray Exp $
 *      Copyright (C) 2002 Operating Systems Research Group, UNSW, Australia.
 *
 *      This file is part of the Mungi operating system distribution.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      version 2 as published by the Free Software Foundation.
 *      A copy of this license is included in the top level directory of
 *      the Mungi distribution.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 ****************************************************************************/

#include "mungi/kernel.h"
#include "mungi/apd.h"
#include "mungi/pdxcache.h"
#include "mungi/mm.h"
#include "mungi/clock.h"

/* HASH function for cache table */
/* WARNING: expands multiple times! Watch out! */
/* XOR random-ish bytes from source apd and proc, and the lower byte of
   dest apd, then AND with 0xff to get a value in PDXCACHE_SIZE */
#define PDXCACHE_SIZE      256          /* # entries in the PDX cache */
#define BYTEN(w, n) (w>>(n<<3))         /* move nth byte -> 0th */
#define HASH(a, b, c) (((((((BYTEN((a), 1) ^ BYTEN((a), 2)) ^ BYTEN((a), 5)) ^ \
                       (BYTEN((b), 1)) ^ BYTEN((b), 2)) ^ BYTEN((b), 5)) ^ \
                       (c))) & 0xff)

struct pdxcache_entry {
	apd_t *apd_src;
	pdx_t  proc;
	const apd_t *apd_dest;
	struct apd_cache_id id;
};


struct pdxcache_entry *pdx_cache = NULL;
lock_t pdx_cache_lock;

/* pdxcache interface functions */
void
pdxcache_init(void)
{

	kprintf( "pdxc: Initialising PDX cache\n" );

	/* ignore a re-request (loudly) */
	if( pdx_cache != NULL )
	{
		kprintf( "WARNING: pdx_cache already initialised!\n" );
		return;
	}

	/* allocate a cache */
	/* FIXME: use kmalloc_obj to force it page aligned? */
	pdx_cache = kmalloc( PDXCACHE_SIZE * sizeof( struct pdxcache_entry ) );

	if( pdx_cache == NULL )
	{
		kprintf( "ERROR: could not kmalloc pdx cache\n" );
		assert( !"pdxcache: can not continue" );
	}

	/* clear it out */
	pdxcache_flush();

	/* initialise the lock */
	_lock_init( &pdx_cache_lock );

	kprintf( "pdxc: done initialising PDX cache\n" );
}

void
pdxcache_flush(void)
{
	assert( pdx_cache != NULL );

	/* flush the whole thing! */
	bzero( pdx_cache, PDXCACHE_SIZE * sizeof( struct pdxcache_entry ) );
}

void
pdxcache_add(apd_t *apd_src, pdx_t proc, const apd_t *apd_dest, 
	     struct apd_cache_id *id)
{
	int hash;

	assert( pdx_cache != NULL );

	/* hash it */
	hash = HASH( (uintptr_t) apd_src, (uintptr_t) proc, 
		     (uintptr_t) apd_dest );

	_lock_lock( &pdx_cache_lock );

	/* set the entry */
	pdx_cache[hash].apd_src  = apd_src;
	pdx_cache[hash].proc     = proc;
	pdx_cache[hash].apd_dest = apd_dest;
	pdx_cache[hash].id       = *id;

	_lock_unlock( &pdx_cache_lock );
}

apd_t *
pdxcache_lookup(apd_t *apd_src, pdx_t proc, const apd_t *apd_dest)
{
	int hash;
	apd_t *ret = NULL;

	clock_stop(IN_PDX_CACHE_LOOKUP);

	assert( pdx_cache != NULL );

	/* hash it */
	hash = HASH( (uintptr_t) apd_src, (uintptr_t) proc, 
		     (uintptr_t) apd_dest );

	clock_stop(POST_HASHING);

	_lock_lock( &pdx_cache_lock );

	clock_stop(POST_CACHE_LOCK);

	if( pdx_cache[hash].apd_src == apd_src
	    && pdx_cache[hash].proc == proc
	    && pdx_cache[hash].apd_dest == apd_dest )
	{
		clock_stop(PRE_APD_CHECK_ID);
		ret = apd_pdx_cache_check_id( &pdx_cache[hash].id );
		clock_stop(POST_APD_CHECK_ID);
	}
	_lock_unlock( &pdx_cache_lock );

	return ret;
}
