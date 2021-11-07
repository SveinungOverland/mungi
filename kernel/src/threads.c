/****************************************************************************
 *
 *      $Id: threads.c,v 1.38.2.2 2002/08/30 06:00:01 cgray Exp $
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

/* thread related calls */

#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/objects.h"
#include "mungi/lock.h"
#include "mungi/security.h"
#include "mungi/upcall.h"
#include "mungi/apd.h"
#include "mungi/mm.h"
#include "mungi/pager.h"
#include "mungi/exception.h"
#include "mungi/limits.h"
#include "mungi/threads.h"
#include "mungi/pending_requests.h"
#include "status.h"
#include "mungi/clock.h"

/*
 * hash from mungi thread id's to mthread_t *'s 
 */
#define thread_hash(x)	((x) & (THREAD_HASHSIZE - 1))
static struct {
	lock_t		lock;
	mthread_t	**threads;
} mungi_threads;

static struct {
	lock_t		lock;
	l4_task_t 	**tasks;
	int		task_free;
	int		task_count;
} l4_tasks;

static lock_t kthreadid_lock;
static l4_threadid_t kthreadid;
static mthreadid_t next_tid = 1;

l4_threadid_t mungi_kernel;
l4_threadid_t asynchronous_pager;
l4_threadid_t default_pager;
static l4_threadid_t exception_handler;

/* private functions */
static mthread_t *thread_find_mthread_from_parent(mthreadid_t, mthread_t *);
static void thread_hash_remove(mthread_t *);
static l4_task_t *task_new(void);
l4_threadid_t kthread_start(void *ip,l4_threadid_t*pagerid,l4_threadid_t*excpt);
static bool start_task(l4_taskid_t *, void *);
static void do_threads_delete(mthread_t *, int, bool);
static void thread_release(mthread_t *);
static void notify_waiters(mthread_t *);
inline static void l4task_thread_add(mthread_t *);
inline static void l4task_thread_remove(mthread_t *);
inline static mthreadid_t get_tid(void);
inline static mthread_t *thread_get_mthread(apd_t *, mthread_t *, 
					    bool *, bool *);
inline static void *thread_setup_stack(mthread_t *, void *, size_t, apd_t *,
				       bool, void *, cap_t *);
inline static void *top_of_stack(void *base, size_t size);


void start_console_in(void);


/* finds a new thread in the given APD, returns NULL on error
 * new_task is set true iff the thread is in a previously unused L4 task
 * the APD should aready be locked */
inline static mthread_t *
thread_get_mthread(apd_t *apd, mthread_t *parent, bool *new_task, 
		   bool *new_thread)
{
	mthread_t	*new;
	l4_task_t	*l4_task;
	apd_thread_t	*apd_thread;

	apd->users++;

	/* first check the pool of already used threads in the APD */
	if (apd->threads != NULL) {
		apd_thread = apd->threads;
		apd->threads = apd_thread->next;
		new = apd_thread->mthread;
		new->parent = parent;

		/* clean out all the old parameters */
		new->children = NULL;
		new->sibling = NULL;
		new->state = THREAD_OK;
		new->lasterror = 0;
		new->zombies = NULL;
		new->waiters = NULL;
		new->waiting = NULL;
		new->PDX = NULL;
		bzero(&new->excpt_hndlr, sizeof(excpthndlr_t) * EXP_MAX);
		bzero(&new->info, sizeof(threadinfo_t));

		*new_task = false;
		*new_thread = false;

		return new;

	}

	*new_thread = true;

	/* allocate a Mungi thread structure */
	new = (mthread_t *)kmalloc(sizeof(mthread_t));
        if (new == NULL)
                return NULL;
        bzero(new,sizeof(mthread_t));
        new->parent = parent;
	
        /* Find a L4 task */
        for (l4_task = apd->tasks; l4_task != NULL; l4_task = l4_task->next)
		if (l4_task->thread_free)
			break;

	if (l4_task == NULL) { /* create a new L4 task */
		l4_task = task_new();
		if (l4_task == NULL) {
			kfree(new);
                        return NULL;
		}
		l4_task->apd = apd;
		l4_task->next = apd->tasks;
		apd->tasks = l4_task;
		*new_task = true;
	} else {
                *new_task = false;
        }


        apd_thread = kmalloc(sizeof(apd_thread_t));
	if (apd_thread == NULL) {
		kfree(new);
		return NULL;
	}

	apd_thread->task = l4_task;
	apd_thread->lthread = 0;
	apd_thread->stack_base = NULL;
	apd_thread->mthread = new;
	new->apd_threadinfo = apd_thread;
	new->task = apd_thread->task;
	return new;
}

