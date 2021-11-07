/****************************************************************************
 *
 *      $Id: btree.h,v 1.3 2002/05/31 05:49:36 danielp Exp $
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
   Adapted by Gernot Heiser <G.Heiser@unsw.edu.au> on 97-02-08:
    - handle intervals in B-tree code
    - completely separate object handling
 */

/*
  This implementation does not care what the objects are, these are completely
  under user control (who might, for example, maintain a secondary access
  structure such as a linked list). Hence all allocation/deallocation
  of objects is left to the user.

  The user is also given full control over storage allocation for actual
  B-tree nodes. This is done by using the alloc_page, free_page functions,
  and a "page pool" pointer as part of the B-tree datastructure.

  The B-tree can operate as usual, storing objects identified by a single
  key value. Alternatively, it can be set up for storing non-overlapping
  interval objects, i.e. each object occupying an interval of key space.

  In order to use this B+ tree implementation, you'll need to (in btree_conf.h):
   - define a type "BTkey", which is the type of the keys to the
     B+ tree.
   - define "BT_ORDER" as the maximum spread of the B-tree.
     Must be odd.
   - Define the following macros for dealing with keys:
        BTGetObjKey(object)	returns the key to object
	BTKeyLT(key1, key2)	key1 <  key2
	BTKeyGT(key1, key2)	key1 >  key2
	BTKeyEQ(key1, key2)	key1 == key2
     If you want the B+ tree to store objects taking up an interval of
     key space (rather than just a single value), define
	BTGetObjLim(object)     returns upper limit key value (i.e.
	                        the next value available as key for another
				object)
	BTObjMatch(obj, key)	key  IN obj (if the key lies within the
	                        object, i.e. obj->key <= key < obj->limit)
	BTOverlaps(obj1, obj2)  objects overlap.
     For normal B+-trees, leave those undefined.
   - define a type "GBTObject", which should be a pointer to a struct
     containing the data to be indexed.
     It is the pointer values which will be stored in the B+ tree.
   - provide a (possibly empty) function
        void BTPrintObj(GBTObject const obj);
     which will be used for printing your objects by "BTPrint".
   - defined a type "PagePool". Pointers to this type are contained
     in B+ tree references. This ensures that pages are allocated from the
     correct pool, in case several B+ trees are used
   - define functions
        struct sBTPage *alloc_page(PagePool *pool);
	void free_page(PagePool *pool, struct sBTPage *page);
     for (de)allocating pages for the B+ tree.
 */


#ifndef BTREE_H
#define BTREE_H

#include "mungi/btree_conf.h"


extern void BTPrintObj(GBTObject const obj);

#define BT_MAXKEY (BT_ORDER - 1)
#define BT_MINKEY (BT_MAXKEY >> 1)

#if !defined BTObjMatch && !defined BTGetObjLim && !defined BTOverlaps
# define BTObjMatch(obj,key) (BTKeyEQ(BTGetObjKey((obj)),key))
# define BTGetObjLim(obj)    BTGetObjKey(obj)
# define BTOverlaps(o1,o2)   (BTKeyEQ(BTGetObjKey(o1),BTGetObjKey(o2)))
# undef  BT_HAVE_INTERVALS
#else
# define BT_HAVE_INTERVALS
#endif

typedef int   BTKeyCount;

/* one B-Tree page */

struct sBTPage {
  BTKeyCount		count;           /* number of keys used */
  bool			isleaf;          /* true if this is a leaf page */
  BTKey			key[BT_ORDER-1]; /* child[i]<key[i]<=child[i+1] */
  struct sBTPage*	child[BT_ORDER]; /* Child (or object) pointers */
};

typedef struct sBTPage BTPage;

typedef struct {
  int       depth;  /* depth of tree (incl root) */
  BTPage    *root;  /* root page of B-tree */
  PagePool  *pool;  /* pointer to B-tree's page pool */
} BTree_S;
typedef BTree_S BTree;
typedef BTree_S * GBTree;

/* b+tree operations */
void BTPrint(GBTree const);
/* Print a B-tree */

int BTSearch(GBTree const, BTKey const key, GBTObject *obj);
/* Search "key" in "GBTree", return object through "obj".
   Returns BT_FOUND      if successfull,
           BT_INVALID    if invalid "GBTree",
           BT_NOT_FOUND  if key not in B-tree.
 */

int BTModify(GBTree const, BTKey const key, GBTObject **obj);
/* Used for replacing an object in the B+ tree.
   Search "key" in "GBTree", return object handle through "obj", which
   can then be used to replace the pointer to the object in the index
   strucutre.
   WARNING: the new object MUST have the same key values!
   Returns BT_FOUND      if successfull,
           BT_INVALID    if invalid "GBTree",
           BT_NOT_FOUND  if key not in B-tree.
 */

int BTIns(GBTree const, GBTObject const obj, GBTObject *ngb);
/* Insert "obj" in "GBTree".
   Returns, through "ngb", one of the new object's neighbours in the tree.
   Whether the right or left neighbour is returned is undefined.
   NULL is returned iff the new object is the first one in the tree.
   Returning the neighbour allows the user to maintain a secondary access
   structure to their objects.
   Returns BT_FOUND       if successful,
           BT_INVALID     if invalid "GBTree",
           BT_ALLOC_fail  if out of memory,
	   BT_DUPLICATE   if "key" already exists in B-tree,
           BT_OVERLAP     if new object overlaps existing one.
 */

int BTDel(GBTree const, BTKey const key, GBTObject *obj);
/* Delete object with "key" in "GBTree".
   A pointer to the actual object is returned so that the caller can
   then deallocate its storage.
   Only an object with an excatly matching key will be deleted
   (i.e. "key IN obj" is not good enough).
   Returns BT_FOUND       if successfull,
           BT_INVALID     if invalid "GBTree",
           BT_NOT_FOUND   if key does not match ank key in B-tree.
 */

/* possible return values */
#define BT_FOUND 0
#define BT_OK 0

#define BT_INVALID 1
#define BT_NOT_FOUND 2
#define BT_DUPLICATE 3
#define BT_OVERLAP 4
#define BT_ALLOC_fail 5 /* running out of memory */

/* Internal return codes: */
#define BT_PROMOTION 6 /* place the promoted key */
#define BT_LESS_THAN_MIN 7 /* redistribute key from child */

#endif /* BTREE_H */
