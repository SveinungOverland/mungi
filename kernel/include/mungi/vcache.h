/****************************************************************************
 *
 *      $Id: vcache.h,v 1.6 2002/07/22 10:17:39 cgray Exp $
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

#ifndef __MUNGI_K_VCACHE_H
#define __MUNGI_K_VCACHE_H

typedef struct vcache vcache_t;

#include "mungi/kernel.h"
#include "mungi/objects.h"


/*
 * Allocate a new validation cache.
 */
vcache_t * vcache_new(void);

/*
 * free a validation cache.
 */
void vcache_free(vcache_t *);

/*
 * Flush all entries from the validation cache
 */
void vcache_flush(vcache_t *);

/*
 * Flush all entries from the validation cache referencing a given address
 */
void vcache_flush_addr(vcache_t *, const void *);


/*
 * Check if we have a matching entry in the validation cache.
 * returns:
 *	NULL if a validation matching the access is not found.
 *	otherwise the pointer to the object.
 */
inline object_t * vcache_check(vcache_t *, const void *, access_t, 
			       cap_t **, int *);

/*
 * Add an entry to the validation cache.
 * a cap of NULL indicates that the entry is a negative capability.
 */
void vcache_add(vcache_t *, object_t *, access_t, cap_t *, int);

#endif /* __MUNGI_K_VCACHE_H */
