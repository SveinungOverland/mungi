/****************************************************************************
 *
 *      $Id: mm.c,v 1.19.2.1 2002/08/29 04:32:01 cgray Exp $
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
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this
 * implementation, the available sizes are 2^n-4 (or 2^n-10) bytes long.
 * This is designed for use in a virtual memory environment.
 */

/* hohmuth 18 Aug 1995
   adapted to libmach/L3; changes #ifdef'd with L3_LIBMACH */

/* adapted to L4 3/9/96 Jerry Vochteloo  */

/* hohmuth: libmach/L3 adaptions */

/* Copyright (c) 1995 Michael Hohmuth
   See l3/mach_emul/COPYRIGHT for copyright and licensing details */

/* helper functions for libmach-internal memory management: we use a
   simple sbrk() (which essentially uses an L3 standard data space)
   and some ordinary malloc() (borrowed from BSD) on the top of it. */



#include "mungi/kernel.h"
#include "mungi/lock.h"
#include "mungi/mm.h"
#include "mungi/l4_generic.h"

#define	NBUCKETS	80
#define	MAGIC		0xef		/* magic # on accounting info */
#define RMAGIC		0x5555		/* magic # on range info */
#ifdef RCHECK
#define	RSLOP		sizeof (u_short)
#else
#define	RSLOP		0
#endif


/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled then a second word holds the size of the
 * requested block, less 1, rounded up to a multiple of sizeof(RMAGIC).
 * The order of elements is critical: ov_magic must overlay the low order
 * bits of ov_next, and ov_magic can not be a valid ov_next bit pattern.
 */
/* little endian dependent structure */
union	overhead {
#define u_char unsigned char
#define u_short unsigned short
#define u_int uint64_t
	union	overhead *ov_next;	/* when free */
	struct {
		u_char	ovu_magic;	/* magic number */
		u_char	ovu_index;	/* bucket # */
#ifdef RCHECK
		u_short	ovu_rmagic;	/* range magic number */
		u_int	ovu_size;	/* actual block size */
#endif
	} ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
#define	ov_rmagic	ovu.ovu_rmagic
#define	ov_size		ovu.ovu_size
};

typedef struct list {
	void * address;
	size_t size;
	struct list * next;
} freelist_t;


/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
typedef struct heap {
	lock_t heap_lock;
	union overhead *heap_nextf[NBUCKETS];
	uintptr_t heap_pagesz;
	uintptr_t heap_pagebucket;
	void *hs_current_brk;
	size_t hs_size;
#define nextf (heap->heap_nextf)
#define pagesz (heap->heap_pagesz)
#define pagebucket (heap->heap_pagebucket)
#define sbrk(off) (_libmach_heap_space_sbrk(heap, (off)))
} heap_t;



static heap_t  *THE_HEAP;
static freelist_t *_FREELIST;
static lock_t freelist_lock;
extern l4_threadid_t rpagerid;

static void * _libmach_heap_space_sbrk(heap_t *space, size_t incr);
static heap_t * _libmach_heap_new(void * base, size_t size);
static void * _kmalloc(heap_t *heap, size_t size);
static void _kfree(heap_t *heap, void * ptr);
static void morecore(heap_t *heap, uintptr_t bucket);
static void FreeInit(void * address, size_t size);
static void FreePrint(void);

static void *
_kmalloc(heap_t * heap, size_t nbytes)
{
  	union overhead *op;
  	uintptr_t bucket, n;
	uintptr_t amt;

	/*
	 * Convert amount of memory requested into closest block size
	 * stored in hash buckets which satisfies request.
	 * Account for space used per block for accounting.
	 *
	 * assuming heap pagesize will not change on us
	 */

	n = pagesz - sizeof (*op) - RSLOP;

	if (nbytes <= n) {
#ifndef RCHECK
		amt = 8;	/* size of first bucket */
		bucket = 0;
#else
		amt = 16;	/* size of first bucket */
		bucket = 1;
#endif
		n = -((uintptr_t)sizeof(*op) + RSLOP);
	} else {
		amt = pagesz;
		bucket = pagebucket;
	}

	while (nbytes > amt + n) {
		amt <<= 1;
		if (amt == 0) /* huge request overflow */
			goto retNULL;
		bucket++;
	}

	_lock_lock(&heap->heap_lock);

	/*
	 * If nothing in hash bucket right now,
	 * request more memory from the system.
	 */

	if ((op = nextf[bucket]) == NULL) {
		morecore(heap, bucket);
  		if ((op = nextf[bucket]) == NULL){
		        kprintf("_km: hash bucket barf\n");
  			goto retNULL;
		}
	}
	/* remove from linked list */
  	nextf[bucket] = op->ov_next;

	_lock_unlock(&heap->heap_lock);

	op->ov_magic = MAGIC;
	op->ov_index = bucket;

#ifdef RCHECK
	/*
	 * Record allocated size of block and
	 * bound space with magic numbers.
	 */
	op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
	op->ov_rmagic = RMAGIC;
  	*(u_short *)((void *)(op + 1) + op->ov_size) = RMAGIC;
#endif
	op++;

	return (void *)op;

retNULL:
	return NULL;
}


