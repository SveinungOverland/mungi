/****************************************************************************
 *
 *      $Id: security.c,v 1.10.2.1 2002/08/29 04:32:02 cgray Exp $
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

/* Mungi security validation */

#include "mungi/kernel.h"
#include "mungi/objects.h"
#include "mungi/mm.h"
#include "mungi/lock.h"
#include "mungi/security.h"
#include "mungi/clist.h"
#include "mungi/apd.h"
#include "mungi/vcache.h"
#include "mungi/pending_requests.h"


cap_t NULL_CAP = {0,0};
typedef enum {ALLOWED, DENIED, UNKNOWN} validation_result_t;

/*
 * Validates that the given password gives at least the access specified.
 * Returns:
 *      ALLOWED  - Password has appropriate access
 *      UNKNOWN  - Password not found
 *      DENIED   - for negative
 *      PENDING  - result pending on MAC validation
 *      access_t* is the access allowed
 */
static validation_result_t do_validate_passwd(const object_t *, passwd_t,
					      access_t *, void *, int *,
					      apd_t *, struct pending_request *);
static validation_result_t do_validate_passwd_std(const object_t *, passwd_t,
						  access_t *);
static object_t * do_validate_obj(const void *, apd_t *, access_t, cap_t **, 
				  struct pending_request *);

/*
 * Validate that 'cap' gives at least the rights 'access'.
 *
 * This should only ever be called when cloning APD's -- it should not be used
 * as a general validation technique. Doesn't handle PDX validations.
 */
bool
validate_cap(cap_t cap, access_t access)
{
	object_t *obj;
	int ret;

	if (access & M_PDX)
		return false;

	/* find object */
	obj = object_find(cap.address);
	if (obj == NULL)
		return false;

	/* check password */
	ret = do_validate_passwd_std(obj, cap.passwd, &access);

	return ((ret == ALLOWED) ? true : false);
}

/*
 * Validate 'access' to 'addr' with regards to the specified apd.
 */
object_t *
validate_obj(const void * addr, apd_t * apd, access_t access, cap_t ** rcap,
	     struct pending_request *state)
{
	return validate_obj_pdx(addr, apd, access, rcap, state);
}

object_t *
validate_obj_pdx(const void * addr, apd_t * apd, access_t access, cap_t ** rcap,
		 struct pending_request *state)
{
	return do_validate_obj(addr, apd, access, rcap, state);
}

static object_t *
do_validate_obj(const void * addr, apd_t * apd, access_t access, cap_t ** rcap, 
		struct pending_request *state)
{
	int i, j;
	object_t * obj = NULL;
	cap_t *tmp;
	cap_t cap;
	clist_t *clist;
	int pdx_slot = -1;
	validation_result_t val_result;

	/* hack to allow user to specify NULL return */
 	if (rcap == NULL)
		rcap = &tmp;

	/* check for validation in cache */
        obj = vcache_check(VCACHE(apd), addr, access, rcap, &pdx_slot);
	if (obj != NULL) {
		return obj;
	}

        /* cache missed -- proceed with validation */
        
	/* find object */
	obj = object_find(addr);
	if (obj == NULL) {
		kprintf( "VALIDATE_OBJ: object not found\n" );
		return obj;
	}

	/* search APD */
	_lock_lock(&apd->lock);

	for (i = 0; i < SLOT_COUNT(apd); i++) {
                /* get clist -- check CLIST header to make sure it is valid */
                clist = (clist_t *)apd->data.clist[i].address;
		if (IS_NOT_CLIST(clist)) {
			continue;
		}

		/* check every cap in the clist */
		for (j = 0; j < clist->n_caps; j++) {
                        /* get the address of the capability */
			cap = clist->caps[j];

                        /* see if it refers to the object we are validating */
			if (cap.address<obj->base || cap.address>=obj->end)
 				continue;

                        /* check the password (from our APD) against each
			 * password in the object */
			val_result = do_validate_passwd(obj, cap.passwd,
					&access, (void *) addr, &pdx_slot,
					apd, state);
			switch (val_result) {
			case ALLOWED:
                                *rcap = &clist->caps[j];

				/* if allowed add it to the cache */
				vcache_add(VCACHE(apd), obj, access, 
					   &clist->caps[j], pdx_slot);

				goto out;
                        case UNKNOWN:	/* if not mentioned then keep looking */
				break;
                        case DENIED:	/* if explicitly forbidden (M_NOT) then
					 * exit as failed */
				*rcap = &clist->caps[j];
				obj = NULL;
				goto out;
			}
		}
	}

        /* validation failed */
	obj = NULL;
out:	
	_lock_unlock(&apd->lock);
	return obj;
}

/*
 * Validate range
 */
bool
validate_range(object_t * obj, const void * start, size_t length)
{
	return true; /* FIXME */
}

/* 
 * Given an object, a passwd, and an access -- validate 
 */
static validation_result_t
do_validate_passwd_std(const object_t *obj, passwd_t passwd, access_t *access)
{
        int i;
	validation_result_t result = UNKNOWN;
	access_t rights;

	/* go through every passwd in the object and look for deny or accept */
	for (i = 0; i < obj->info.n_caps; i++) {
		/* check the passwd matches */
		if (((obj->info).passwd)[i] != passwd)
			continue;
		rights = ((obj->info).rights[i]);
                /* make sure it's not denying some of the rights */
		if (((*access) & rights) && (rights & M_NOT)) {
			result = DENIED;
			break;
		}

		/* see if it's allowing all the rights */
		if (((*access) & (~rights)) == 0) {
			result = ALLOWED;
			*access = rights;
			break;
                }
	}
        
	return result;
}


static validation_result_t
do_validate_passwd_pdx(const object_t * obj, passwd_t passwd,
		       access_t * access, void *proc, int *pdx_slot,
		       apd_t * apd, struct pending_request *state)
{
	int i, j;

	/* have to find a PDX entry with matching passwd and entry point */
	for (i = 0; i < obj->info.n_pdx; i++) {
		/* make sure the passwd matches */
		if ((obj->info.pdx.passwd)[i] != passwd)
			continue;

		/* look for an entry point */
		for (j = (obj->info.pdx.n_entry)[i];
					j < (obj->info.pdx.x_entry[i]); j++) {
			if (((obj->info.pdx.entry)[j]) == proc)
				goto found;
		}
	}

	/* no match found */
	return UNKNOWN;

found:
	/* return the PDX slot to the caller */
	if (pdx_slot != NULL)
		*pdx_slot = i;

	return ALLOWED;
}

static validation_result_t
do_validate_passwd(const object_t * obj, passwd_t passwd,
		   access_t * access, void * proc, int *pdx_slot,
		   apd_t * apd, struct pending_request *state)
{
	/* make sure an access doesn't mix M_PDX with others */
	assert((((*access) & M_PDX) == 0) || (((*access) & M_OWNER) == 0));

	/* demultiplex */
	if (((*access) & M_PDX) == 0)
		return do_validate_passwd_std(obj, passwd, access);
	else
		return do_validate_passwd_pdx(obj, passwd, access,
					      proc, pdx_slot, apd, state);
}

void
security_init(void)
{
	system_clist.caps[0].address = l4_setup.upcall.base; 
}
