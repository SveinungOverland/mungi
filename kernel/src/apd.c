/****************************************************************************
 *
 *      $Id: apd.c,v 1.20.2.1 2002/08/29 04:32:00 cgray Exp $
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

/* PD/APD management */

#include "mungi/kernel.h"
#include "mungi/lock.h"
#include "mungi/apd.h"
#include "mungi/security.h"
#include "mungi/clist.h"
#include "mungi/mm.h"
#include "mungi/threads.h"
#include "mungi/pdxcache.h"
#include "status.h"
#include "mungi/clock.h"

clist_t system_clist = { 'c', 1, CL_UNSRT_0, 1, 0, { { 0, 0} } };
static cap_t upcall_cap;


/* 
 * Hash table of joined PD's (ie APD's that can be joined) 
 */
#define APD_HASH_SIZE	512 /* FIXME */
/* FIXME real hash function needed */
#define PD_HASH(a)	(((uintptr_t)(a) >> L4_LOG2_PAGESIZE)&(APD_HASH_SIZE-1))

static struct {
	lock_t	lock;
	apd_t	**table;
} apd_table;

/*
 * Hash table of APD's cached for PDX calls
 */
#define APD_PDX_CACHE_SIZE	20
#ifdef ALPHAENV
#define APD_PDX_CACHE_TIMEOUT	((time_t)40000) /* L4/Alpha is broken */
#else
#define APD_PDX_CACHE_TIMEOUT	((time_t)60) /* 1 minute */
#endif

struct apd_pdx_cache_entry {
	apd_t		*orig;
	cap_t		clist;
	apd_t		*apd;
	time_t		timestamp;
};
static struct {
	lock_t				lock;
	struct apd_pdx_cache_entry	*table;
} apd_pdx_cache;

/* monotonically increasing ID for APDs */
static uintptr_t apd_serial = 0;

static bool apd_fill(apd_t *);
static bool apd_clone(apd_t *, apd_t *);
static void apd_pdx_cache_insert(apd_t *, cap_t, apd_t *, 
				 struct apd_cache_id *id);
inline static apd_t * apd_pdx_cache_lookup(apd_t *, cap_t,
					   struct apd_cache_id *id);
static void apd_pdx_cache_remove(apd_t *apd, bool);


void
apd_init(void)
{
        int i;
        object_t *upcall_obj;
        
	/* initialise the joined PD's table */
	_lock_init(&apd_table.lock);
	apd_table.table = (apd_t **)kmalloc(APD_HASH_SIZE*sizeof(apd_t *));
	assert(apd_table.table != NULL);

	/* initialise the PDX APD cache */
	_lock_init(&apd_pdx_cache.lock);
	apd_pdx_cache.table = kmalloc(APD_PDX_CACHE_SIZE
					* sizeof(struct apd_pdx_cache_entry));
	assert(apd_pdx_cache.table != NULL);

	for (i = 0; i < APD_PDX_CACHE_SIZE; i++)
		apd_pdx_cache.table[i].apd = NULL;

	/* setup the cap for upcall which is added to all APDs */
	upcall_obj = object_find(l4_setup.upcall.base);
	assert(upcall_obj != NULL);
	upcall_cap.address = upcall_obj->base;
	upcall_cap.passwd = mrandom();
	/* FIXME: should only be read and exec, but need write for globals */
	object_passwd(upcall_obj, upcall_cap.passwd, M_READ|M_WRITE|M_EXECUTE);
 
	/* initialise the PDX cache */
        pdxcache_init();
}

apd_t *
apd_find_or_create(const apddesc_t *pd)
{
	int hashval;
	apd_t *apd;

	hashval = PD_HASH(pd);
	_lock_lock(&apd_table.lock);

	/* search for already instantiated PD */
        for (apd = apd_table.table[hashval]; apd != NULL; apd = apd->next)
                if (apd->pd == pd)
			break;

	if (apd == NULL) {
		/* create new APD */
		apd = apd_from_template(pd);
		if (apd) {
			apd->pd = pd;

			/* add APD to joined PD's TABLE */
			apd->next = apd_table.table[hashval];
			if (apd->next)
				apd->next->pprev = &apd->next;
			apd_table.table[hashval] = apd;
			apd->pprev = &apd_table.table[hashval];
		}
	}
	_lock_unlock(&apd_table.lock);

	return apd;
}

apd_t *
apd_from_template(const apddesc_t *pd)
{
	apd_t *new;
	int i;

	/* create the kernel APD */
	new = (apd_t *)kmalloc(sizeof(*new));
	if (new == NULL)
		goto out;

	/* clean out the new one - ceg */
	bzero( new, sizeof( apd_t ) );
        
	/* copy data from PD */
	memcpy(&new->data, pd, sizeof(new->data));

	/* sanity check PD data */
	if (new->data.n_apd > APD_MAX_ENTRY)
		goto out;

	/* validate all caps for the clists greater than slot 0 */
	for (i = 1; i < new->data.n_apd; i++)
		if (!validate_cap(new->data.clist[i], M_READ))
			goto out;

	/* fill 'er up */
	if (apd_fill(new))
		goto out;

	return new;

out:
	if (new != NULL)
		kfree(new);
	return NULL;
}


