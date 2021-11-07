/****************************************************************************
 *
 *      $Id: vm_types.h,v 1.8.2.1 2002/08/30 05:59:58 cgray Exp $
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

#ifndef _VM_TYPES_H
#define _VM_TYPES_H

#include <mungi/l4_generic.h>

#define DELETED 2
#define VALID 1
#define INVALID 0

/* Magics used in communication, note both odd :) */
#define PAGER_MAGIC	(uintptr_t)-31
#define SHUTDOWN_MAGIC  (uintptr_t)-37

/* No more deceiving! */
#define MPAGER_MAGIC    ((uintptr_t)7)

#define MAX_PTABLE (ptable_size / sizeof(page_table_t))
#define MAX_CTABLE (ctable_size / sizeof(page_table_t))
#define MAX_FTABLE (ftable_size / sizeof(frame_table_t))
#define CTABLE_LIST_END MAX_CTABLE
#define FTABLE_LIST_END MAX_FTABLE
#define PT_LOCK_MASK (unsigned long)0x4000000000000000 /* used in 
							  bitestandset */
#define FT_LOCK_MASK (unsigned long)0x0000000000000001 /* used in 
							  bitestandset */
#define NO_PTE (unsigned long)0xffffffffffffffff /* tell RAM pager 
						    no PTE found */
#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif
							
#define HASH(x, y) (x % y)

typedef struct {
    unsigned long valid : 2;
    unsigned long locked : 1; /* synchronisation */
    unsigned long pagenum : 36;
    unsigned long framenum : 20;
    unsigned long chain : 15; /* collision chain */
} used_page_t;

/* free_page_t: used to keep track of free entries in collision table */
typedef struct {
    unsigned long valid : 1;
    unsigned long locked : 1;
    unsigned long next : 62; /* next free; not all bits used */
} free_page_t;

typedef union {
    used_page_t up;
    free_page_t fp;
    unsigned long  page;
} page_table_t;

#define NIL_BLK  (unsigned long)0xffffffff
#define REFERENCED 1
#define NOT_REFERENCED 0
#define OSRSRVD 1
#define NON_OSRSRVD 0
#define BUSY 1
#define NOT_BUSY 0
#define DIRTY 1
#define CLEAN 0
#define COW_PAGE 1
#define WRITEABLE 0

/* Should be 64 bits in size */
typedef struct {
    uint64_t pad      :11;  
    uint64_t freelist :20; /* freelist index */
    uint64_t pagenum  :28; /* The virtual page (40-12 bits) */
    uint64_t reference: 1; /* mapped */
    uint64_t osrsrvd  : 1; /* mpr, rpager or mungi resevred page */
    uint64_t dirty    : 1; /* dirty page */
    uint64_t cow      : 1; /* Is mapped Copy on write */
    uint64_t lock     : 1; 
} frame_table_t;


/* BSN: max number of Pending Page Faults */
#define MAX_PF 1024
#define PPF_NIL MAX_PF

/* this needs to optomized */
typedef struct pending_pf {    
        l4_threadid_t faulter;
        l4_threadid_t faultfor;
        uintptr_t fault_page;
        access_t access;
        int valid;
        struct pending_pf *collision;
} pending_pf_t;

#define CTABLE_SHIFT 2 /* used to work out size of collision table */   
typedef struct {
  uint64_t fill:1;
  uint64_t front:31;
  uint64_t back:31;
  uint64_t lock:1;
} freelist;

struct dirty_list {
        uint64_t frame;
        uint64_t page;
};

#define MFS_PENDING_MAX	256

#endif




