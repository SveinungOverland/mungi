/****************************************************************************
 *
 *      $Id: mm.h,v 1.6 2002/08/26 07:03:08 cgray Exp $
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

#ifndef __M_MM_H 
#define __M_MM_H

#include "mungi/types.h"
#include "mungi/l4_generic.h"

void mm_init(l4_threadid_t,void *,void *);
void memory_cleanup(void);

/* By Brett Nash */
/* From the MIPS manual to work out maximum usable address space *
 * Then subtract some for I/O space */
#define ADDRESS_SPACE_END (uintptr_t)(0x700000000)
#define ADDRESS_SPACE_BEGIN (uintptr_t)(0x1000000)

/* Heap: This is Mungis own data structure region */
#define HEAP_SIZE       (uintptr_t)(0x4000000)
#define HEAP_BASE       (void *)(ADDRESS_SPACE_END - HEAP_SIZE)

/* Object Region */
#define OBJECT_REGION_BEGIN       ADDRESS_SPACE_BEGIN
#define OBJECT_REGION_SIZE        (uintptr_t)(ADDRESS_SPACE_END \
					      - HEAP_SIZE       \
					      - OBJECT_REGION_BEGIN)

/* The `MAGIC' IO address - the first disk is mapped here */
#define MAGIC_IO_BEGIN  ADDRESS_SPACE_END
#define MAGIC_IO_END    (uintptr_t)(0x800000000)
#define MAGIC_IO_SIZE   (long long)(0x100000000)

/* End nash defines */


/* malloc/free for kernel data structures 
 * memory returned by kzmalloc is zeroed
 */
void * kmalloc(size_t size);
void * kzmalloc(size_t size);
void kfree(void * ptr);

/* malloc/free for objects */
void *kmalloc_obj(size_t size);
void *kmalloc_obj_addr(void * address, size_t size);
int kfree_obj(void * address, size_t size);

/* Save heap & Freelist */
void save_heap_data(void **the_heap,void **the_freelist);

#endif /* __M_MM_H */
