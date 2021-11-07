/****************************************************************************
 *
 *      $Id: btree.c,v 1.3 2002/05/31 06:14:10 danielp Exp $
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

/* btree.c: a b+tree index implementation, written 28/10/95 
   modified: 30/10/95 (adding all the const)
              1/11/95 BTModify
	24/4/96 by Jerry to make it fit in the Mungi kernel stuff
	08/02/97 by Gernot adapted for OO1 use
	22/02/97 by Gernot cleaned up:
		  - total separation of object and index structure
		  - interval support in index structure
		  - user control over memory allocation.
*/
/* count is the number of keys (= number of children - 1)
 * All key values in child[j] are >= key[i], for j>i
 * all key values in child[j] are <  key[i], for j<=i.
 */
#include <l4/types.h>
#include "mungi/kernel.h"
#include "mungi/klibc.h"
#include "mungi/btree.h"



/* static declarations */
static int BTDelete(PagePool *pool, BTPage * const current, BTKey const key,
		    GBTObject *obj);
static int BTInsertPage(BTPage * const current, BTKeyCount const hi,
			BTKey const kpromoted, BTPage * const ppromoted);
static int BTInsert(PagePool *pool, BTPage * const current, GBTObject const obj,
		    BTPage ** const ppromoted, BTKey * const kpromoted,
		    GBTObject *ngb);
static int BTDeletePage(BTPage * const current, BTKeyCount idx);
static BTKey BTRedistribute(BTPage* const left, BTPage* const right);
static void BTCollapse(PagePool *pool, BTPage *const left,
		       BTPage * const right);


BTPage *
alloc_page(PagePool *pool)
{
	unsigned int w, b, mask;

	for (w=0; w*sizeof(int)*8<pool->size && pool->bitmap[w] == ~0; w++)
		;
	if (w*sizeof(int)*8 >= pool->size) {
		return NULL; /* pool out of memory */
	}

	for (b = 0, mask = 1; (mask&(pool->bitmap[w])) != 0; mask <<= 1, b++)
		;
	pool->bitmap[w] |= mask;
	pool->n_allocs++;

	return (BTPage *)((uintptr_t)pool->base + 
		(uintptr_t)L4_PAGESIZE*(w*8*sizeof(int) + b));
}


void
free_page(PagePool *pool, BTPage *p)
{
	unsigned long w, b;
	unsigned int  mask;

	b = ((char*)p - pool->base)/L4_PAGESIZE;
	w = b / (8*sizeof(int));
	mask = 1 << (b % 8*sizeof(int));

	if (!(pool->bitmap[w] & mask))
		panic("free_page: Tried to deallocate page %p twice!\n", p);

	pool->bitmap[w] &= ~mask;
	pool->n_frees++;
}


#ifdef DEBUGP
void
BTPrintObj(const GBTObject blah)
{
  uint16_t i;

  kprintf("Base = %p, Length = %p\n",blah->base,blah->length);
  if (blah->otherinfo.n_caps)
    kprintf("Caps\r\n");
  for(i=0; i< blah->otherinfo.n_caps;i++)
    kprintf("\tPasswd  %llx\r\n",blah->otherinfo.caplist[i].passwd);
}


/* print B-tree index in inorder way started from root */
void BTPrintInt(BTPage const * const root) 
{
  BTKeyCount i;

  kprintf("Page address[%lx]   \r\n", root);

  for(i = 0; i < root->count; i++) 
    kprintf("%lx %lx ", root->child[i], (unsigned long)root->key[i]);
  kprintf("%lx \n\r", root->child[i]);
  /* print all the children */
  for(i = 0; i <= root->count; i++) {
      if (root->isleaf) {
	  BTPrintObj((GBTObject)(root->child[i]));
      } else {
	  BTPrintInt(root->child[i]);
      }
  }
  kprintf("\n\r");
}

