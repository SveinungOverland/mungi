/****************************************************************************
 *
 *      $Id: objects.c,v 1.20.2.1 2002/08/29 04:32:01 cgray Exp $
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

/* object management */

#include "mungi/l4_generic.h"
#include "mungi/kernel.h"
#include "mungi/security.h"
#include "mungi/mm.h"
#include "mungi/btree.h"
#include "mungi/objects.h"
#include "mungi/pdxcache.h"
#include "objects.h"


/* btree that holds object table */
static GBTree objtable;
static void objtable_populate(void);


/*
 * Find an object corresponding to the address given
 */
object_t *
object_find(const void * addr)
{
	object_t * obj;

	if (BTSearch(objtable, (uintptr_t)addr, &obj) == BT_FOUND)
		return obj;

	return NULL;
}

cap_t
object_pdx_get_clist(object_t * obj, pdx_t proc, passwd_t passwd)
{
	cap_t clist = NULL_CAP;
	register int i, j;
	objinfo_t * info;

	_lock_lock(&obj->lock);

	/* find PDX entry matching password and entry point */
	info = &obj->info;
	for (i = 0; i < O_MAX_PDX; i++) {
		if (info->pdx.passwd[i] == passwd) {
			int limit;

			limit = info->pdx.x_entry[i];
			for (j = info->pdx.n_entry[i]; j < limit; j++)
				if (proc == info->pdx.entry[j]) {
					/* entry point found! */
					clist.address = 
						info->pdx.clist[i].address;
					clist.passwd = passwd;
					break;
				}
		}
	}

	_lock_unlock(&obj->lock);

	return clist;
}

object_t *
object_add(const void *addr, size_t length, passwd_t passwd)
{
	object_t *obj, *nbd;

	/* allocate object entry */
	obj = (object_t *)kmalloc(sizeof(object_t));
	if (obj == NULL)
		return NULL;

	/* initialise object data */
	obj->base = (void *)addr;
	obj->end = (void *)((uintptr_t)addr + (uintptr_t)length);
	obj->semp = NULL;
	obj->info.passwd[0] = passwd;
	obj->info.rights[0] = M_OWNER;
	obj->info.n_caps = 1;
	obj->info.length = length;
        _lock_init(&obj->lock);

	/* insert object into object table */
	if (BTIns(objtable, obj, &nbd) != BT_FOUND) {
		kfree(obj);
		return NULL;
	}

	return obj;
}

object_t *
object_create(size_t length, objinfo_t *info, passwd_t passwd)
{
	object_t *obj;
	void *base;

	/* allocate objects backing */
	base = kmalloc_obj(length);
        if (base == NULL){
                kprintf("Failed to kmalloc\n");
		return NULL;
        }

	/* add object to object table */
	obj = object_add(base, length, passwd);
        if (obj == NULL){
                kprintf("Failed to object add\n");
		kfree_obj(base, length);
        }

	return obj;
}

int
object_delete(void * obj_addr)
{
	object_t * obj;

	/* remove object from object table */
	if (BTDel(objtable, (uintptr_t)obj_addr, &obj) != BT_FOUND)
		return fail;

	/* free object store */
	/* FIXME allocation lazy delete list */
	kfree_obj(obj->base, (size_t)((uintptr_t)obj->end-(uintptr_t)obj->base));
	kfree(obj);
	
	return success;
}

/*
 * change object passwords (not PDX passwords)
 */
int
object_passwd(object_t * obj, passwd_t passwd, access_t rights)
{
	int i, empty = -1;

	rights &= (~M_PDX);
	for (i = 0; i < O_MAX_CAPS; i++) {
		if ((empty == -1) && (obj->info.rights[i] == 0)) {
			empty = i;
		} else if (obj->info.passwd[i] == passwd) {
			/* password only exists once */
			obj->info.rights[i] = rights;
			return success;
		}
	}

	if (empty == -1)
		return fail;

	/* n_caps tells the validator how far it has to check, not 
	  actually how many exist. update it */
	if (empty >= obj->info.n_caps)
		obj->info.n_caps = empty + 1;

	obj->info.passwd[empty] = passwd;
	obj->info.rights[empty] = rights;

	return success;
}