/* get the next mungi thread id */
inline static mthreadid_t
get_tid(void)
{
	mthreadid_t tid;

	_lock_lock(&kthreadid_lock);
	tid = next_tid++;
	_lock_unlock(&kthreadid_lock);

	return tid;
}


inline static void
l4task_thread_add(mthread_t *thread)
{
	register int lthread, hashval;
        l4_task_t *l4_task;

	/*
	 * add thread to L4 task data
	 */
	l4_task = thread->apd_threadinfo->task;
	if (thread->apd_threadinfo->lthread == 0) {
		for (lthread = 1; lthread < L4_THREADS_PER_TASK; lthread++)
			if (l4_task->l4_threads[lthread] == NULL)
				break;
		assert(lthread < L4_THREADS_PER_TASK);
		thread->apd_threadinfo->lthread = lthread;
	} else {
		lthread = thread->apd_threadinfo->lthread;
	}
 
	thread->task = l4_task;
	thread->l4id = l4_task->task;
	thread->l4id.id.lthread = lthread;
	l4_task->l4_threads[lthread] = thread;
	l4_task->thread_free--;

	thread->tid = get_tid();

	/*
	 * add thread to hash table
	 */
	hashval = thread_hash(thread->tid);

	_lock_lock(&mungi_threads.lock);

	thread->next = mungi_threads.threads[hashval];  
	if (thread->next != NULL)
		thread->next->pprev = &thread->next;

	mungi_threads.threads[hashval] = thread;
	thread->pprev = &mungi_threads.threads[hashval];

	_lock_unlock(&mungi_threads.lock);
}

inline static void
l4task_thread_remove(mthread_t *thread)
{
	l4_task_t *l4_task;
	register int lthread;

	l4_task = thread->task;
	lthread = thread->apd_threadinfo->lthread;
	assert(l4_task->l4_threads[lthread] == thread);
	l4_task->l4_threads[lthread] = NULL;
	l4_task->thread_free++;

}


/* finds the top of a stack */
inline static void *
top_of_stack(void *base, size_t size)
{
	return &((uint64_t *)base)[(size / sizeof(uint64_t)) - 1];
}


inline static bool
start_task(l4_taskid_t *task, void *stack)
{
	l4_taskid_t ret;

#ifdef ALPHAENV
        ret = l4_task_new(*task, (uintptr_t)0x88, (uintptr_t)stack,
                           (uintptr_t)l4_setup.upcall.entry, default_pager);
#else
	ret = l4_task_new(*task, (uintptr_t)0x88, (uintptr_t)stack, 
			   (uintptr_t)l4_setup.upcall.entry, default_pager, 
			   exception_handler);
#endif

	if (l4_is_nil_id(ret))
		return true;

	*task = ret;
	return false;
}

/* creates and sets up stack for a new thread, returns NULL on error
 * the APD should aready be locked */
inline static void *
thread_setup_stack(mthread_t *thread, void *supplied_stack, size_t stack_size,
			apd_t *apd, bool new_task, void *ip, cap_t *param)
{
	void *stack, *user_stack = NULL;

	if (new_task) {
		if ((stack = apd_get_stack(apd, MTHREAD_STACK_SIZE)) == NULL)
			return NULL;
		stack = top_of_stack(stack, MTHREAD_STACK_SIZE);
		if (supplied_stack != NULL)
			user_stack = supplied_stack;
		else {
			user_stack = apd_get_stack(apd, stack_size);
			if (user_stack == NULL)
				return NULL;
			thread->apd_threadinfo->stack_base = user_stack;
			thread->apd_threadinfo->stack_size = stack_size;
			user_stack = top_of_stack(user_stack, stack_size);
		}
	} else if (supplied_stack != NULL)
		stack = supplied_stack;
	else if (thread->apd_threadinfo->stack_base != NULL
			&& thread->apd_threadinfo->stack_size >= stack_size)
		stack = top_of_stack(thread->apd_threadinfo->stack_base,
					stack_size);
	else {
		if (thread->apd_threadinfo->stack_base != NULL)
			object_delete(thread->apd_threadinfo->stack_base);
		stack = apd_get_stack(apd, stack_size);
		if (stack == NULL)
			return NULL;
		thread->apd_threadinfo->stack_base = stack;
		thread->apd_threadinfo->stack_size = stack_size;
		stack = top_of_stack(stack, stack_size);
	}
 
	/* place arguments on the stack */

	if (new_task) {
		/* 'stack' is for the upcall thread, this one is for the user */
		*(--(uintptr_t *)stack) = (uint64_t) user_stack;
	}

	*(--(uintptr_t *)stack) = (uintptr_t)ip;
	*(--(uintptr_t *)stack) = (uintptr_t)(param->address);
	*(--(uintptr_t *)stack) = (uintptr_t)(param->passwd);
	((uintptr_t *)stack)--;

        return stack;
}


