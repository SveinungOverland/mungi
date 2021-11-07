/****************************************************************************
 *
 *      $Id: semaphore.c,v 1.6 2002/05/31 06:27:40 danielp Exp $
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

/* semaphores */

#include "mungi/kernel.h"
#include "mungi/semaphore.h"
#include "mungi/waitq.h"
#include "mungi/objects.h"
#include "mungi/mm.h"
#include "mungi/lock.h"
#include "status.h"

int
sem_create(object_t *obj, void *address, int value)
{
	int i, ret = ST_SUCC, found = -1;
	sem_t *semp;

	_lock_lock(&obj->lock);

	semp = obj->semp;
	if (semp == NULL) { /* allocate semaphore block */
		semp = obj->semp = kmalloc(sizeof(*semp));
		if (semp == NULL) {
			ret = ST_ERR;
			goto sem_create_out;
		}
	}

	for (i = 0; i < SEM_MAX; i++)
		if ((found < 0) && (semp->semaphores[i].address == NULL)) {
			found = i;
		} else if (semp->semaphores[i].address == address) {
			/* semaphore already exists */
			ret = fail;
			goto sem_create_out;
		}

	if (found >= 0) { /* create semaphore */
		semp->semaphores[found].address = address;
		semp->semaphores[found].value = value;
		semp->count++;
	} else
		ret = fail;

sem_create_out:
	_lock_unlock(&obj->lock);

	return ret;
}

int
sem_delete(object_t *obj, void *address)
{
	int i, ret = success;
	sem_t *semp;

	_lock_lock(&obj->lock);

	semp = obj->semp;
	if (semp == NULL) {
		ret = ST_SEMA;
		goto sem_delete_out;
	}

	for (i = 0; i < SEM_MAX; i++)
		if (semp->semaphores[i].address == address) { /* found it */
			mthread_t *thread;

			semp->semaphores[i].address = NULL;
			semp->count--;
			while ((thread = /* fail waiting threads */
				waitq_remove(&semp->semaphores[i].waitq))) {
				upcall_send(L4ID(thread), fail);
			}

			if (semp->count == 0) {
				kfree(semp);
				obj->semp = NULL;
			}
			goto sem_delete_out;
		}

	/* failed to find semaphore */
	ret = ST_SEMA;
sem_delete_out:
	_lock_unlock(&obj->lock);

	return ret;
}

int
sem_wait(object_t *obj, void *address, mthread_t *thread, int *retval)
{
	int i, make_thread_wait = false;
	sem_t *semp;

	_lock_lock(&obj->lock);

	semp = obj->semp;
	if (semp == NULL) {
		*retval = fail;
		goto sem_wait_out;
	}

	*retval = success;
	for (i = 0; i < SEM_MAX; i++)
		if (semp->semaphores[i].address == address) {
			if ((semp->semaphores[i].value--) <= 0) {
				if (waitq_add(&semp->semaphores[i].waitq, 
							thread)) {
					/* error adding to wait Q */
					semp->semaphores[i].value++;
					break;
				} else {
					make_thread_wait = true;
				}
			}
			goto sem_wait_out;
		}

	/* failed to find semaphore */
	*retval = ST_PROT;
sem_wait_out:
	_lock_unlock(&obj->lock);

	return make_thread_wait;
}

int
sem_signal(object_t *obj, void *address)
{
	int i, ret = success;
	sem_t *semp;

	_lock_lock(&obj->lock);

	semp = obj->semp;
	if (semp == NULL) {
		ret = fail;
		goto sem_signal_out;
	}

	for (i = 0; i < SEM_MAX; i++)
		if (semp->semaphores[i].address == address) { /* found it */
			mthread_t *thread;

			semp->semaphores[i].value++;
			/* get waiting thread */
			thread = waitq_remove(&semp->semaphores[i].waitq);
			if (thread != NULL) {
				if (upcall_send(L4ID(thread), success)) {
					ret = fail;
					--semp->semaphores[i].value;
				}
			}
			goto sem_signal_out;
		}

	/* failed to find semaphore */
	ret = fail;
sem_signal_out:
	_lock_unlock(&obj->lock);

	return ret;
}
