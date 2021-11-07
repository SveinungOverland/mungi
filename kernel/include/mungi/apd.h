/****************************************************************************
 *
 *      $Id: apd.h,v 1.8.2.1 2002/08/29 04:31:56 cgray Exp $
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

#ifndef __M_APD_H__
#define __M_APD_H__

typedef struct apd apd_t;
typedef struct apd_thread apd_thread_t;

#include "mungi/kernel.h"
#include "mungi/lock.h"
#include "mungi/vcache.h"
#include "mungi/threads.h"

#define VCACHE(a)		((a)->vcache)
#define SLOT_COUNT(a)		((a)->data.n_apd)

struct apd {
	lock_t		lock;
	apddesc_t	data;		/* APD slots & lock count */
	uint32_t	flags;		/* general APD flags */
	int		users;		/* reference count for APD */
	l4_task_t   	*tasks;		/* L4 tasks associated with APD */
	vcache_t	*vcache;	/* Validation cache for APD */
        apd_thread_t	*threads;	/* list of free threads/stacks in APD */
	const apddesc_t	*pd;		/* PD that APD is joined to or NULL */
	apd_t		*next;		/* PD hash table next */
	apd_t		**pprev;	/* PD hash table pprev */
        bool            closing;        /* Flag that apd should be closed
                                                (FIXME: remove this) */
	uintptr_t       serial;         /* Unique ID */
};

struct apd_thread {
	l4_task_t	*task;
	uint8_t		lthread;
	void		*stack_base;	/* NULL if never used */
	size_t		stack_size;
	mthread_t	*mthread;	/* associated mthread struct */
	apd_thread_t	*next;		/* next in list, when not running */
};

/* struct for the PDX cache */
struct apd_cache_id {
	int pos;          /* position in cache */
	uintptr_t serial; /* serial reference */
};
apd_t * apd_pdx_cache_check_id( struct apd_cache_id * );


void apd_init(void);
apd_t *apd_find_or_create(const apddesc_t *);
apd_t *apd_from_template(const apddesc_t *);
apd_t *apd_pdx_find_or_create(const apddesc_t *, cap_t,
			      struct apd_cache_id *id);
apd_t *apd_pdx_merge(apd_t *, cap_t, struct apd_cache_id *id);
bool apd_close(apd_t *, bool);

void apd_get(apd_t *, apddesc_t *);
int apd_slot_delete(apd_t *, apdpos_t);
int apd_slot_insert(apd_t *, const clist_t *, passwd_t, apdpos_t);
int apd_slot_lock(apd_t *, apdpos_t);
void apd_flush(apd_t *);
void *apd_get_stack(apd_t *, size_t);

void clist_autoadd(apd_t *, cap_t);

#endif /* __M_APD_H__ */