/*
 * Allocate more memory to the indicated bucket.
 *
 * this should only be called from _kmalloc, thus the heap should be
 * locked
 */
static void
morecore(heap_t *heap, uintptr_t bucket)
{
  	register union overhead *op;
	register size_t sz;		/* size of desired block */
  	uintptr_t amt;			/* amount to allocate */
  	uintptr_t nblks;			/* how many blocks we get */

	/*
	 * sbrk_size <= 0 only for big, FLUFFY, requests (about
	 * 2^30 bytes on a VAX, I think) or for a negative arg.
	 */
	sz = 1 << (bucket + 3);
	if (sz <= 0)
		return;
	if (sz < pagesz) {
		amt = pagesz;
  		nblks = amt >> (bucket + 3);
	} else {
		amt = sz + pagesz;
		nblks = 1;
	}

	op = (union overhead *)sbrk(amt);


	/* no more room! */
  	if (op == (union overhead *)-1)
	    return;
	/*
	 * Add new memory allocated to that on
	 * free list for this hash bucket.
	 */
  	nextf[bucket] = op;
  	while (--nblks > 0) {
                union overhead * next = (union overhead *)(void *)
                                             ((char *)op + (uintptr_t)sz);

		op->ov_next = next;
		op = next;
  	}
	op->ov_next = NULL;
}

static void
_kfree(heap_t *heap, void * ptr)
{
  	register uintptr_t size;
	register union overhead *op;

	assert(ptr != NULL);


	op = (union overhead *)ptr - 1;
  	assert(op->ov_magic == MAGIC);		/* make sure it was in use */

#ifdef RCHECK
  	assert(op->ov_rmagic == RMAGIC);
	assert(*(u_short *)((void *)(op + 1) + op->ov_size) == RMAGIC);
#endif
  	size = op->ov_index;
  	assert(size < NBUCKETS);

	_lock_lock(&heap->heap_lock);

	op->ov_next = nextf[size];	/* also clobbers ov_magic */
  	nextf[size] = op;

	_lock_unlock(&heap->heap_lock);
}

/*
 * caller must have heap locked
 */
static void *
_libmach_heap_space_sbrk(heap_t *hs, size_t incr)
{
	void * old_brk;

	if (((uintptr_t)hs->hs_current_brk + (uintptr_t)incr) > 
			((uintptr_t)hs + (uintptr_t)hs->hs_size))
		/* the library's heap memory is exhausted -- it's the caller's
		   responsibility to handle this case */
		return (void *)-1;

	old_brk = hs->hs_current_brk;

	hs->hs_current_brk = (void *)((uintptr_t)hs->hs_current_brk + incr);

	return old_brk;
}

void *
kmalloc(size_t size)
{
	return _kmalloc(THE_HEAP, size);
}

void *
kzmalloc(size_t size)
{
	void * new;
	int i;

	new = _kmalloc(THE_HEAP, size);
	if (new != NULL){
	    /* memset(new, 0, size); Orignal - causes exeption 14. */
	    for (i = 0 ; i < size ; i ++)
		((char *)new)[i] = 0;
	}


	return new;
}

void
kfree(void *ptr)
{
	_kfree(THE_HEAP, ptr);
}

/*
 * put chunk back onto freelist
 */