int
object_resize(object_t *obj, size_t new_size)
{
	size_t extra, cur_size;
	uintptr_t orig_page, new_page;
	void *obj_end;
	int ret = fail;

	_lock_lock(&obj->lock);

	/* Make sure the new size is page aligned! */
	new_size = (uintptr_t) page_round_up( new_size );

	obj_end = (void *)((size_t)obj->base + new_size);
	cur_size = obj->info.length;
	orig_page = (uintptr_t)page_round_up(obj->end);
	new_page = (uintptr_t)page_round_up(obj_end);

	if (new_page != orig_page) {
                if (new_size == obj->info.length){ /* same */
                        /* Do nothing... */
                } else if (new_size < obj->info.length) { /* smaller */
			extra = cur_size - new_size;
			if (!kfree_obj(obj_end, extra))
				goto object_resize_out;
		} else { /* larger */
			extra = new_size - cur_size;
			if (kmalloc_obj_addr(obj->end, extra) == NULL) {
				/* bad luck if it could not grow */
			        goto object_resize_out;
			}
		}
	}

	obj->end = obj_end;
	obj->info.length = new_size;
	ret = success;

object_resize_out:
	_lock_unlock(&obj->lock);

	return ret;
}

int
object_info(object_t * obj, objinfo_t * info, int flags)
{
	/* get all public values - just doing the dumb way to avoid any 
	   compiler problems */
	info->extent = obj->info.extent;
	info->creation = obj->info.creation;
	info->modification = obj->info.modification;
	info->access = obj->info.access;
	info->accounting = obj->info.accounting;
	info->userinfo = obj->info.userinfo;
	info->acctinfo = obj->info.acctinfo;
	info->length = obj->info.length;

#ifdef TESTING

	/* in testing mode get the PDX data out, too */
	info->n_pdx = obj->info.n_pdx;
	memcpy( &info->pdx, &obj->info.pdx, sizeof( pdxdata_t ) );

#endif

	return 0;
}


static void
kill_pdx_slot( int slot, objinfo_t *info )
{
	int diff, i;
	
	/* sanity */
	assert( slot >= 0 && slot < O_MAX_PDX );

	/* get the number of entry points to remove */
	diff = info->pdx.x_entry[slot] - info->pdx.n_entry[slot];

	/* start moving entry points down */
	for( i = info->pdx.n_entry[slot]; i < (O_MAX_ENTPT - diff); i++ )
		info->pdx.entry[i] = info->pdx.entry[i + diff];

	/* move all the arrays back down */
	for( i = slot; i < info->n_pdx; i++ )
	{
		/* correct the indices on the n_ and x_ records */
		info->pdx.n_entry[i] = info->pdx.n_entry[i+1] - diff;
		info->pdx.x_entry[i] = info->pdx.x_entry[i+1] - diff;
		info->pdx.clist[i] = info->pdx.clist[i+1];
		info->pdx.passwd[i] = info->pdx.passwd[i+1];
	}
	
	/* decrement the PDX counter */
	info->n_pdx--;
	

	/* since we've deleted we should flush the pdx cache */
	pdxcache_flush();

}

static int
set_pdx_slot( int slot, int n_entries, pdx_t * entrypts, 
	      passwd_t passwd, cap_t clist, object_t * obj )
{
	int j, next, end;
	objinfo_t *info;

	/* sanity */
	assert( slot >= 0 && slot < O_MAX_PDX );

	info = &obj->info;

	/* find next free entry point entry */
	if( slot == 0 )
		next = 0;
	else
		next = info->pdx.x_entry[slot-1];  /* x_entry points to the
						    * *next* one */

	end = next + n_entries;
	if (end >= O_MAX_ENTPT)
		return fail;
		
	/* copy entry points */
	for (j = 0; j < n_entries; j++) {
		if (((char *)entrypts[j] < (char *)obj->base) ||
		    ((char *)entrypts[j] > (char *)obj->end)) {
			return fail;
		} else {
			info->pdx.entry[next + j] = entrypts[j];
		}
	}
		
	info->pdx.n_entry[slot] = next;
	info->pdx.x_entry[slot] = end;
	info->pdx.clist[slot] = clist;
	info->pdx.passwd[slot] = passwd;
	info->n_pdx++;

	return success;
}

