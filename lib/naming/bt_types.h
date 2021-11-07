/****************************************************************************
 *
 *      $Id: bt_types.h,v 1.2 2002/05/31 07:56:34 danielp Exp $
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

/* Type definitions for namespace B-tree */

#ifndef BT_TYPES_H
#define BT_TYPES_H

#include <mungi.h>
#include "nstree.h"

#undef true
#undef false

typedef enum {false, true} boolean;

typedef unsigned long int64;		/* NOTE: MUST be a 32-bit integer! */

typedef int64     BTKey;
typedef nsent_t*  BTObject;

#define max(x,y) ((x>y) ? x : y)
#define min(x,y) ((x<y) ? x : y)

#define BTGetObjKey(object) ((BTKey)(object)->hash)

/* key comparison */
#define BTKeyLT(key1, key2) (key1 < key2)
#define BTKeyGT(key1, key2) (key1 > key2)
#define BTKeyEQ(key1, key2) (key1 == key2)
/* we manage key clashes externally so equal if keys are equal */
#define BTObjMatch(obj, ky) (BTGetObjKey(obj) == key)

/* New, improved, now with Vitamin R Enum macros */
#define BTE_ENUM  0
#define BTE_RLOOK 1
/*
#define BTEnumMatch( tree_obj, ref_obj, match )                   \
                           ( ( (match) == BTE_ENUM ) ?            \
                            ( BTGetObjKey((BTObject)tree_obj) >=  \
			      BTGetObjKey(ref_obj) ) :            \
			    (((BTObject)tree_obj)->value ==   \
			     ((BTObject)ref_obj )->value ) )
*/
#define BTEnumMatch( tree_obj, ref_obj, match )                   \
                            ( BTGetObjKey((BTObject)tree_obj) >=  \
			      BTGetObjKey(ref_obj) )


#define MAX_POOL_SIZE 2048
typedef struct PAGE_POOL 
{
	void         *heap;  /* hmalloc heap */
	unsigned int n_allocs, n_frees;
} PagePool;


#endif /* !BT_TYPES_H */