static bool
apd_fill(apd_t *new)
{
        object_t *clist_obj;
        passwd_t clist_pwd;
        cap_t cap;

	/* assign a unique ID */
	new->serial = apd_serial++;

	/* NB: if this assert has triggered then your Mungi box has been
	   up for a *long* time */
	/* FIXME: this will also need a node ID when distributed */
	/* FIXME: if this ever triggers, could we just flush all APDs? */
	assert( apd_serial != 0 );

	/* create validation cache */
	if (VCACHE(new) == NULL) {
		VCACHE(new) = vcache_new();
		if (VCACHE(new) == NULL)
			goto out;
	}

	/* create the system clist */
	clist_pwd = mrandom();
	clist_obj = object_create(PAGESIZE, NULL, clist_pwd);
	if (clist_obj == NULL)
		goto out;

	if (new->data.n_apd == 0)
		new->data.n_apd = 1;

	/* original slot 0 is ignored, and replaced */
	new->data.clist[0].address = clist_obj->base;
	new->data.clist[0].passwd = clist_pwd;

	/* initialise system C-list */
	CLIST_INIT(clist_obj->base);

	cap.address = clist_obj->base;
	cap.passwd = clist_pwd;
	clist_autoadd(new, cap);

        /* give it a capability for upcall */
	clist_autoadd(new, upcall_cap);

        
	_lock_init(&(new->lock));

	return false;

out:
	if (VCACHE(new)) {
		vcache_free(VCACHE(new));
	}

	return true;
}

static bool
apd_clone(apd_t *new, apd_t *apd)
{
	_lock_lock(&apd->lock);
	memcpy(&new->data, &apd->data, sizeof(apd_t));
	_lock_unlock(&apd->lock);
	/* TODO should copy vcache */

	return false;
}

apd_t *
apd_pdx_find_or_create(const apddesc_t *pd, cap_t clist_cap, 
		       struct apd_cache_id *id)
{
	apd_t *apd = NULL;

	if (pd != NULL) { /* find APD associated with PD */
		apd = apd_find_or_create(pd);
		if (apd == NULL)
			return NULL;
	}

	apd = apd_pdx_merge(apd, clist_cap, id);

	return apd;
}

apd_t *
apd_pdx_merge(apd_t *apd, cap_t clist_cap,
	      struct apd_cache_id *id)
{
	apd_t *new;
	int i;

	/* look through cache */
        new = apd_pdx_cache_lookup(apd, clist_cap, id);
	if (new != NULL)
		return new;
        
	/* allocate space for new APD */
	new = (apd_t *)kmalloc(sizeof(apd_t));
	if (new == NULL)
		return NULL;

        /* AE - must set to zero, otherwise below where we merge in
         * clists won't work (nec.) */
	memset( new, 0, sizeof(apd_t) );

	if (apd != NULL) {
		if (apd_clone(new, apd))
			goto out;

                /* AE - surely apd_clone should not use the same l4 tasks? 
                 * Otherwise the originals can get access to the extra bits */
		new->tasks = NULL;
	}

	/* setup the apd -- create stack */
	if (apd_fill(new))
		goto out;

	/* try to merge in extra clist */
	for (i = new->data.n_locked; i < APD_MAX_ENTRY; i++) {
		if (new->data.clist[i].address == NULL) {
			new->data.clist[i] = clist_cap;
			new->data.n_apd++;

			/* add to pdx cache */
                        apd_pdx_cache_insert(apd, clist_cap, new, id);
			return new;
		}
	}
	
out:
	kfree(new);
	return NULL;
}

void
apd_get(apd_t * apd, apddesc_t * buffer)
{
	int i;
	apddesc_t * info;

	info = &apd->data;

	_lock_lock(&apd->lock);

	/*
	 * Copy the apd data. We cannot use memcpy since buffer is readable by
	 * the user, and we don't wish to copy passwords.
	 */

	buffer->n_locked = info->n_locked; 
	buffer->n_apd = info->n_apd; 

        
	for (i = 0; i < APD_MAX_ENTRY; i++) {
		buffer->clist[i].address = info->clist[i].address;
		buffer->clist[i].passwd = (passwd_t)0;
	}

	_lock_unlock(&apd->lock);
}