int
object_pdx(object_t * obj, passwd_t passwd, cap_t clist, uint n_entrypt, 
	   pdx_t * entrypts)
{
	int i, slot;
	int retval = fail;
	objinfo_t *info;

	_lock_lock(&obj->lock);

	info = &obj->info;

	/* first find if we are deleting one (matching passwd) */
	for (i = 0; i < info->n_pdx; i++) {
		if( info->pdx.passwd[i] == passwd )
		{
			kill_pdx_slot( i, info );
			break;
		}
	}
		
	/* find a free PDX entry or one with the same passwd */
	slot = info->n_pdx;

	/* none empty and none overwritten */
	if( slot >= O_MAX_PDX )
		goto out;

	/* just return if they specified 0 entries (side effect is delete) */
	if( n_entrypt == 0 )
	{
		retval = success;
		goto out;
	}

	/* fill in the info */
	retval = set_pdx_slot( slot, n_entrypt, entrypts, passwd, clist, obj );
out:
	_lock_unlock(&obj->lock);

	return retval;
}

void
objtable_init(void *saved_objtable)
{
	if (saved_objtable != NULL){
		VERBOSE("Using restored Objtable\n");
		objtable = saved_objtable;
	} else {
		/* alloc btree information */
		objtable = (GBTree)kmalloc(sizeof(BTree));
		assert(objtable);
		objtable->root = NULL;
		
		/* allocate free page pool structure */
		objtable->pool = (PagePool *)kmalloc(sizeof(PagePool));
		assert(objtable->pool);
		objtable->pool->n_allocs = objtable->pool->n_frees = 0;
		
		/* allocate memory for free page pool */
		objtable->pool->size = MAX_POOL_SIZE;
		objtable->pool->base = (char *)kmalloc((objtable->pool->size)*
						       L4_PAGESIZE);
		assert(objtable->pool->base);
		
		/* make page aligned */
		objtable->pool->base = (char *)page_round_up(objtable->pool->base);
		
		/* clear free page pool bitmap */
		memset(objtable->pool->bitmap,0,MAX_POOL_SIZE/(8*sizeof(int)));
		
		objtable_populate();
	}
}

void
save_objtable(void **saved_objtable){
        *saved_objtable = (void *)saved_objtable;
}

static void
objtable_populate(void)
{
	object_t *obj;
	
	/*
	 * Add all of the predefined objects to the allocated list
	 */
	
	/*ret = kmalloc_obj_addr(l4_setup.mungi.base, l4_setup.mungi.size);
	assert(ret != NULL); 
	ret = kmalloc_obj_addr(l4_setup.upcall.base, l4_setup.upcall.size);
	assert(ret != NULL);
	*ret = kmalloc_obj_addr(l4_setup.object_tbl.base, 
				l4_setup.object_tbl.size);
	assert(ret != NULL);
	*/
	obj = object_add(l4_setup.upcall.base, l4_setup.upcall.size, 0);
	assert(obj != NULL);
	object_passwd(obj, 0, M_READ|M_EXECUTE);
        /*
	obj = object_add(l4_setup.mungi.base, l4_setup.mungi.size, 0);
	assert(obj != NULL);
	object_passwd(obj, 0, 0);
	obj = object_add(l4_setup.object_tbl.base, l4_setup.object_tbl.size, 0);
	assert(obj != NULL);
	object_passwd(obj, 0, 0);  * enable this once persistent objects */
	
}
