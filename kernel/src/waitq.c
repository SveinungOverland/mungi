/****************************************************************************
 *
 *      $Id: waitq.c,v 1.3 2002/05/31 06:27:40 danielp Exp $
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

/* wait queue management */

#include "mungi/waitq.h"
#include "mungi/kernel.h"
#include "mungi/mm.h"

int
waitq_add(waitq_t **waitq, mthread_t *thread)
{
	waitq_t *new, *ptr;

	/* allocate a new wait Q item */
	new = (waitq_t *)kmalloc(sizeof(waitq_t));
	if (new == NULL)
		return fail;

	/* find end of waitq */
	ptr = *waitq;
	for (; (ptr != NULL) && (ptr->next != NULL); ptr = ptr->next)
		;

	/* add to waitq */
	new->next = NULL;
	new->thread = thread;
	if (ptr != NULL) {
		ptr->next = new;
		new->pprev = &ptr->next;
	} else {
		*waitq = new;
		new->pprev = waitq;
	}

	return success;
}

mthread_t *
waitq_remove(waitq_t **waitq)
{
	mthread_t *thread;
	waitq_t *ptr;

	ptr = *waitq;
	if (ptr == NULL)
		return NULL;
	
	/* remove from wait Q */
	thread = ptr->thread;
	*ptr->pprev = ptr->next;
	kfree(ptr);

	return thread;
}

void
waitq_cancel(waitq_t **waitq, mthread_t *thread)
{
	waitq_t *ptr;

	ptr = *waitq;
	if (ptr == NULL)
		return;
	
	for (; ptr != NULL; ptr = ptr->next)
		if (ptr->thread == thread) {
			waitq_t *tmp;

			tmp = ptr;
			*ptr->pprev = ptr->next;
			ptr = ptr->next;
			kfree(tmp);
			continue;	
		}
}