mthread_t *
thread_find_from_l4id(l4_threadid_t tid)
{
	int task_id;
	mthread_t *thread = NULL;
	l4_task_t *task;

	task_id = tid.id.task;
	if ((task_id >= l4_setup.l4_task_start) &&
			(task_id <= l4_setup.l4_task_end)) {

		_lock_lock(&l4_tasks.lock);

		task = l4_tasks.tasks[task_id - l4_setup.l4_task_start];
		if (task != NULL)
			thread = task->l4_threads[tid.id.lthread];

		_lock_unlock(&l4_tasks.lock);
	}

	return thread;
}

mthread_t *
thread_find_from_mid(mthreadid_t tid)
{
	mthread_t *thread;
	int hashval;

	hashval = thread_hash(tid);

	_lock_lock(&mungi_threads.lock);

	thread = mungi_threads.threads[hashval];
	for (; thread != NULL; thread = thread->next)
		if (thread->tid == tid)
			break;

	_lock_unlock(&mungi_threads.lock);

	return thread;
}


static mthread_t *
thread_find_mthread_from_parent(mthreadid_t tid, mthread_t *parent)
{
	mthread_t *child, *ptr;

	if (tid == THREAD_MYSELF)
		return parent;

	ptr = child = thread_find_from_mid(tid);
	for (; ptr != NULL; ptr = ptr->parent)
		if (ptr == parent)
			return child;

	return NULL;
}

static void
thread_hash_remove(mthread_t *thread)
{
	register int hashval;

	hashval = thread_hash(thread->tid);

	_lock_lock(&mungi_threads.lock);

	*thread->pprev = thread->next;
	if (thread->next)
		thread->next->pprev = thread->pprev;

	_lock_unlock(&mungi_threads.lock);
}

int
thread_resume(mthreadid_t tid, mthread_t *caller)
{
	mthread_t *child;

	child = thread_find_mthread_from_parent(tid, caller);
	if ((child == NULL) || THREAD_NOT_SLEEPING(child))
		return fail;
	
	return upcall_resume(L4ID(child));
}

static void
thread_release(mthread_t *child)
{
        apd_t *apd = child->task->apd;
	apd_thread_t *apd_thread;
        
	thread_hash_remove(child);
	_lock_lock(&apd->lock);
	apd_thread = apd->threads;
	apd->threads = child->apd_threadinfo;
	apd->threads->next = apd_thread;
	apd->users--;
	if (apd->closing && apd_close(apd, false))
		return; /* don't unlock if we've just closed the APD */
	_lock_unlock(&apd->lock);
}

int
thread_wait(mthreadid_t tid, int *status, mthreadid_t *rtid, mthread_t *caller)
{
	mthread_t *child;
	int make_caller_wait = false;

	if (tid == THREAD_ANY) {
		/* check zombie list */
		child = waitq_remove(&caller->zombies); 
		if (child) {
			*status = child->lasterror;
			*rtid = tid;
			thread_release(child); /* child has now been reaped */
		} else if (caller->children) {
			caller->state = THREAD_WAITING;
			make_caller_wait = true;
		} else /* no children to wait for */
			*rtid = THREAD_NULL;
	} else {
		child = thread_find_mthread_from_parent(tid, caller);
		if ((child == NULL) || (THREAD_DEAD(child))) {
			*rtid = THREAD_NULL;
		} else if (THREAD_ZOMBIE(child)) {
			waitq_cancel(&caller->zombies, child);
			*status = child->lasterror;
			*rtid = tid;
			thread_release(child); /* child has now been reaped */
		} else {
			/* add to childs wait Q */
			if (waitq_add(&child->waiters, caller))
				*rtid = THREAD_NULL;
			else {
				assert(child->waiters != NULL);
				caller->state = THREAD_WAITING;
				make_caller_wait = true;
				caller->waiting = child;
			}
		}
	}

	return make_caller_wait;
}

