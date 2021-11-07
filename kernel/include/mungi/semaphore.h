/****************************************************************************
 *
 *      $Id: semaphore.h,v 1.4 2002/08/23 08:24:20 cgray Exp $
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

#ifndef __M_SEMAPHORE_H_
#define __M_SEMAPHORE_H_

typedef struct sem sem_t;

#include "mungi/kernel.h"
#include "mungi/objects.h"
#include "mungi/threads.h"
#include "mungi/waitq.h"

#define SEM_MAX	16
struct sem {
	int count;
	struct {
		void	* address;
		int	value;
		waitq_t	* waitq;
	} semaphores[SEM_MAX];
};

int sem_create(object_t *, void *, int);
int sem_delete(object_t *, void *);
int sem_wait(object_t *, void *, mthread_t *, int *);
int sem_signal(object_t *, void *);

#endif /* __M_SEMAPHORE_H_ */