int
kfree_obj ( void * address, size_t size )
{
	freelist_t *ptr, *ptr2, *entry;
	uintptr_t coalesced = 0;

	_lock_lock(&freelist_lock);

	if (_FREELIST == NULL) { /* empty freelist */
		FreeInit(address, size);
		goto freefree_out;
	}
        if (size == 0)
                goto freefree_out;

	/*
	 * find where to put the chunk
	 */
	ptr2 = ptr = _FREELIST;
	do {
		if (ptr->address < address) {
			ptr2 = ptr;
			ptr = ptr->next;
		} else
			break;
	} while (ptr != NULL);

	size = (size_t)page_round_up(size);

	/*
	 * see if we can coalesce
	 */
        coalesced = 0;
        /* Can we join to end of another entry? */
	if ((void *)((uintptr_t)ptr2->address + (uintptr_t)ptr2->size) == address) {
		ptr2->size += size;
		coalesced = 1;
        }
        /* How about at the front? */
        if ((ptr != NULL) 
	    && (ptr->address == (void *)((uintptr_t)address + (uintptr_t)size))) {
                /* This code has caused problems - not sure why - it seems 
                 * reasonable */
		if (coalesced) {
			ptr2->size += ptr->size;
			kfree(ptr);
		} else {
			ptr->size += size;
			ptr->address = address;
		}
	} else if (coalesced == 0) {
		entry = (freelist_t *)_kmalloc(THE_HEAP, sizeof(freelist_t));
		if (!entry)
			return false;   /* No heap space left */
		entry->address = address;
		entry->size = size;
		entry->next = ptr;

		/* sanity check */
		assert( (((uintptr_t)entry->address) % L4_PAGESIZE) == 0 );

		if (ptr == _FREELIST)
			_FREELIST = entry;
		else
			ptr2->next = entry;
	}

freefree_out:
	_lock_unlock(&freelist_lock);
	return true;
}


void *
kmalloc_obj_addr(void * address, size_t size)
{
	freelist_t *ptr, *prev, *tmp;

	_lock_lock(&freelist_lock);

	/*
	 * find chunk that contains our address
	 */
	tmp = _FREELIST;
	prev = ptr = NULL;
	address = page_round_down(address);
	while ((tmp != NULL) && (tmp->address < address)) {
		prev = ptr;
		ptr = tmp;
		tmp = tmp->next;
	}

	if ((tmp != NULL) && (tmp->address == address)) {
		ptr = tmp;
	} else if ((ptr == NULL) || (ptr->address > address)) {
		address = NULL;
		goto kmalloc_obj_addr_out;
	}

	size = (size_t)page_round_up(size);
	if (((size_t)address + size) <= ((size_t)ptr->address + ptr->size)) {
		size_t leftover;

		leftover = (size_t)ptr->address + ptr->size - 
			   ((size_t)address + size);

		if (ptr->address == address) {
			if (leftover == 0) {
				if (prev != _FREELIST)
					prev->next = ptr->next;
				else
					_FREELIST = NULL;

				kfree(ptr);
			} else {
				ptr->address = (void *)((uintptr_t)address + (uintptr_t)size);
				ptr->size -= size;
			}
			goto kmalloc_obj_addr_out;
		} else {
			ptr->size = (size_t)address - (size_t)ptr->address;

			if (leftover != 0) {
				tmp = _kmalloc(THE_HEAP, sizeof(freelist_t));
	      
				tmp->size = leftover;
				tmp->next = ptr->next;
				ptr->next = tmp;
				tmp->address = (void *)((uintptr_t)address + (uintptr_t)size);

				goto kmalloc_obj_addr_out;
			}
		}
	}

kmalloc_obj_addr_out:
	_lock_unlock(&freelist_lock);
	return address;
}


/*
 * Allocate size bytes from the freelist
 */
void *
kmalloc_obj(size_t size)
{
	freelist_t *ptr, *ptr2, *fit;
	void *address = NULL;

	_lock_lock(&freelist_lock);

	if (_FREELIST == NULL)
		goto kmalloc_obj_out;
        if (size == 0)
                goto kmalloc_obj_out;

	/* we will use first fit here 
	   best fit is usually best, but then we may
	   loose the ability to resize most objects
	   worst will let us resize most of the time,
	   but will cause excessive fragmentation. Thus
	   it will be this way until we have proper mm,
	   in which case we can do random fit :)
	   */
	ptr = ptr2 = _FREELIST;
	fit = NULL;
	size = (size_t)page_round_up(size);
	do {	/* find the first entry that is big enough */
		if (ptr->size >= size) {
			fit = ptr;
			break;
		}
		/* sanity check the freelist as we go */
		assert( (((uintptr_t)ptr->address) % L4_PAGESIZE) == 0 );
		assert( (((uintptr_t)ptr->size)    % L4_PAGESIZE) == 0 );
		ptr2 = ptr;
		ptr = ptr->next;
	} while (ptr != NULL);

	if (fit == NULL)
		goto kmalloc_obj_out;

	address = fit->address;
	if (fit->size != size) {
		fit->address = (void *)((uintptr_t)fit->address + (uintptr_t)size);
		fit->size -= size;
	} else { /* exact fit of free chunk */
		if (fit == _FREELIST)    /* We are at the start of the list */
			_FREELIST = fit->next;
		else
			ptr2->next = fit->next;
		kfree(fit);
	}

kmalloc_obj_out:
	_lock_unlock(&freelist_lock);

	return address;
}