int
thread_info(mthreadid_t tid, threadinfo_t *info, mthread_t *caller)
{
	mthread_t *child;

	child = thread_find_mthread_from_parent(tid, caller);
	if (child == NULL)
		return fail;

	memcpy(info, &child->info, sizeof(*info));

	return success;
}

int
thread_delete(mthreadid_t tid, int status, bool adopt, mthread_t *caller)
{
	mthread_t *child;
	int caller_return = true;

	/* find thread */
	child = thread_find_mthread_from_parent(tid, caller);
	if ((child == NULL) || THREAD_ZOMBIE(child) || THREAD_DEAD(child))
		goto error;

	if (child == caller)
		caller_return = false;
	
	/* do the recursive deletion */
	do_threads_delete(child, status, adopt);
error:
	return caller_return;
}

static void
do_threads_delete(mthread_t *thread, int status, bool adopt)
{
	excpthndlr_t handler;

	/* check state of thread */
	switch (thread->state) {
	case THREAD_SLEEP:
		thread->state = THREAD_OK;
		break;
	case THREAD_WAITING:
		thread->state = THREAD_OK;
		if (thread->waiting) {
			waitq_cancel(&(thread->waiting)->waiters, thread);
			thread->waiting = NULL;
		}
		break;
	case THREAD_DEAD: case THREAD_ZOMBIE:
		assert(0); /* should never get here */
	case THREAD_OK:
		;
	}

	/* see if there is a registered KILL handler */
	handler = EXCPT(thread)[E_KILL];
	if (handler) {
		/* call KILL handler */
		upcall_excp(L4ID(thread), handler, (void *)(uintptr_t)status, 
				E_KILL);
	} else {
		l4task_thread_remove(thread);
		/* delete thread */
		thread->lasterror = status;	/* set exit status */
		/* make zombie */
		thread->state = THREAD_ZOMBIE;
		notify_waiters(thread);
	}
}

static void
notify_waiters(mthread_t *thread)
{
	mthread_t *waiter;
	bool skip = false;

	/* notify all waiters */
	while((waiter = waitq_remove(&thread->waiters)) != NULL) {
		int i;

		assert(THREAD_WAITING(waiter));
		waiter->state = THREAD_OK;
		waiter->waiting = NULL;
		i = upcall_waitret(L4ID(waiter), TID(thread), STATUS(thread));
		assert(i == 0);
		skip = true;
	}

	if (THREAD_ISDETACHED(thread) || skip) { /* auto reap */
		thread->state = THREAD_DEAD;
		thread_release(thread);
	} else { /* add to parents zombie list */
                if (thread->parent != NULL && thread->parent != (mthread_t*)-1)
			waitq_add(&(thread->parent->zombies), thread);
		else { 
                        VERBOSE("threads.c: A thread without"
                                       " a parent has died.\n");
			//assert(!"INIT (or an upcall) has died");
		}
	}
}

