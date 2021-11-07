/****************************************************************************
 *
 *      $Id: threads.h,v 1.13.2.2 2002/08/30 05:59:57 cgray Exp $
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

#ifndef __M_THREADS_H__
#define __M_THREADS_H__

typedef struct mthread mthread_t; 
typedef struct l4_task l4_task_t; 

#include "mungi/l4_generic.h"
#include "mungi/exception.h"
#include "mungi/syscallbits.h"
#include "mungi/waitq.h"
#include "mungi/apd.h"

struct l4_task {
	l4_taskid_t     task;                   /* L4 task ID */
	apd_t           *apd;                   /* APD for this task */
	mthread_t       *l4_threads[L4_THREADS_PER_TASK]; /* active threads */
	l4_task_t       *next;                  /* next in APD task list */
	int             thread_free;            /* # free threads free in task */
};

/* 
 * thread state 
 */
typedef enum { THREAD_OK, THREAD_SLEEP, THREAD_ZOMBIE, THREAD_DEAD, 
		THREAD_WAITING } state_t;

struct mthread {
	mthreadid_t     tid;                    /* mungi thread ID */
	l4_threadid_t   l4id;                   /* L4 thread ID */
	mthread_t	*next;			/* next in hash chain */ 
	mthread_t	**pprev;		/* previous ptr in hash chain */
	l4_task_t       *task;                  /* l4 task info (above) */
	mthread_t       *parent;                /* parent thread */
	mthread_t       *children;              /* unused? */
	mthread_t       *sibling;               /* unused? */
	state_t         state;                  /* thread state */
	int             lasterror;              /* error code (FIXME: unused) */
	waitq_t         *zombies;               /* zombie children threads */
	waitq_t         *waiters;               /* waiting parent threads */
	mthread_t       *waiting;               /* thread we are waiting for */
	mthread_t       *PDX;                   /* calling thread (if PDX) */
	excpthndlr_t    excpt_hndlr[EXP_MAX];   /* exception handlers */
	threadinfo_t    info;                   /* user thread info (types.h) */
        apd_thread_t	*apd_threadinfo;        /* L4 id and stack */
};


#define APD(thread)	((thread)->task->apd)
#define EXCPT(thread)	((thread)->excpt_hndlr)
#define TID(thread)	((thread)->tid)
#define L4ID(thread)	((thread)->l4id)
#define ERROR(thread)	((thread)->lasterror)
#define STATUS(thread)	ERROR(thread)

#define THREAD_SLEEPING(t) ((t)->state == THREAD_SLEEP)
#define THREAD_NOT_SLEEPING(t) ((t)->state != THREAD_SLEEP)
#define THREAD_ZOMBIE(t) ((t)->state == THREAD_ZOMBIE)
#define THREAD_WAITING(t) ((t)->state == THREAD_WAITING)
#define THREAD_DEAD(t) ((t)->state == THREAD_DEAD)
#define THREAD_NOT_DEAD(t) ((t)->state != THREAD_DEAD)
#define THREAD_ISDETACHED(t) ((t)->info.flags & THREAD_DETACHED)
#define THREAD_IN_PDX(t) ((t)->PDX != NULL)

/* forward decl. for threads */
struct pending_request;


void thread_init(l4_threadid_t);
mthread_t *thread_find_from_l4id(l4_threadid_t);
mthread_t *thread_find_from_mid(mthreadid_t);
void thread_die(mthread_t *);
int thread_delete(mthreadid_t, int, bool, mthread_t *);
bool thread_sleep(mthreadid_t, time_t, int *, mthread_t *);
int thread_info(mthreadid_t, threadinfo_t *, mthread_t *);
int thread_resume(mthreadid_t, mthread_t *);
int thread_wait(mthreadid_t, int *, mthreadid_t *, mthread_t *);
int thread_create(syscall_t *, apd_t *, threadinfo_t *, mthread_t *);
int thread_pdx_create(pdx_t, cap_t, apd_t *, mthread_t *, 
		      struct pending_request *, struct pending_request **);
void thread_return(cap_t, mthread_t *);
void task_release(l4_task_t *);

#endif /* __M_THREADS_H__ */
