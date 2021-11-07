/****************************************************************************
 *
 *      $Id: vm.h,v 1.8.2.1 2002/08/30 05:59:58 cgray Exp $
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

#ifndef _VM_H_
#define _VM_H_

/* vm.h
 * Should only be included by RAM pager
 */

#include <vm/vm_types.h>
#include "mungi/l4_generic.h"

/* convenient macros */

/* physical frame number  to frame table index 
 *  x : physical frame number
 */
#define PF2IND(x) (x - fbase)

/* physical address to frame table index 
 *  x : physical address
 */
#define PA2IND(x) PF2IND(((x) >> L4_LOG2_PAGESIZE)) 

/* frame table index to frame number 
 *  x : frame table index
 */
#define IND2PF(x) (x + fbase)

/* frame table index to physical frame address 
 *  x : frame table index
 */
#define IND2PA(x) (((unsigned long)IND2PF(x)) << L4_LOG2_PAGESIZE)

#define NO_FREE_FRAMES CTABLE_LIST_END /* returned by alloc_frame() */

#define PT_SEARCHED_MASK (unsigned long)0x8000000000000000 /* used to see if  pte lookup done yet */

/* States used by Save STATE */
#define GO_FIND_PAGE		0
#define GO_FIND_FREE_FRAME 	1
#define GO_FIND_PAGE_DATA 	2
#define GO_LOAD_PAGE_DATA	3
#define GO_ADD_ENTRY		4
#define GO_MAP_PAGE			5

/* IO System status */
#define IO_NONE                 0
#define IO_NORMAL               1
#define IO_NORMAL_RESTART       2


/* mask bits used in set/clr_ft_bit() */
#define FT_DIRTY ((unsigned int) 4)
#define FT_BUSY ((unsigned int) 8)
#define FT_OSRSRVD ((unsigned int) 16)
#define FT_REFERENCE ((unsigned int) 32) 
#define FT_MAPPED ((unsigned int)64)

/* Defines used by save_state*/
#define ALREADY_REQUESTED -1

/* function prototypes */
extern void vm_init(void);
extern page_table_t *add_pte(uintptr_t vpno, uintptr_t pfno, char mode);
extern unsigned int alloc_frame(unsigned int vpno, unsigned int osrsrvd, 
				unsigned int dirty);  
extern unsigned int force_frame(unsigned int vpno, unsigned int osrsrvd, 
				unsigned int dirty);
unsigned int alloc_frame_off_list(freelist *);
int alloc_mpr_frames(void);
void add_frame_to_list(freelist *, unsigned int);
void add_to_free_list(unsigned int pfn);

extern void set_ft_bit(unsigned int pfno, unsigned int mask);
extern void clr_ft_bit(unsigned int pfno, unsigned int mask);
void set_ft_bit_nolock(unsigned int pfno, unsigned int mask); /* Warning - dangerous! */


void kprintfd(void);
void debug(char *,...);
int page_block_out(uint32_t,uint32_t,int);
int page_block_in(uint32_t,uint32_t,int);
int save_state(l4_threadid_t faulter,l4_threadid_t faultfor,
                uintptr_t fault_page,access_t access);


/* global variables */
extern unsigned int ptable_size ; /* page table size, in bytes  */
extern unsigned int ctable_size; /* collision table size, in bytes */
extern page_table_t *ptable; /* page table */
extern page_table_t *ctable; /* collision table */
extern frame_table_t *ftable;
extern uintptr_t fbase;


/* Threads */
extern l4_threadid_t rpagerid;			/* RAM pager (this thread) id */
extern l4_threadid_t mpagerid;			/* mungi pager id */
extern l4_threadid_t apagerid;			/* asynchronous pager id */
extern l4_threadid_t mprid;                   /* smarter filesystem */
extern l4_threadid_t adiskid;                 /* asynchronous disk */
extern l4_threadid_t kprintfdid;               /* handles the printf */
extern l4_threadid_t startupid;                /* starts up other threads */
extern l4_threadid_t serverid;		/* Mungi server id */



	
/* Thread Stacks */
#define STACK_SIZE 1024 
uintptr_t startup_stack[STACK_SIZE];
uintptr_t mpr_stack[STACK_SIZE];
uintptr_t kprintfd_stack[STACK_SIZE];
uintptr_t adisk_stack[STACK_SIZE];
#endif




