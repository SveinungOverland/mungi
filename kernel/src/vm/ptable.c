/****************************************************************************
 *
 *      $Id: ptable.c,v 1.16 2002/08/01 03:28:17 cgray Exp $
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

/* L4 includes */
#include "mungi/l4_generic.h"

/* mungi includes */
#include "mungi/kernel.h"

/* virtual memory includes */
#include <vm/vm_types.h>
#include <vm/mutex.h>
#include <vm/ptable.h>

//#define PT_DBG
//#define ADBG

#ifdef ADBG
#define debug(x...) kprintf(x)
int	result;
#else
#define debug(x...)
#endif



page_table_t *add_pte(uintptr_t vpno, uintptr_t pfno, char mode);
extern page_table_t *ptable, *ctable;
extern frame_table_t *ftable;
extern unsigned int ftable_size, ctable_size, ptable_size;
static page_table_t *add_collision(uintptr_t vpno, uintptr_t pfno, char mode,
				   page_table_t * pte);
void 
invalidate_pte(unsigned int vpno) {
    page_table_t *pte;
    unsigned int pindex = HASH(vpno, MAX_PTABLE); 
    bittestandset(&ptable[pindex], PT_LOCK_MASK);
    pte = &ptable[pindex];
    if (pte->up.chain != MAX_CTABLE)
        pte->up.valid = DELETED;
    else 
        pte->up.valid = INVALID;
    pte->up.locked = 0;
}
  
/*
 * note: if a valid pte is found, it is returned *locked*.
 * calller of find_pte must unlock this pte once it has finished with it.
 */
page_table_t *
find_pte(unsigned int vpno)
{
	unsigned int pindex;
	page_table_t *pte;

        assert(ptable_size != 0);
        pindex = HASH(vpno, MAX_PTABLE);
 
	//bittestandset(&ptable[pindex], PT_LOCK_MASK);
	pte = &ptable[pindex];
	debug("find_pte: locked1 0x%p\n", pte);
	if (pte->up.valid == VALID && pte->up.pagenum == vpno) {
#ifdef ADBG
	    debug("find_pte: 0x%x:0x%x\n", vpno, pte->up.framenum);
#endif
	    return (pte);
        }
	if (pte->up.valid != INVALID) {
                /* need to check collision table */
                while (pte->up.pagenum < vpno && pte->up.chain != MAX_CTABLE) {
                        /* keep checking until pass where vpno should be
	                   or reach end of chain */
                        pte->up.locked = 0;
                        debug("find_pte: unlocked 0x%llx\n", pte);
                        pte = &ctable[pte->up.chain];
#ifdef PT_DBG
                        assert(pte->up.valid == VALID);
#endif
                        //bittestandset(pte, PT_LOCK_MASK);
                        debug("find_pte: locked2 0x%llx\n", pte);
                        if (pte->up.pagenum == vpno) {
#ifdef ADBG
                                debug("find_pte: (in ctable) 0x%x:0x%x\n",vpno,
                                                pte->up.framenum);
#endif
		                return pte;
	                }
                }				
	   			
	}		
	pte->up.locked = 0;
	return NULL;
}

page_table_t *
add_pte(uintptr_t vpno, uintptr_t pfno, char mode)
{
	unsigned int pindex = HASH(vpno, MAX_PTABLE);
	used_page_t *page;

	debug("add_pte: pindex = %d, ", pindex);
        debug("vpno = 0x%x, pfno = 0x%x\n", (unsigned int)vpno, 
                        (unsigned int)pfno);

	/* sanity check */
	if (pindex < 0 || pindex >= MAX_PTABLE) {
		debug("add_pte: invalid page!\n");
		return (page_table_t *) - 1;
	}

	page = &ptable[pindex].up;

	//bittestandset(page, PT_LOCK_MASK);
	if (page->valid != VALID) {
		/* no collision - just insert */
		page->valid = VALID;
		ftable[pfno].dirty = mode;
		page->pagenum = vpno;
		page->framenum = pfno;
		page->chain = CTABLE_LIST_END;
		page->locked = 0;
		return (&ptable[pindex]);
	} else {					/* collision */
		page->locked = 0;
		return add_collision(vpno, pfno, mode, &ptable[pindex]);
	}
}

/* assumes vpno is not already present */
static page_table_t *
add_collision(uintptr_t vpno, uintptr_t pfno, char mode, page_table_t * pte)
{
	used_page_t *page = &pte->up;
        used_page_t *prev = NULL;
	page_table_t *new;
	unsigned int cindex;

	//bittestandset(&ctable[MAX_CTABLE], PT_LOCK_MASK);
	/* get next free collision table entry */
	cindex = ctable[MAX_CTABLE].fp.next;

#ifdef PT_DBG
	debug("adding collision page 0x%x at %d\n", vpno, cindex);
	/* collision table full */
	assert(cindex != CTABLE_LIST_END);
#endif
        if (cindex == CTABLE_LIST_END){
                debug("Out of memory error\n");
                debug("Occurred:" __FILE__ __LINE__ "\n");
                assert(!"Out of collision table\n");
                return NULL;
        }
        
	new = &ctable[cindex];		/* next free entry */
	ctable[MAX_CTABLE].fp.next = new->fp.next;	/* update free list head */
	ctable[MAX_CTABLE].fp.locked = 0;

        if (new->up.valid == VALID){
                debug("This frame allready allocated!!\n");
                assert(!"Frame allready allocated\n");
                return NULL;
        }
        
        /* fill in new entry */
	new->up.valid = VALID;
	new->up.locked = 0;   /* FIXME: should this be locked?? */
	ftable[pfno].dirty = mode;
	new->up.pagenum = vpno;
	new->up.framenum = pfno;
	/* do ordered insertion */

	/* find correct insertion spot */
	//bittestandset(page, PT_LOCK_MASK);
	while (page->pagenum < vpno && page->chain != CTABLE_LIST_END) {
                if (vpno == page->pagenum){
                        debug("vpno in collision table\n");
                        assert(!"vpno in collision table\n");
                        return NULL;
                }
		page->locked = 0;
                prev = page;
                page = &ctable[page->chain].up;
		//bittestandset(page, PT_LOCK_MASK);
	}

        /* insert */
        if (prev != NULL){
                new->up.chain = prev->chain;
                prev->chain = cindex;
        } else {
                new->up.chain = page->chain;
                page->chain = cindex;
        }
	page->locked = 0;

	return new;
}


#if 0
void pte_print(const page_table_t * pte)
{
	aprintf("vpno 0x%x, pfno 0x%x, ",
			pte->up.pagenum, pte->up.framenum);
#if 0
	switch (pte->up.mode) {
	case R_PAGE:
		aprintf("mode READ\r\n");
		break;
	case RW_PAGE:
		aprintf("mode READ/WRITE\r\n");
		break;
	case COW_PAGE:
		aprintf("mode COW\r\n");
		break;
	}
#endif
}
#endif