int
thread_create(syscall_t *msg, apd_t *apd, threadinfo_t *info, mthread_t *parent)
{
	mthread_t *new;
        bool new_task, new_thread;
	void *stack, *supplied_stack = NULL;
	size_t stack_size = MTHREAD_STACK_SIZE;
        cap_t param;
        
	/* 
	 * parse thread info 
	 */
	if (info != NULL) {
                int iflags = info->flags;
                
		if (iflags & THREAD_STACK_ADDR) {
                        /* FIXME: validate RW access to stack */
                        supplied_stack = info->stack_addr;
		} else if (iflags & THREAD_STACK_SIZE) {
			stack_size = info->stack_size;
                        /* FIXME: stack size of zero should give invalid stack,
			 * but we need a stack to pass arguments on! */
			assert(stack_size != 0);
		}
		if (iflags & THREAD_MEM_LIMIT) {
			/* FIXME: implement me */
		}
		if (iflags & THREAD_CPU_TIME) {
			/* FIXME: implement me */
		}
		if (iflags & THREAD_DETACHED) {
			/* FIXME: implement me */
		}
		if (iflags & THREAD_BANK_ACCOUNT) {
                        /* FIXME implement me */;
                }
	}

	_lock_lock(&apd->lock);
	new = thread_get_mthread(apd, parent, &new_task, &new_thread);
	if (new == NULL) {
		_lock_unlock(&apd->lock);
		return fail;
 	}

        param.address = msg->syscall.data.thread.create.param;
	param.passwd = 0;
	stack = thread_setup_stack(new, supplied_stack, stack_size, apd,
			new_task, msg->syscall.data.thread.create.ip, &param);
	_lock_unlock(&apd->lock);

	/*
	 * Do thread creation
	 */
	_lock_lock(&l4_tasks.lock);
	l4task_thread_add(new);
	_lock_unlock(&l4_tasks.lock);

        msg->sysret.thread.create.l4id = L4_NIL_ID;
	if (new_task || parent == NULL) {
		if (start_task(&new->task->task, stack)) /*task create failed*/
			goto error;
                new->l4id.ID |= new->task->task.ID; /* update version info */
	} else {
	        assert(parent != NULL); /* last-chance! cgray */
		if (task_equal(new->task->task, parent->task->task)) {
			msg->sysret.thread.create.l4id = new->l4id;
			msg->sysret.thread.create.stack = stack;
                } else { /* non local thread create */
			if (upcall_create(new->l4id, stack) != 0){
				goto error;
			}
		}
	}

	msg->sysret.thread.create.tid = new->tid;
	return success;
error:
	_lock_lock(&l4_tasks.lock);
	l4task_thread_remove(new);
	_lock_unlock(&l4_tasks.lock);
	kfree(new);
        
	return fail;
}

/* Functions from syscall so PDX can block properly */
extern struct pending_request * syscall_return(struct pending_request *);
extern struct pending_request * syscall_wait(struct pending_request *);

int
thread_pdx_create(pdx_t proc, cap_t param, apd_t *apd, mthread_t *caller, 
		  struct pending_request *prq, 
		  struct pending_request ** new_prq)
{
	mthread_t *new;
	void *stack;
	bool new_task, new_thread;

	clock_stop(IN_THREAD_PDX_CREATE);
	_lock_lock(&apd->lock);

	clock_stop(PRE_GET_MTHREAD);
	new = thread_get_mthread(apd, caller, &new_task, &new_thread);
	clock_stop(POST_GET_MTHREAD);

	if (new == NULL) {
		_lock_unlock(&apd->lock);
		return fail;
	}
	new->PDX = caller;

	clock_stop(PRE_SETUP_STACK);
	stack = thread_setup_stack(new, NULL, MTHREAD_STACK_SIZE, apd,
					new_task, proc, &param);
	clock_stop(POST_SETUP_STACK);

	_lock_unlock(&apd->lock);

	/*
	 * Do thread creation
	 */
	clock_stop(PRE_THREAD_ADD);
	_lock_lock(&l4_tasks.lock);
	l4task_thread_add(new);
	_lock_unlock(&l4_tasks.lock);
	clock_stop(POST_THREAD_ADD);

	assert(prq != NULL); /* This may (?) happen with MAC enabled */

	if (new_task) {
		if (start_task(&new->task->task, stack)) /*task create failed*/
			goto error;
		new->l4id.ID |= new->task->task.ID; /* update version info */

		/* Wait for the next syscall */
		if (prq != NULL) {
			*new_prq = syscall_wait(prq);
		}
	} else {
		if (new_thread) {
			if (upcall_create(new->l4id, stack))
				goto error;

			/* wait for a new syscall */
			*new_prq = syscall_wait(prq);
		} else {
			/* Here we send directly to the thread instead */
			struct pending_syscall *call = &prq->data.syscall;
			call->sysmsg.upcall.data.create.l4id = new->l4id;
			call->sysmsg.upcall.data.create.stack = stack;
			call->sysmsg.upcall.type = UPCALL_CREATE;
			call->caller = new->l4id;

			clock_stop(PRE_PDX_IPC);
			*new_prq = syscall_return(prq);
			clock_stop(POST_CE_IPC);
		}
	}
	return success;

error:
	_lock_lock(&l4_tasks.lock);
	l4task_thread_remove(new);
	_lock_unlock(&l4_tasks.lock);
	kfree(new);
	return fail;
}