int
apd_slot_insert(apd_t *apd, const clist_t *clist, passwd_t passwd, apdpos_t pos)
{
	int retval = 0;

	/* check if valid clist and valid position */
	if (IS_NOT_CLIST(clist) || 
	    (pos < 0) || (pos >= APD_MAX_ENTRY))
		return 1;

	_lock_lock(&apd->lock);	

	if ((apd->data.n_apd == APD_MAX_ENTRY) || (pos < apd->data.n_locked)) { 
		if( apd->data.n_apd == APD_MAX_ENTRY ) 
			retval = ST_OVFL;
		else
			retval = ST_LOCK;
	} else { /* insert clist into APD */
		int i;

		/* shift slots upwards */
		for (i = (apd->data.n_apd - 1); i >= pos; i--)
			apd->data.clist[i+1] = apd->data.clist[i];

                /* AE - updated so the maintenance of n_apd is consistent 
                 * across the kernel code */
                //i = apd->data.n_apd++;
                //pos = (pos < i) ? pos : i;
                apd->data.n_apd = ((pos + 1 <= apd->data.n_apd) 
                                ? apd->data.n_apd : (pos+1));

		apd->data.clist[pos].address = (void *)clist;
		apd->data.clist[pos].passwd = passwd;
	}

	_lock_unlock(&apd->lock);

	return retval;
}

int
apd_slot_delete(apd_t * apd, apdpos_t pos)
{
	int retval = 0;

	_lock_lock(&apd->lock);

	if ((pos >= apd->data.n_apd) || (pos < apd->data.n_locked)) {
		if( pos < apd->data.n_locked )
			retval = ST_LOCK;
		else
			retval = 1;
	} else { /* shift slots downwards */
		int i;

		apd->data.n_apd--;
		for (i = pos; i < apd->data.n_apd; i++)
			apd->data.clist[i] = apd->data.clist[i+1];

		apd->data.clist[i] = NULL_CAP;
	}

	_lock_unlock(&apd->lock);

	return retval;
}

int
apd_slot_lock(apd_t * apd, apdpos_t pos)
{
	int retval = 0;

	/* check for invalid parameter */
	if (((pos < 0) && (pos != -1)) || (pos >= APD_MAX_ENTRY))
		return 1;

	_lock_lock(&apd->lock);

	if (pos == -1)
		apd->data.n_locked = APD_MAX_ENTRY;
	else if (pos >= apd->data.n_locked)
		apd->data.n_locked = pos + 1;
	else
		retval = 1;

	_lock_unlock(&apd->lock);

	return retval;
}

void
apd_flush(apd_t * apd)
{
	_lock_lock(&apd->lock);
	vcache_flush(VCACHE(apd));
	_lock_unlock(&apd->lock);
}

void
clist_autoadd(apd_t * apd, cap_t cap)
{
	clist_t *clist;
	int i;

	clist = (clist_t *)apd->data.clist[0].address;

	i = clist->n_caps++;
	clist->caps[i] = cap;
}


/* 
 * apd_close
 * 
 * Attempts to free apd and associated data
 * If APD is still in use, marks it to be freed (sets closing to true) and
 * removes all cache references. Returns false
 * If the APD is not used, it is freed. Returns true.
 *
 * APD lock should already be held by the calling thread
 *
 * FIXME: Only removes pdx cache entries, should also remove apd cache entries
 */
bool
apd_close(apd_t * apd, bool cache_lock_held)
{
        apd_thread_t	*thrd_elt, *thrd_tmp;
	l4_task_t	*task_elt, *task_tmp;

        if (apd->closing == false)
		apd_pdx_cache_remove(apd, cache_lock_held);
	if (apd->users != 0) {
		apd->closing = true;
		return false;
        }
        
#ifdef ALPHAENV
        kprintf( "Closing an APD! Everyone dies now!!\n" );
#endif
        
	/* free cached threads and stacks */
	thrd_elt = apd->threads;
	while (thrd_elt != NULL) {
		kfree(thrd_elt->mthread);
		if (thrd_elt->stack_base != NULL)
			object_delete(thrd_elt->stack_base);
        	thrd_tmp = thrd_elt;
		thrd_elt = thrd_elt->next;
		kfree(thrd_tmp);
	}

	/* free task structs */
	task_elt = apd->tasks;
        while (task_elt != NULL) {
                task_tmp = task_elt;
               	task_elt = task_elt->next;
		task_release(task_tmp);
	}
 
	vcache_free(VCACHE(apd));

	_lock_lock(&apd_table.lock);

	if (apd->pprev)
		*apd->pprev = apd->next;

	_lock_unlock(&apd_table.lock);

        kfree(apd);
	return true;
}