void BTPrint(GBTree const btree)
/* print b+tree (index and leaves)*/
{

  if (btree && btree->root) {
    kprintf("\n\r\nB-tree %p\n\n", btree);
    BTPrintInt(btree->root); kprintf("\n\r");
  } else {
    kprintf("B-tree is empty\n\r");
  }
}
#endif


int
BTModify(GBTree const btree, BTKey const key, GBTObject **obj)
{
	BTKeyCount lo, hi, mid;
	BTPage *current;

	if (!btree) return BT_INVALID;
	if (!(current=btree->root)) return BT_NOT_FOUND;

	for(;;) {
		/* use binary search to look thru the page */
		lo = 0; hi = current->count;
		while(lo < hi) {
			mid = (lo + hi) / 2;
			if (BTKeyEQ(current->key[mid], key)) {
				hi = mid+1; break;
			} else if (BTKeyGT(current->key[mid], key))
				hi = mid;
			else
				lo = mid+1;
		}
		if (current->isleaf) {
			break;
		}
		current = current->child[hi];
	}

	*obj = (GBTObject*)&(current->child[hi]);
	if (BTObjMatch(**obj, key)) {
		return BT_OK;
	} else {
		return BT_NOT_FOUND;
	}
}



int
BTSearch(GBTree const btree, BTKey const key, GBTObject *obj)
{
	BTKeyCount lo, hi, mid;
	BTPage *current;

	if (!btree) return BT_INVALID;
	if (!(current=btree->root)) return BT_NOT_FOUND;

	for (;;) { /* use binary search to look thru the page */
		lo = 0; hi = current->count;
		while(lo < hi) {
			mid = (lo + hi) / 2;
			if (BTKeyEQ(current->key[mid], key)) {
				hi = mid+1; break;
			} else if (BTKeyGT(current->key[mid], key))
				hi = mid;
			else
				lo = mid+1;
		}
		if (current->isleaf) {
			break;
		}
		current = current->child[hi];
	}

	*obj = (GBTObject)(current->child[hi]);
	if (BTObjMatch(*obj, key)) {
		return BT_OK;
	} else {
		return BT_NOT_FOUND;
	}
}


int
BTIns(GBTree const btree, GBTObject const obj, GBTObject *ngb)
{
	BTPage *ppromoted, *newroot;
	BTKey kpromoted;
	int retval;
	int i;

	if (btree == NULL) { /* no tree */
		return BT_INVALID;
	}

	if (btree->root == NULL) { /* tree empty */
		newroot = (BTPage *)alloc_page(btree->pool);
		if (newroot == NULL) return BT_ALLOC_fail;

		for (i = 0; i < BT_ORDER; i++) {
			newroot->child[i] = NULL;
			newroot->key[i] = 0;
		}
		newroot->child[i] = NULL;

		newroot->count = 0;
		newroot->isleaf = true;
		newroot->child[0] = (BTPage *)obj;
		btree->root       = newroot;
		btree->depth      = 1;
		*ngb = NULL;

		return BT_OK;
	}

	if ((retval = BTInsert(btree->pool, btree->root, obj,
				&ppromoted, &kpromoted, ngb)) == BT_PROMOTION) {

		newroot = (BTPage *)alloc_page(btree->pool);
		if (newroot == NULL) return BT_ALLOC_fail;

		for (i = 0; i < BT_ORDER; i++) {
			newroot->child[i] = NULL;
			newroot->key[i] = 0;
		}
		newroot->child[i] = NULL;

		newroot->count = 1; newroot->isleaf = false;
		newroot->child[0] = btree->root;
		newroot->child[1] = ppromoted;
		newroot->key[0] = kpromoted;
		btree->root = newroot;
		btree->depth++;
		retval = BT_OK;
	}

	return retval;
}



