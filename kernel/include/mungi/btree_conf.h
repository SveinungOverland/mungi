/****************************************************************************
 *
 *      $Id: btree_conf.h,v 1.5 2002/08/23 08:24:19 cgray Exp $
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


#ifndef __BTREE_CONF_H__
#define __BTREE_CONF_H__

#include "mungi/types.h"
#include "mungi/objects.h"

typedef uintptr_t    BTKey;

#define BPIns BTIns
#define BPSearch BTSearch
#define BPDel BTDel

typedef object_t BTObject;
typedef BTObject *GBTObject;

#define MAX_POOL_SIZE 1000
typedef struct PAGE_POOL {
  char         *base;
  unsigned int size; /* pool size in pages, == bitmap size in bits */
  unsigned int n_allocs, n_frees;
  unsigned int bitmap[MAX_POOL_SIZE/(8*sizeof(int))];
} PagePool;

extern struct sBTPage *alloc_page(PagePool *pool);

extern void free_page(PagePool *pool, struct sBTPage *page);


/* Defines for B-tree: */
/* For 4kb pages we use a 128-255 tree */
#define BT_ORDER  255

#define BTGetObjKey(object) (((uintptr_t)((object)->base)))
#define BTKeyLT(key1, key2) ((key1) <  (key2))
#define BTKeyGT(key1, key2) ((key1) >  (key2))
#define BTKeyEQ(key1, key2) ((key1) == (key2))

#define BTObjMatch(obj, ky) ((ky) >= ((uintptr_t)((obj)->base)) && \
			    (ky)<((uintptr_t)(obj)->end))
#define BTOverlaps(o1,o2) (((uintptr_t)(o1)->base <= (uintptr_t)(o2)->base && \
			   ((uintptr_t)(o2)->base < (uintptr_t)(o1)->end)) || \
			   ((uintptr_t)(o2)->base <= (uintptr_t)(o1)->base && \
			    (uintptr_t)(o1)->base < (uintptr_t)(o2)->end))
#endif /* _BTREE_CONF_H_ */