static l4_task_t *
task_new(void)
{
	l4_task_t *new;
	int i;

	new = (l4_task_t *)kmalloc(sizeof(l4_task_t));
	if (new == NULL)
		return (l4_task_t *)NULL;
        bzero(new, sizeof(l4_task_t));
	
	new->l4_threads[0] = (mthread_t *)kmalloc(sizeof(mthread_t));
	if (new->l4_threads[0] == NULL) {
		kfree(new);
		return NULL;
        }
        bzero(new->l4_threads[0], sizeof(mthread_t));

	new->l4_threads[0]->task = new;

	/* 
	 * insert task into task table 
	 */
	_lock_lock(&l4_tasks.lock);
	for (i = 0; i < l4_tasks.task_count; i++){
		if (l4_tasks.tasks[i] == NULL) {
			l4_tasks.tasks[i] = new;
			break;
		}
        }
	_lock_unlock(&l4_tasks.lock);

	if (i == l4_tasks.task_count) { /* ran out of tasks */
		kfree(new->l4_threads[0]);
		kfree(new);
		return (l4_task_t *)NULL;
	}

	new->task.id.task = i + l4_setup.l4_task_start;
	new->thread_free = (L4_THREADS_PER_TASK - 1);

	return new;
}
 
void
task_release(l4_task_t *task)
{
	int i;

	assert(task->thread_free == (L4_THREADS_PER_TASK - 1));
	i = task->task.id.task - l4_setup.l4_task_start;
	assert(l4_tasks.tasks[i] == task);
	_lock_lock(&l4_tasks.lock);
	l4_tasks.tasks[i] = NULL;
	_lock_unlock(&l4_tasks.lock);
	upcall_sleep(task->task, SLEEP_INFINITY);
	kfree(task);
}


bool
thread_sleep(mthreadid_t tid, time_t time, int *ret, mthread_t *caller)
{
	mthread_t *child;
	int result = success;
	bool caller_return = true;

	child = thread_find_mthread_from_parent(tid, caller);
	if (child == NULL) {
		result = fail;
	} else if (child == caller) {
		child->state = THREAD_SLEEP;
		caller_return = false;
		if (tid != THREAD_MYSELF)
			upcall_sleep(child->l4id, time);
	} else {
		upcall_sleep(child->l4id, time);
	}

	*ret = result;
	return caller_return;
}

void
thread_return(cap_t cap, mthread_t *thread)
{
	if (THREAD_IN_PDX(thread)) {
                l4task_thread_remove(thread);  /* CEG fix */
		thread_release(thread);
	} else {
		thread_delete(THREAD_MYSELF, (int)(long)cap.address, true, 
				thread);
	}
}

void
thread_die(mthread_t *thread)
{	
	/* delete thread */
	thread->lasterror = 0;	/* set exit status */
	/* make zombie */
	thread->state = THREAD_ZOMBIE;
	notify_waiters(thread);
}

/*
 * Initialise data structures, create kernel threads, start first user
 * thread
 */