int
BTDel(GBTree const btree, BTKey const key, GBTObject *obj)
{
	BTPage *current;
	int retval;

	if (btree == NULL) { /* no tree */
		return BT_INVALID;
	}

	if ((current=btree->root) == NULL) { /* tree empty */
		return BT_NOT_FOUND;
	}

	if ((retval = BTDelete(btree->pool, current, key, obj)) == 
			BT_LESS_THAN_MIN) {
		if (current->isleaf) { /* only page */
			if (current->count < 0) { /* B-tree now empty */
				free_page(btree->pool, current);
				btree->root  = NULL;
				btree->depth = 0;
			}
		} else {
			if (current->count==0) {
				btree->root = current->child[0];
				btree->depth--;
				free_page(btree->pool, current);
			} 
		}
		retval = BT_OK;
	}
	return retval;
}


static void
GetLeastKey(BTPage * current, BTKey * const key)
{
  /* there must be a better way than this... */
  while (!current->isleaf) {
    current = current->child[0];
  }
  *key = BTGetObjKey((GBTObject)(current->child[0]));
}

static int BTDelete(PagePool *pool, BTPage * const current, BTKey const key,
		    GBTObject *obj)
/* delete key from b-tree with root pointed by current, returns:
  - BT_NOT_FOUND if there's no object with key
  - BT_LESS_THAN_MIN if deletion result in current->count < BT_MINKEY
       [the caller should take care of collapsing or redistributing keys]
  - BT_OK if key can be deleted successfully
*/
{
  BTKeyCount lo, hi, mid;
  BTKeyCount med, sibling;
  int retval;
  
  if (!current) return BT_NOT_FOUND;

  /* use binary search to look thru the page */
  lo = 0; hi = current->count;
  while(lo < hi) {
    mid = (lo + hi) / 2;
    if (BTKeyEQ(current->key[mid], key)) {
      hi = mid+1; break;
    } else if (BTKeyGT(current->key[mid], key))
      hi = mid;
    else
      lo = mid+1;
  }

  if (current->isleaf) {
    if (!BTKeyEQ(BTGetObjKey((GBTObject)(current->child[hi])),key)) {
      return BT_NOT_FOUND;
    }
    *obj = (GBTObject)(current->child[hi]);
    retval = BTDeletePage(current, hi-1);
  } else {
    retval = BTDelete(pool, current->child[hi], key, obj);
    if (hi && (retval == BT_OK  || retval == BT_LESS_THAN_MIN) &&
	BTKeyEQ(current->key[hi-1],key)) {
	GetLeastKey(current->child[hi], &(current->key[hi-1]));
    }
    if (retval == BT_LESS_THAN_MIN) {
      /* try to redistribute first, identify fattest sibling */
      sibling = (!hi) ? 1 : 
	((hi==current->count) ? hi-1 : 
	 ((current->child[hi-1]->count > current->child[hi+1]->count) ? hi-1 : hi+1));
      med = (hi + sibling) / 2;
      if (current->child[sibling]->count > BT_MINKEY) {
	current->key[med] = BTRedistribute(current->child[min(hi, sibling)], 
					   current->child[max(hi, sibling)]);
	retval = BT_OK;
      } else {
	/* collapse the two pages, get the median value down */
	BTCollapse(pool, current->child[min(hi, sibling)],
		   current->child[max(hi, sibling)]);
	retval = BTDeletePage(current, med);
      }
    }
  } /* if (current->isleaf) */
  return retval;
}

static int BTInsertPage(BTPage * const current, BTKeyCount const hi,
			BTKey const kpromoted, BTPage * const ppromoted) 
/* insert kpromoted with child ppromoted at index hi in page current,
   ppromoted is kpromoted's right child */
{
  BTKeyCount i;

  for(i = current->count-1; i >= max(hi,0); i--) {
    current->key[i+1] = current->key[i]; 
    current->child[i+2] = current->child[i+1];
  }
  if (hi<0) {
    current->key[0]   = BTGetObjKey((GBTObject)(current->child[0]));
    current->child[1] = current->child[0];
    current->child[0] = ppromoted;
  } else {
    current->key[hi] = kpromoted; current->child[hi+1] = ppromoted;
  }
  current->count++;
  return BT_OK;
} /* BTInsertage */