/*
 * initialisation code below only
 */
static void
FreeInit(void * address, size_t size)
{
        kprintf("mungi: mm.c: calling kmalloc\n");
	_FREELIST = _kmalloc(THE_HEAP, sizeof(freelist_t));
        kprintf("mungi: mm.c: called kmalloc\n");
	if (_FREELIST == NULL)
		panic("Could not create Freelist");

	_FREELIST->address = address;
	_FREELIST->size = size;
	_FREELIST->next = NULL;
        kprintf("mungi: mm.c: leaving FreeInit\n");

}


void
mm_init(l4_threadid_t rpager_id,void *stored_heap,void *stored_freelist){
        void *heapsplit;
        rpagerid = rpager_id; /* For the Mungi pager */

        if (stored_heap == NULL){
                VERBOSE("mungi: Kernel heap from %p - 0x%lx bytes\n",
                                HEAP_BASE,HEAP_SIZE); 
                kprintf("libmach\n");
                THE_HEAP = _libmach_heap_new(HEAP_BASE,HEAP_SIZE);
                kprintf("lockint\n");
                _lock_init(&freelist_lock);
                FreeInit(l4_setup.memory_start,OBJECT_REGION_SIZE);
        } else if ((uintptr_t)stored_heap == -1){
	        /* No I/O system, no paging, no backup system */
                heapsplit = (void *)(((uintptr_t)l4_setup.memory_end - 
                                (uintptr_t)l4_setup.memory_start)/2);
                VERBOSE("Kernel heap from: %p to %p\n", heapsplit, 
                                l4_setup.memory_end);
                VERBOSE("Object allocation from: %p to %p\n",
                                l4_setup.memory_start, heapsplit);
                THE_HEAP = _libmach_heap_new(heapsplit, 
                                (size_t)((uintptr_t)l4_setup.memory_end 
                                         - (uintptr_t)heapsplit)); 
                FreeInit(l4_setup.memory_start, (size_t)((uintptr_t)heapsplit - 
                       (uintptr_t)l4_setup.memory_start));

        } else {
	        kprintf("Using preexisting data\n");
        	THE_HEAP = stored_heap;
	        _FREELIST = stored_freelist;
        	_lock_init(&freelist_lock);
        }
        FreePrint();
        kprintf("mm_init done\n");
    
}


static heap_t *
_libmach_heap_new(void * address, size_t size)
{
	heap_t *newheap;
	uintptr_t bucket, i;
	uintptr_t amt;

	newheap = (heap_t *)address;
        kprintf("libmach lock: %p,lock %p\n",newheap,&newheap->heap_lock);
	_lock_init(&newheap->heap_lock);
        kprintf("libmach lock done\n");

	newheap->hs_size = size;
	newheap->heap_pagesz = L4_PAGESIZE;

	/* page align brk */
	newheap->hs_current_brk = page_round_up((uintptr_t)address + 
					      (uintptr_t)sizeof(heap_t));
	
	for (i = 0; i < NBUCKETS; i++)
		newheap->heap_nextf[i] = NULL;
  
	/* find the bucket for a pagesize chunk */
	bucket = 0;
	amt = 8;
	while (amt < newheap->heap_pagesz) {
		amt <<= 1;
		bucket++;
	}
	newheap->heap_pagebucket = bucket;
	return newheap;
}

static void
FreePrint(void)
{
	freelist_t *p;

	for (p = _FREELIST; p; p = p->next)
                VERBOSE("FreeChunk at %p to %lx\n", p->address, 
                                ((uintptr_t)p->address + (uintptr_t)p->size));
}

void
memory_cleanup(void)
{
	l4_msgdope_t result;

	for(;;) {
		l4_ipc_sleep(L4_IPC_NEVER, &result);
	}
}

void
save_heap_data(void **the_heap,void **the_freelist){
        *the_heap = (void *)THE_HEAP;
        *the_freelist = (void *)_FREELIST;
}






