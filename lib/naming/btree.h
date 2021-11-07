/****************************************************************************
 *
 *      $Id: btree.h,v 1.2 2002/05/31 07:56:35 danielp Exp $
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

/* B+ tree implementation.

   Written by Ruth Kurniawati, Oct/Nov 1995.
   Adapted by Jerry Vochteloo in April 1996.
   Adapted by Gernot Heiser <G.Heiser@unsw.edu.au> on 97-02-08.
   Modified by Charles Gray <cgray@cse.unsw.edu.au> on 01-09-24
 */

/*
  In order to use this B+ tree implementation, you'll need to:
*  - define a type "BTkey", which is the type of the keys to the
     B+ tree. 
*  - Define the following macros for dealing with keys:
        BTGetObjKey(object)	returns the key to object
	BTKeyLT(key1, key2)	key1 <  key2
	BTKeyGT(key1, key2)	key1 >  key2
	BTKeyEQ(key1, key2)	key1 == key2
*  - define a type "BTObject", which should be a pointer to a struct
     containing the data to be indexed.
     It is the pointer values which will be stored in the B+ tree.
*  - provide a (possibly empty) function
        void BTPrintObj(BTObject const obj);
     which will be used for printing your objects by "BPPrint".
*  - defined a type "PagePool". Pointers to this type are contained
     in B+ tree references. This ensures that pages are allocated from the
     correct pool, in case several B+ trees are used
   - define functions
        struct sBTPage *alloc_page(PagePool *pool);
	void free_page(PagePool *pool, struct sBTPage *page);
     for (de)allocating pages for the B+ tree.
 */


#ifndef BTREE_H
#define BTREE_H

#include "bt_types.h"



/* B-tree parameters */
#ifdef ALPHAENV

/* For 8kb pages we use a ???? tree */
#define PAGE_SIZE 8192
#define BT_ORDER  255
/* #define BT_ORDER  10 */
#define BT_MAXKEY (BT_ORDER - 1)
#define BT_MINKEY (BT_MAXKEY >> 1)
#endif

#ifdef MIPSENV

/* For 4kb pages we use a 128-255 tree */
#define PAGE_SIZE 4096
#define BT_ORDER  255
#define BT_MAXKEY (BT_ORDER - 1)
#define BT_MINKEY (BT_MAXKEY >> 1)

#endif
typedef int   BTKeyCount;

/* one B-Tree page */

struct sBTPage 
{
  BTKeyCount count;          /* number of keys used */
  boolean    isleaf;         /* true if the page is the 
				b+tree index's leaf page */
  BTKey      key[BT_ORDER - 1];
  struct     sBTPage* child[BT_ORDER];
};

typedef struct sBTPage BTPage;

typedef struct sBTree
{
    BTPage    *root;
    PagePool  *pool;
    int       depth;
} BTree_S;
typedef BTree_S *BTree;


/* b+tree operations */
void BTPrint(BTree const btree);
/* Print a B-tree */

int BTSearch(BTree const btree, BTKey const key, BTObject *obj);
/* Search "key" in "btree", return object through "obj".
   Returns BT_FOUND      if successfull,
           BT_INVALID    if invalid "btree",
           BT_NOT_FOUND  if key not in B-tree.
 */

int BTIns(BTree const btree, BTObject const obj);
/* Insert "obj" in "btree".
   Returns BT_FOUND       if successfull,
           BT_INVALID     if invalid "btree",
           BT_ALLOC_FAIL  if out of memory,
	   BT_DUPLICATE   if "key" already exists in B-tree,
           BT_NOT_FOUND   if key not in B-tree.
 */

int BTDel(BTree const btree, BTKey const key, BTObject *obj);
/* Delete object with "key" in "btree".
   Returns BT_FOUND       if successfull,
           BT_INVALID     if invalid "btree",
           BT_NOT_FOUND   if key not in B-tree.
 */

int BTModify(BTree const btree, BTKey const key, BTObject **obj);
/* Return address of object with "key" in "btree".
   Returns BT_OK          if successfull,
           BT_INVALID     if invalid "btree",
           BT_NOT_FOUND   if key not in B-tree.
 */

int BTEnum( BTree const btree, BTObject out[], int len, 
	    BTObject find, int mode );
/* Returns # items copied into out[] array */

int BTEmpty( BTree const btree );
/* Returns if the BTree is empty
   Returns BT_OK          if NOT empty
           BT_INVALID     if invalid "btree",
           BT_NOT_FOUND   if empty
 */



/* possible return values */
#define BT_FOUND 0
#define BT_OK    0

#define BT_INVALID    1
#define BT_NOT_FOUND  2
#define BT_DUPLICATE  3
#define BT_ALLOC_FAIL 4    /* running out of memory */

/* Internal return codes: */
#define BT_PROMOTION     5 /* place the promoted key */
#define BT_LESS_THAN_MIN 6 /* redistribute key from child */
#endif