/* get a new stack in the APD, returns NULL on error */
void *
apd_get_stack(apd_t * apd, size_t size)
{
	passwd_t stack_pwd;
	object_t *stack_obj;
	cap_t cap;

	/* allocate new stack object */
	stack_pwd = mrandom();
	stack_obj = object_create(size, NULL, stack_pwd);
	if (stack_obj == NULL)
		return NULL;

	/* add capability for it to the APD */
	cap.address = stack_obj->base;
	cap.passwd = stack_pwd;
	clist_autoadd(apd, cap);

        return stack_obj->base;
}

/* insert an entry in the PDX APD cache */
static void
apd_pdx_cache_insert(apd_t *orig, cap_t clist, apd_t *new, 
		     struct apd_cache_id *id)
{
	time_t oldest = apd_pdx_cache.table[0].timestamp;
	apd_t *apd;
	int pos = 0, i;

	_lock_lock(&apd_pdx_cache.lock);
	for (i = 0; i < APD_PDX_CACHE_SIZE; i++) {
		apd = apd_pdx_cache.table[i].apd;
		if (apd == NULL) {
			pos = i;
			break;
		} else if (apd_pdx_cache.table[i].timestamp < oldest) {
			pos = i;
			oldest = apd_pdx_cache.table[i].timestamp;
		}
	}

	if (apd != NULL) {
		_lock_lock(&apd->lock);
		if (!apd_close(apd, true))
			_lock_unlock(&apd->lock);
		assert(apd_pdx_cache.table[i].apd == NULL);
	}

	apd_pdx_cache.table[pos].orig = orig;
	apd_pdx_cache.table[pos].clist = clist;
	apd_pdx_cache.table[pos].apd = new;
	apd_pdx_cache.table[pos].timestamp = gettime_lazy();

	/* copy in the ID */
	if( id != NULL ) {
		id->pos = pos;
		id->serial = new->serial;
	}

	_lock_unlock(&apd_pdx_cache.lock);
}

/* lookup an entry in the PDX APD cache, returns NULL if not found */
inline static apd_t *
apd_pdx_cache_lookup(apd_t *orig, cap_t clist,
		     struct apd_cache_id *id)
{
	apd_t *apd = NULL;
	time_t now = gettime_lazy();
	int i;

	_lock_lock(&apd_pdx_cache.lock);
	for (i = 0; i < APD_PDX_CACHE_SIZE; i++) {
		apd = apd_pdx_cache.table[i].apd;
		if (apd_pdx_cache.table[i].orig == orig
			&& apd_pdx_cache.table[i].clist.address == clist.address
                        && apd_pdx_cache.table[i].clist.passwd == clist.passwd
                        && apd != NULL) {
			if (now - apd_pdx_cache.table[i].timestamp
						> APD_PDX_CACHE_TIMEOUT) {
				/* timed out, close the APD */
				_lock_lock(&apd->lock);
				if (!apd_close(apd, true))
					_lock_unlock(&apd->lock);
				assert(apd_pdx_cache.table[i].apd == NULL);
				break;
			} else {
				_lock_unlock(&apd_pdx_cache.lock);
				/* copy in the ID */
				if( id != NULL ) {
					id->pos = i;
					id->serial = apd->serial;
				}
				return apd_pdx_cache.table[i].apd;
			}
		}
	}
	_lock_unlock(&apd_pdx_cache.lock);
	return NULL;
}

/* remove an entry in the PDX APD cache, if it exists */
static void
apd_pdx_cache_remove(apd_t *apd, bool cache_lock_held)
{
	int i;

	if (cache_lock_held == false)
		_lock_lock(&apd_pdx_cache.lock);

	for (i = 0; i < APD_PDX_CACHE_SIZE; i++)
		if (apd_pdx_cache.table[i].apd == apd)
			apd_pdx_cache.table[i].apd = NULL;

	if (cache_lock_held == false)
		_lock_unlock(&apd_pdx_cache.lock);
}

/* check the ID on an entry and return and apd_t* if matched, else NULL */
apd_t *
apd_pdx_cache_check_id( struct apd_cache_id *id )
{
	int pos;
	struct apd_pdx_cache_entry *ent;
	time_t now;
	apd_t *ret = NULL;

	assert( id != NULL );

	now = gettime_lazy();

	pos = id->pos;
	
	/* sanity checks! */
	assert( pos >= 0 && pos < APD_PDX_CACHE_SIZE );
	assert( id->serial < apd_serial );
		
	ent = &apd_pdx_cache.table[pos];

	_lock_lock(&apd_pdx_cache.lock);

	/* check the entry is semi-valid */
	if( ent->apd == NULL )
		goto out;

	/* this will cause the slow path and an expiry */
	if (now - ent->timestamp > APD_PDX_CACHE_TIMEOUT)
		goto out;

	/* check the APD's serial against the ID's */
	if( ent->apd->serial != id->serial )
		goto out;

	/* if we've got this far, it's OK! */
	ret = ent->apd;

 out:
	_lock_unlock(&apd_pdx_cache.lock);

	return ret;
}
