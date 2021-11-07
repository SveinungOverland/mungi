/****************************************************************************
 *
 *      $Id: vm_dbg.c,v 1.3 2002/05/31 06:27:58 danielp Exp $
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

#if 0
#include <assert.h>
#include <vm/kprintf.h>
#include <vm/vm_dbg.h>

#define ADBG

#ifdef ADBG
#include <vm/kprintf.h>
#define kprintf(x...) aprintf(x)
#else
#define kprintf(x...) 
#endif

static void chain_dump(const page_table_t *ctable, unsigned int ctable_size, 
		       const page_table_t *pte);

extern void
pt_dump(const page_table_t *ptable, unsigned int max_ptable, 
	const page_table_t *ctable, unsigned int max_ctable)
{
    unsigned int i;

    kprintf("*** valid ptable entries start ***\n");
    for (i = 0; i < max_ptable; i++)
	if (ptable[i].up.valid == VALID) {
	    kprintf("0x%x: vpno 0x%x, pfno 0x%x, ",
		    i, ptable[i].up.pagenum, ptable[i].up.framenum);

	    switch (ptable[i].up.mode) {
	    case R_PAGE:
		kprintf("mode READ\n");
		break;
	    case RW_PAGE:
		kprintf("mode READ/WRITE\n");
		break;
	    case COW_PAGE:
		kprintf("mode COW\n");
		break;
		
	    }
	    chain_dump(ctable, max_ctable, &ptable[i]);
	}
    kprintf("*** valid ptable entries end ***\n");
}

extern void
ct_dump(const page_table_t *ctable, unsigned int max_ctable)
{
    unsigned int i;

    kprintf("*** valid ctable entries start ***\n");
    for (i = 0; i < max_ctable; i++)
	if (ctable[i].up.valid == VALID) {
	    kprintf("0x%x: vpno 0x%x, pfno 0x%x, ",
		    i, ctable[i].up.pagenum, ctable[i].up.framenum);

	    switch (ctable[i].up.mode) {
	    case R_PAGE:
		kprintf("mode READ\n");
		break;
	    case RW_PAGE:
		kprintf("mode READ/WRITE\n");
		break;
	    case COW_PAGE:
		kprintf("mode COW\n");
		break;
		
	    }
	}
    kprintf("*** valid ctable entries end ***\n");
}

static void
chain_dump(const page_table_t *ctable, unsigned int max_ctable, 
	   const page_table_t *pte)
{
    unsigned int i;
    
    while (pte->up.chain != max_ctable) {
	i = pte->up.chain;
	pte = &ctable[i];
	assert(pte->up.valid == VALID);
	kprintf("\t0x%x: vpno 0x%x, pfno 0x%x, ",
		i, pte->up.pagenum, pte->up.framenum);
	
	switch (pte->up.mode) {
	case R_PAGE:
	    kprintf("mode READ\n");
	    break;
	case RW_PAGE:
	    kprintf("mode READ/WRITE\n");
	    break;
	case COW_PAGE:
		kprintf("mode COW\n");
		break;
		
	}
    }
}

static char tmp[14650]; // hack: hardcoded for 64MB (due to stack problems)

extern void
ft_dump(const frame_table_t *ftable, unsigned int max_ftable)
{
    unsigned int ff = max_ftable, i;

    for (i = 0; i < max_ftable; i++)
	tmp[i] = 0;

    /* mark free frames */
    while ((ff = ftable[ff].ff.next) != max_ftable)  	
	tmp[ff] = 1;

    /* dump used frames */
    kprintf("*** used frames start ***\n");

    for (i = 0; i < max_ftable; i++) 
	if (tmp[i] == 0) 
	    kprintf("%d: vpno 0x%x, ref %d, osr %d, busy %d, d %d\n",
		    i, ftable[i].uf.pagenum, ftable[i].uf.reference,
		    ftable[i].uf.osrsrvd, ftable[i].uf.busy, 
		    ftable[i].uf.dirty);

    kprintf("*** used frames end ***\n");
}

#endif