void
thread_init(l4_threadid_t unused)
{
	syscall_t msg;
	apd_t *apd;
	cap_t cap;
	passwd_t pd_passwd, passwd;
        object_t *pd_obj;
        object_t *pdx_obj;
        object_t *usr_obj;
	int i;

        kprintf("mungi: thread_init\n");
	/* 
	 * Start kernel threads 
	 */
	_lock_init(&kthreadid_lock);
	mungi_kernel = kthreadid = l4_myself();

	default_pager = kthread_start(pager,NULL,NULL);
	exception_handler = kthread_start(exception,NULL,NULL);
	kthread_start(memory_cleanup,NULL,NULL);
        kprintf("mungi: mpager started\n");

	/* 
	 * create mungi threads hash table 
	 */
	_lock_init(&mungi_threads.lock);
	mungi_threads.threads = (mthread_t **)kmalloc(sizeof(mthread_t *)*
						      THREAD_HASHSIZE);
	assert(mungi_threads.threads != NULL);

	/* 
	 * creating lookup table 
	 */
	i = l4_setup.l4_task_end - l4_setup.l4_task_start + 1;
	l4_tasks.tasks = (l4_task_t **)kmalloc(sizeof(l4_task_t *)*i);
	assert(l4_tasks.tasks != NULL);
	l4_tasks.task_free = l4_tasks.task_count = i;
	_lock_init(&l4_tasks.lock);

	/*
	 * Create initial PD object
	 */
	pd_passwd = mrandom();
	pd_obj = object_create(sizeof(apddesc_t), NULL, pd_passwd);
	assert(pd_obj != NULL);
	memset(pd_obj->base, 0, sizeof(apddesc_t));
	
	/* make master PD object */
	pd_obj->info.special |= O_PD;
	pd_obj->info.cntrl_object.address = pd_obj->base;
	pd_obj->info.cntrl_object.passwd = pd_passwd;
        
	apd = apd_find_or_create(pd_obj->base);
	assert(apd != NULL);

	cap.address = pd_obj->base;
	cap.passwd = pd_passwd;
	clist_autoadd(apd, cap);

	/* 
	 * creating initial user thread 
	 */
	/* BEING AKI PATCH */

 	/* make an object / owner cap for the initial user */
 	cap.passwd = mrandom();
 	usr_obj = object_add(l4_setup.object_tbl.base, 
			     l4_setup.object_tbl.size, cap.passwd);
 	cap.address = usr_obj->base;
	clist_autoadd(apd, cap);

	/* FIXME: we are using the PDX stuff from AKI to pass a cap to
	 * init for mishell. This needs to be generalised (how??)
	 * For now, we've left it the same but pass the KIP instead of
	 * the entry point as the param. ceg
	 */
 	if (l4_setup.pdx.size) {
			VERBOSE("mungi: setting up pdx object\n");
			passwd = mrandom();
			pdx_obj = object_add(l4_setup.pdx.base, 
					     l4_setup.pdx.size, passwd);
			cap.address = pdx_obj->base;
			cap.passwd = passwd;
			clist_autoadd(apd, cap);
	}
	
	/* Setup an object for the KIP */
	/* FIXME: don't give them an owner cap! */
	VERBOSE("mungi: setting up KIP object\n");
	cap.address = l4_setup.kinfo;
	cap.passwd = mrandom();
	object_add(l4_setup.kinfo, PAGESIZE, 
			   cap.passwd);
	clist_autoadd(apd, cap);
	
	/* setup an object for the DIT */
	/* FIXME: don't give them an owner cap! */
	VERBOSE("mungi: setting up DIT object\n");
	cap.address = l4_setup.dit_start;
	cap.passwd = mrandom();
	object_add(l4_setup.dit_start, l4_setup.dit_len, 
			   cap.passwd);
	clist_autoadd(apd, cap);
	
	
	/* create the user-land thread */
  	msg.syscall.data.thread.create.ip = l4_setup.object_tbl.entry;
        msg.syscall.data.thread.create.param = l4_setup.kinfo;
  	kprintf("mungi: starting init\n");
	/* sanity check */
	assert(msg.syscall.data.thread.create.ip != NULL);
  	thread_create(&msg, apd, NULL, NULL);

	kprintf("mungi: init started from %p\n",&msg);	

        /* start the character listener */
	start_console_in(); 
}

/*
 * Start Mungi kernel threads
 */
l4_threadid_t
kthread_start(void *ip,l4_threadid_t *givenpager,l4_threadid_t *givenexcpt)
{
	void *new_stack;
	l4_threadid_t kthread, pager_id, excpt_id;
	uintptr_t dummy;

	_lock_lock(&kthreadid_lock);

	kthreadid.id.lthread++;
	kthread = kthreadid;

	_lock_unlock(&kthreadid_lock);

	new_stack = kmalloc(KTHREAD_STACK_SIZE);
	assert(new_stack != NULL);
	new_stack = (void *)((uintptr_t)new_stack + 
			(uintptr_t)KTHREAD_STACK_SIZE); 
        
        /* Find a pager & Exception handler */
        if (givenpager != NULL){
                pager_id = *givenpager;
                excpt_id = *givenexcpt;
        } else {
        	pager_id = excpt_id = L4_INVALID_ID;
        }

	l4_thread_ex_regs(kthread, (uintptr_t)ip, (uintptr_t)new_stack, 
			  &excpt_id, &pager_id,&dummy, &dummy);

	return kthread;
}