static int BTInsert(PagePool *pool, BTPage * const current, GBTObject const obj,
		    BTPage ** const ppromoted, BTKey * const kpromoted,
		    GBTObject *ngb)
/* insert obj in the b+tree with b-tree index pointed by current,
   return:
     BT_PROMOTION : if current splitted to two pages, 
        ppromoted will point to the right child of key kpromoted
        (while current will hold the left child)
     BT_OK : if insert succesfull
     BT_DUPLICATE : if there has been another object with the same key in tree 
     BT_ALLOC_fail : if there's not enough memory to allocate a new page
        (the upper level index can be inconsistent with the lower level)
*/
{
  BTKeyCount lo, hi, mid;
  BTKeyCount i, j;
  BTPage*    newpage;
  int        retval;
  BTKey      key, kmed; /* median key */
  int        in_old;

  if (current == NULL) {
    *kpromoted = BTGetObjKey(obj);
    *ppromoted = NULL; /* this is the _initialization_ for all ptr */
    return BT_PROMOTION;
  }
  
  key = BTGetObjKey(obj);
  /* use binary search to look thru the page */
  lo = 0; hi = current->count;
  while(lo < hi) {
    mid = (lo + hi) / 2;
    if (BTKeyEQ(current->key[mid], key)) {
      return BT_DUPLICATE;
    } else if (BTKeyGT(current->key[mid], key))
      hi = mid;
    else
      lo = mid+1;
  }

#ifdef BT_HAVE_INTERVALS
  if (hi<current->count && BTObjMatch(obj,current->key[hi])) {
    return BT_OVERLAP;
  }
#endif /* BT_HAVE_INTERVALS */

  if (current->isleaf) {
#ifdef BT_HAVE_INTERVALS
    if (BTObjMatch((GBTObject)(current->child[hi]), key) ||
	(!hi && BTOverlaps(obj,(GBTObject)(current->child[0])))) {
      return BT_OVERLAP;
    }
#endif /* BT_HAVE_INTERVALS */
    *ppromoted = (BTPage *) obj;
    *kpromoted = BTGetObjKey(obj);
    retval = BT_PROMOTION;
    *ngb = (GBTObject)(current->child[hi]);
    if (!hi && BTKeyLT(key,BTGetObjKey((GBTObject)(current->child[0])))) {
	hi=-1;
    }
  } else {
    retval = BTInsert(pool, current->child[hi], obj, ppromoted, kpromoted, ngb);
  }
  if (retval==BT_PROMOTION) {
    if (current->count < BT_MAXKEY) {
      return BTInsertPage(current, hi, *kpromoted, *ppromoted);
    } 
    /* split the page BT_MINKEY keys on left (org) and BT_MAXKEY-BT_MINKEY on right */
    newpage = (BTPage *) alloc_page(pool);
    if (newpage == NULL) return BT_ALLOC_fail;
    newpage->isleaf = current->isleaf;
    newpage->count = BT_MAXKEY-BT_MINKEY;
    current->count = BT_MINKEY;
    if ((in_old = (hi < current->count))) {
      /* insert promoted key in the left (original) page */
      current->count--;
      kmed = current->key[current->count];
      current->key[current->count] = 0;
    } else { /* insert promoted key in the right (new) page */
      newpage->count--;
      if (hi == current->count) {
	kmed = *kpromoted;
      } else {
	kmed = current->key[current->count];
	current->key[current->count] = 0;
      }
    }
    newpage->child[0] = current->child[current->count+1];
    current->child[current->count+1] = NULL;
    for (j=current->count+1, i=0; i<newpage->count; i++, j++) {
      newpage->key[i]     = current->key[j];
      newpage->child[i+1] = current->child[j+1];
      current->key[j]     = 0;
      current->child[j+1] = NULL;
    }
    for (; i<BT_MAXKEY; i++) {
      newpage->key[i]     = 0;
      newpage->child[i+1] = NULL;
    }

    if (in_old) {
      retval = BTInsertPage(current, hi, *kpromoted, *ppromoted);
    } else {
      hi -= current->count+1;
      retval = BTInsertPage(newpage, hi, *kpromoted, *ppromoted);
    }
    *kpromoted = kmed;
    *ppromoted = newpage;
    return BT_PROMOTION;
  }
  return retval; /* can't just return ok, sth might go wrong */
} /* BTInsert */


