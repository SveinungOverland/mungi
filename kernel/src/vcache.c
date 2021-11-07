/****************************************************************************
 *
 *      $Id: vcache.c,v 1.12.2.1 2002/08/29 04:32:05 cgray Exp $
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

/*
 * Mungi validation cache
 *
 * Each APD contains a validation cache. This is a mapping from an object
 * address-range to the rights this APD has for the object.
 */

#include "mungi/kernel.h"
#include "mungi/mm.h"
#include "mungi/vcache.h"
#include "mungi/security.h"
#include "mungi/lock.h"
#include "mungi/limits.h"

#define VCACHE_TIMEOUT ((time_t)60) /* 1 minute */
/* WARNING: Macro uses addr twice */
#define IN_RANGE(addr, base, limit) (((addr) >= (base)) && ((addr) < (limit)))

struct vcache_entry {
	void * base;
	void * limit;
};

struct vcache_info {
	time_t timestamp;
	access_t rights;
	object_t * obj;
	cap_t * cap;
        int pdx_slot;
};

struct vcache {
	spinlock_t lock;
	int last;	/* last entry that matched */
        struct vcache_entry cache_obj[VCACHE_SIZE];	/* vcache */
        struct vcache_info cache_inf[VCACHE_SIZE];	/* vcache info */
};

inline static bool check_entry(struct vcache_entry *, struct vcache_info *,
			       const void *, access_t, time_t now);


vcache_t *
vcache_new(void)
{
	vcache_t * new;

	new = (vcache_t *)kzmalloc(sizeof(*new));

	spin_lock_init(&new->lock);

	return new;
}

void
vcache_free(vcache_t * vcache)
{
        assert(_lock_locked(&vcache->lock) == 0);
	kfree(vcache);
}

void
vcache_flush(vcache_t * vcache)
{
	spin_lock(&vcache->lock);

	vcache->last = 0;
	memset(&vcache->cache_obj, 0, sizeof(struct vcache_entry)*VCACHE_SIZE);

	spin_unlock(&vcache->lock);
}

void
vcache_flush_addr(vcache_t * vcache, const void *addr)
{
	struct vcache_entry *entry;
	int i;

	/* acquire the vcache lock */
	spin_lock(&vcache->lock);

	/* look for any cache entry that includes the address and flush */
	for (i = 0; i < VCACHE_SIZE; i++) {
		entry = &((vcache->cache_obj)[i]);
		if (IN_RANGE(addr, entry->base, entry->limit))
			entry->base = entry->limit = NULL;
	}
 
	/* release the vcache lock */
 	spin_unlock(&vcache->lock);
}


inline object_t *
vcache_check(vcache_t * vcache, const void * addr, access_t access,
	     cap_t ** cap, int * pdx_slot)
{
	int i;
	object_t * obj = NULL;
        time_t now = gettime();
        struct vcache_entry *entry;
	struct vcache_info *info;

	/* deny any access to addr 0 (otherwise we'll match on empty entries) */
	if (addr == NULL)
		return NULL;

        /* acquire the vcache lock */
	spin_lock(&vcache->lock);

	/* check the last entry hit */
	entry = &((vcache->cache_obj)[vcache->last]);
	info = &((vcache->cache_inf)[vcache->last]);
	if (check_entry(entry, info, addr, access, now)) {
		obj = info->obj;
		*cap = info->cap;
                *pdx_slot = info->pdx_slot;
		goto out;
	}
 
	/* otherwise check the whole cache for a match in the range with the
	 * correct permissions */
	for (i = 0; i < VCACHE_SIZE; i++) {
		entry = &((vcache->cache_obj)[i]);
		info = &((vcache->cache_inf)[i]);
		if (check_entry(entry, info, addr, access, now)) {
			obj = info->obj;
			*cap = info->cap;
                        *pdx_slot = info->pdx_slot;
			vcache->last = i;
			goto out;
 		}

	}

out:
        /* release the lock */
	spin_unlock(&vcache->lock);

	return obj;
}

inline static bool
check_entry(struct vcache_entry *entry, struct vcache_info *info,
			const void *addr, access_t access, time_t now)
{
	/* check if the entry refers to the correct object */
	if ((access & M_PDX) != 0) {
		if (addr != entry->base || addr != entry->limit)
			return false;
	} else {
		if (!IN_RANGE(addr, entry->base, entry->limit))
			return false;
	}

	/* check the timestamp */
	if (now - info->timestamp > VCACHE_TIMEOUT) {
		entry->base = entry->limit = NULL;
		return false;
	}

	/* verify that we are not caching negative rights */
	assert((info->rights & M_NOT) == 0);

	/* check if the entry has sufficient rights */
	if (((~(info->rights)) & access) != 0)
		return false;

	/* done -- access OK */
	return true;
}

void
vcache_add(vcache_t * vcache, object_t * obj, access_t access, cap_t * cap,
                int pdx_slot)
{
	int i;
	int slot = 0;
	void * base, * limit;
	time_t current_slot_timestamp = gettime() + 1;
	struct vcache_entry *entry;
	struct vcache_info *info;

	/* negative capabilities should never be cached -- why 
	 * optimise the failure case? */
	assert((access & M_NOT) == 0);
 
	/* pdx rights should never be mixed with other rights */
	assert(((access & M_PDX) == 0) || ((access & M_OWNER) == 0));

	/* get segment info from the object */
	spin_lock(&obj->lock);
	base = obj->base;
	limit = obj->end;
	spin_unlock(&obj->lock);

        /* acquire the vcache lock */
	spin_lock(&vcache->lock);

        /* find the slot to store -- either an empty one or the oldest one */
	for (i = 0; i < VCACHE_SIZE; i++) {
                entry = &((vcache->cache_obj)[i]);
		info = &((vcache->cache_inf)[i]);
		if (entry->base == NULL) {
			slot = i;
			break;
		}
                if (info->timestamp < current_slot_timestamp) {
			current_slot_timestamp = info->timestamp;
 			slot = i;
                }
	}

	/* fill in slot */
	entry = &((vcache->cache_obj)[slot]);
	info = &((vcache->cache_inf)[slot]);

	/* for PDX validations the valid address range is only the target
	 * address for all other rights the range is the whole object */
	if ((access & M_PDX) != 0) {
		entry->base = cap->address;
		entry->limit = cap->address;
	} else {
		entry->base = base;
		entry->limit = limit;
	}
	info->obj = obj;
	info->rights = access;
	info->cap = cap;
        info->pdx_slot = pdx_slot;
	info->timestamp = gettime();

	/* set this as the last item accessed */
	vcache->last = slot;
 
	/* release the vcache lock */
	spin_unlock(&vcache->lock);
}
