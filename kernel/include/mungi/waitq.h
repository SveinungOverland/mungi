/****************************************************************************
 *
 *      $Id: waitq.h,v 1.2 2002/05/31 05:49:38 danielp Exp $
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

#ifndef __M_WAITQ_H_
#define __M_WAITQ_H_

typedef struct waitq waitq_t;

#include "mungi/kernel.h"
#include "mungi/threads.h"

struct waitq {
	mthread_t *thread;
	waitq_t *next;
	waitq_t **pprev;
};

int waitq_add(waitq_t **, mthread_t *);
void waitq_cancel(waitq_t **, mthread_t *);
mthread_t *waitq_remove(waitq_t **);

#endif /* __M_WAITQ_H_ */