static int BTDeletePage(BTPage* const current, BTKeyCount idx) 
/* Delete key with index idx in page current, overwrites the right child ptr
   current != NULL, idx < current->count, possible return values:
      BT_OK : if the number of keys remaining within the page is >= BT_MINKEY
      BT_LESS_THAN_MIN : if the page after deletion holds < BT_MINKEY keys
*/
{
  BTKeyCount i;

  if (idx<0) {
    current->child[0] = current->child[1];
    idx++;
  }
  for(i = idx+1; i < current->count; i++){
    current->key[i-1] = current->key[i];
    current->child[i] = current->child[i+1];
  }
  current->child[current->count] = NULL;
  current->count--;
  if (current->count>=0) {
    current->key[current->count] = 0;
  }
  return (current->count < BT_MINKEY) ? BT_LESS_THAN_MIN : BT_OK;
}

static void BTCollapse(PagePool *pool, BTPage * const left,
		       BTPage * const right) 
/* make left and right one page, copy to left, kmed is the median from parent */
{
  BTKeyCount i, j;
  BTKey kmed;

  GetLeastKey(right, &kmed);
  left->key[left->count] = kmed;
  left->child[left->count+1] = right->child[0];
  for(i=0, j = left->count+1; i < right->count; i++, j++) {
    left->key[j] = right->key[i];
    left->child[j+1] = right->child[i+1];
  }
  left->count = left->count + right->count + 1;
  free_page(pool, right);
} /* BTCollapse */

static BTKey BTRedistribute(BTPage* const left, BTPage* const right)
/* redistribute from left to right page [kmed is the median value b/w left and right], 
   returns the new median */
{
  BTKeyCount half, delta, i, j;
  BTKey kmedian, kmed;


  half = (left->count + right->count) >> 1;
  GetLeastKey(right,&kmed);
  if (left->count < half) {
    /* get (half - left->count)-1 keys from right to left */
    delta = (half - left->count); /* distance travelled */
    left->key[left->count] = kmed;
    for(i = left->count+1, j=0; i < half; i++, j++) {
      left->key[i] = right->key[j];
      left->child[i] = right->child[j];
    }
    left->child[half] = right->child[j];
    kmedian = right->key[j];
    right->child[0]=right->child[j+1];
    i = 0;
    while(j++ < right->count) {
      right->key[i]     = right->key[j];
      right->child[i+1] = right->child[j+1];
      right->key[j]     = 0;
      right->child[j+1] = NULL;
      i++;
    }
    right->count = right->count - delta;    
    left->count = half;
  } else {
    /* move (left->count - half -1) keys from left to right */
    delta = left->count - half; /* distance travelled */
    /* make space */
    right->child[right->count+delta] = right->child[right->count];
    for(j=right->count+delta-1, i = right->count-1; i >= 0; j--, i--) {
      right->key[j] = right->key[i];
      right->child[j] = right->child[i];
    }
    right->key[j] = kmed;
    right->child[j] = left->child[left->count];
    i = left->count-1; 
    j--;
    while(j >= 0) {
      right->key[j]   = left->key[i];
      right->child[j] = left->child[i];
      left->key[i]    = 0;
      left->child[i]  = NULL;
      i--; j--;
    }
    kmedian         = left->key[half];
    left->key[half] = 0;
    left->count     = half;
    right->count    = right->count + delta;
  }
  return kmedian;
} /* BTRedistribute */
