/****************************************************************************
 *
 *      $Id: syscalls.c,v 1.31.2.2 2002/08/30 06:00:01 cgray Exp $
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

/* Mungi system calls */

#include "mungi/kernel.h"
#include "mungi/threads.h"
#include "mungi/objects.h"
#include "mungi/syscalls.h"
#include "mungi/semaphore.h"
#include "mungi/security.h"
#include "mungi/clist.h"
#include "mungi/syscallbits.h"
#include "mungi/pending_requests.h"
#include "mungi/pdxcache.h"
#include "status.h"

#include "clock.h"

extern int console_setreader(l4_threadid_t);

/*
 * variables used for syscall IPC
 */
l4_timeout_t syscall_timeout;
/* syscall jump table */
static struct pending_request * (*(msys_call[SYSCALLS_MAX]))(struct pending_request *);

/* thread/task of the pager */
extern l4_threadid_t startupid;

/*
 * system calls
 */
static struct pending_request * sys_thread_return(struct pending_request *);
static struct pending_request * sys_thread_create(struct pending_request *);
static struct pending_request * sys_thread_delete(struct pending_request *);
static struct pending_request * sys_thread_myid(struct pending_request *);
static struct pending_request * sys_thread_resume(struct pending_request *);
static struct pending_request * sys_thread_wait(struct pending_request *);
static struct pending_request * sys_thread_sleep(struct pending_request *);
static struct pending_request * sys_thread_info(struct pending_request *);
static struct pending_request * sys_object_create(struct pending_request *);
static struct pending_request * sys_object_info(struct pending_request *);
static struct pending_request * sys_object_pdx(struct pending_request *);
static struct pending_request * sys_object_resize(struct pending_request *);
static struct pending_request * sys_object_delete(struct pending_request *);
static struct pending_request * sys_object_passwd(struct pending_request *);
static struct pending_request * sys_pdx_call(struct pending_request *);
static struct pending_request * sys_apd_get(struct pending_request *);
static struct pending_request * sys_apd_insert(struct pending_request *);
static struct pending_request * sys_apd_delete(struct pending_request *);
static struct pending_request * sys_apd_lookup(struct pending_request *);
static struct pending_request * sys_apd_flush(struct pending_request *);
static struct pending_request * sys_apd_lock(struct pending_request *);
static struct pending_request * sys_excpt_reg(struct pending_request *);
static struct pending_request * sys_last_error(struct pending_request *);
static struct pending_request * sys_sem_create(struct pending_request *);
static struct pending_request * sys_sem_delete(struct pending_request *);
static struct pending_request * sys_sem_wait(struct pending_request *);
static struct pending_request * sys_sem_signal(struct pending_request *);
static struct pending_request * sys_page_copy(struct pending_request *);
static struct pending_request * sys_page_flush(struct pending_request *);

struct pending_request * syscall_return(struct pending_request *);
struct pending_request * syscall_wait(struct pending_request *);

static struct pending_request * sys_not_implemented(struct pending_request *);
static struct pending_request * sys_invalid(struct pending_request *);

static struct pending_request * sys_debug(struct pending_request *);

static struct pending_request *
sys_debug(struct pending_request *prq)
{
	static int reader_set = 0;
	struct pending_syscall *call = &prq->data.syscall;

	/* magic for setting a console reader (once of for dodgey
	   security) */
	if(call->sysmsg.msg.reg[0] == 0xfeedb17e) {
		if (reader_set == 0)
			reader_set = console_setreader(call->caller);
		else
			kprintf("syscall: Re-setting reader!!!\n");
	} else {
		/* kprintf("\e[34;1m**-> %s\e[m", (char *)&sysmsg.msg); */
		kprintf("%s", (char *)&call->sysmsg.msg);
	}

	return syscall_return(prq);
}

void
syscall_loop(void)
{
	int i;
	int syscall_number;
	struct pending_request *last_req;

	/*
	 * initialise syscall table
	 */
	for (i = 0; i < SYSCALLS_MAX; i++)
		msys_call[i] = sys_invalid;

	msys_call[0]	= sys_debug; /* FIXME */

	msys_call[SYS_THREAD_CREATE]	= sys_thread_create;
	msys_call[SYS_THREAD_INFO]	= sys_thread_info;
	msys_call[SYS_THREAD_DELETE]	= sys_thread_delete;
	msys_call[SYS_THREAD_SLEEP]	= sys_thread_sleep;
	msys_call[SYS_THREAD_RESUME]	= sys_thread_resume;
	msys_call[SYS_THREAD_WAIT]	= sys_thread_wait;
	msys_call[SYS_THREAD_MYID]	= sys_thread_myid;
	msys_call[SYS_THREAD_RETURN]	= sys_thread_return;
	msys_call[SYS_OBJECT_CREATE]	= sys_object_create;
	msys_call[SYS_OBJECT_DELETE]	= sys_object_delete;
	msys_call[SYS_OBJECT_RESIZE]	= sys_object_resize;
	msys_call[SYS_OBJECT_INFO]	= sys_object_info;
	msys_call[SYS_OBJECT_PDX]	= sys_object_pdx;
	msys_call[SYS_OBJECT_PASSWD]	= sys_object_passwd;
	msys_call[SYS_APD_INSERT]	= sys_apd_insert;
	msys_call[SYS_APD_DELETE]	= sys_apd_delete;
	msys_call[SYS_APD_GET]		= sys_apd_get;
	msys_call[SYS_APD_FLUSH]	= sys_apd_flush;
	msys_call[SYS_APD_LOCK]		= sys_apd_lock;
	msys_call[SYS_APD_LOOKUP]	= sys_apd_lookup;
	msys_call[SYS_EXCPT_REG]	= sys_excpt_reg;
	msys_call[SYS_LAST_ERROR]	= sys_last_error;
	msys_call[SYS_PDX_CALL]		= sys_pdx_call;
	msys_call[SYS_SEM_CREATE]	= sys_sem_create;
	msys_call[SYS_SEM_DELETE]	= sys_sem_delete;
	msys_call[SYS_SEM_WAIT]		= sys_sem_wait;
	msys_call[SYS_SEM_SIGNAL]	= sys_sem_signal;
	msys_call[SYS_OBJECT_PAGER]	= sys_not_implemented;
	msys_call[SYS_PAGE_COPY]	= sys_page_copy;
	msys_call[SYS_PAGE_MAP]		= sys_not_implemented;
	msys_call[SYS_PAGE_UNMAP]	= sys_not_implemented;
	msys_call[SYS_PAGE_FLUSH]	= sys_page_flush;

	last_req = syscall_wait(NULL);

	for(;;) {
		syscall_number = last_req->data.syscall.sysmsg.syscall.number
				 & (SYSCALLS_MAX - 1);
		last_req = msys_call[syscall_number](last_req);
	}
}

struct pending_request *
syscall_return(struct pending_request *prq)
{
        int r;
	l4_msgdope_t result;
	struct pending_syscall *call = &prq->data.syscall;

	assert(prq != NULL && prq->slot_used && prq->is_syscall);
	r = l4_ipc_reply_and_wait(call->caller, L4_IPC_SHORT_MSG,
			&call->sysmsg.msg, &call->caller, L4_IPC_SHORT_MSG,
			&call->sysmsg.msg, syscall_timeout, &result);

        if (r != 0){
		DEBUG("result %d %s error: %d", r, result.md.snd_error ? 
			"send" : "foo", result.md.error_code);
                return syscall_wait(prq);
        } else {
		call->thread = thread_find_from_l4id(call->caller);
		if (call->thread == NULL
				&& !task_equal(call->caller, startupid)) {
			DEBUG("Non existent thread %lx\n", call->caller.ID);
			return syscall_wait(prq);
		}
	}
	return prq;
}

struct pending_request *
syscall_wait(struct pending_request *prq)
{
        int r;
	l4_msgdope_t result;
	struct pending_syscall *call;

	if (prq == NULL) {
		prq = pending_requests_get_slot();
		prq->is_syscall = true;
	}

	assert(prq->slot_used && prq->is_syscall);
	call = &prq->data.syscall;

        do {
                r = l4_ipc_wait(&call->caller, L4_IPC_SHORT_MSG,
				&call->sysmsg.msg, L4_IPC_NEVER, &result);

                if (r != 0) {
                        DEBUG("syscall_wait: r is %x\nsyscall_wait: error "
				"code is %x\n", r, (int)result.md.error_code);
			call->thread = NULL;
                        continue;
                }
                call->thread = thread_find_from_l4id(call->caller);
        } while (call->thread == NULL);
	return prq;
}





static struct pending_request *
sys_thread_myid(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;
 	/* return callers Mungi ID */
	call->sysmsg.sysret.thread.tid = TID(call->thread);
	return syscall_return(prq);
}

static struct pending_request *
sys_thread_create(struct pending_request *prq)
{
        const apddesc_t * pd;
	apd_t * apd;
	threadinfo_t * info;
 	struct pending_syscall *call = &prq->data.syscall;

	/* check for threadinfo */ 
	info = (threadinfo_t *)call->sysmsg.syscall.data.thread.create.info;
	if (info != NULL) { /* validate object */
		object_t * obj;

		/* validate access to object */
		obj = validate_obj(info, APD(call->thread), M_READ, NULL, prq);
		if (obj == NULL)
			goto error;

		/* check info length */
		if (validate_range(obj, info, sizeof(*info)) == false)
			goto error;
	}

        /* determine what APD the new thread will join */
        pd = call->sysmsg.syscall.data.thread.create.pd;
	if (pd != NULL) {
		object_t * obj;

		/* verify READ access to PD pointer */
		obj = validate_obj(pd, APD(call->thread),
				   M_EXECUTE, NULL, prq); 
                if (obj == NULL) /* FIXME: should use IS_NOT_PDOBJECT() */
                        goto error;

		/* possibly make copy if need be */
		if ((info == NULL) || !(info->flags & THREAD_NO_JOINPD))
			apd = apd_find_or_create(pd);
		else
			apd = apd_from_template(pd);

		if (apd == NULL)
			goto error;
	} else {
		/* use callers APD */
		apd = APD(call->thread);
	}

	/* do the thread create */
	if (thread_create(&call->sysmsg, apd, info, call->thread))
		goto error;

	return syscall_return(prq);

error:
	call->sysmsg.sysret.thread.create.l4id = L4_NIL_ID;
	call->sysmsg.sysret.thread.create.tid = THREAD_NULL;
	return syscall_return(prq);
}

static struct pending_request *
sys_thread_delete(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	/* try to delete the thread */
	if (thread_delete(call->sysmsg.syscall.data.thread.delete.tid, 
			  call->sysmsg.syscall.data.thread.delete.status,
			  call->sysmsg.syscall.data.thread.delete.adopt,
			  call->thread)) {
		return syscall_return(prq);
	} else {
		return syscall_wait(prq); /* caller is the one being deleted */
	}
}

static struct pending_request *
sys_thread_resume(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	/* resume a sleeping thread */
	call->sysmsg.sysret.retval
		= thread_resume(call->sysmsg.syscall.data.thread.resume.tid,
				call->thread);
	return syscall_return(prq);
}

static struct pending_request *
sys_thread_wait(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	/* wait for said thread to exit */
	if (thread_wait(call->sysmsg.syscall.data.thread.wait.tid, 
			&call->sysmsg.sysret.thread.wait.status, 
			&call->sysmsg.sysret.thread.wait.tid, call->thread)) {
		return syscall_wait(prq);
	} else {
		return syscall_return(prq);
	}
}

static struct pending_request *
sys_thread_sleep(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	if (thread_sleep(call->sysmsg.syscall.data.thread.sleep.tid,
			 call->sysmsg.syscall.data.thread.sleep.time,
			 &call->sysmsg.sysret.retval, call->thread)) {
		return syscall_return(prq);
	} else {
		return syscall_wait(prq);
	}
}

static struct pending_request *
sys_thread_info(struct pending_request *prq)
{
	int retval = fail;
	object_t * obj;
	threadinfo_t * info;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate access to info buffer */
	info = call->sysmsg.syscall.data.thread.info.info;
	obj = validate_obj(info, APD(call->thread), M_READ|M_WRITE, NULL, prq);
	if (obj == NULL)  
 		goto out;
	else if (validate_range(obj, info, sizeof(*info)) == false)
		goto out;

	/* get thread info */
	retval = thread_info(call->sysmsg.syscall.data.thread.info.tid, info,
			     call->thread);

out:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_object_create(struct pending_request *prq)
{
	object_t * obj;
	objinfo_t * info;
	void * addr = NULL;
  	struct pending_syscall *call = &prq->data.syscall;

	/* validate access to the object information */
	info = (objinfo_t *)call->sysmsg.syscall.data.object.create.info;
	if (info != NULL) {
		obj = validate_obj(info, APD(call->thread), M_READ, NULL, prq);
		if (obj == NULL) 
			goto out;
		else if (validate_range(obj, info, sizeof(*info)) == false)
			goto out;
	}

	/* create the object */ /* FIXME add accounting parameter */
	obj = object_create(call->sysmsg.syscall.data.object.create.size, info,
			    call->sysmsg.syscall.data.object.create.passwd);
	if (obj == NULL)
		goto out;
	addr = obj->base;

        /* put the userinfo and acctinfo values passed by the user into 
	 * the object's info. Also copy the passed security_type into 
	 * the info -- if no security_type was provided then use the 
	 * default type from the current thread's APD -- AE */
        if (info != NULL) {
                obj->info.userinfo = info->userinfo;
                obj->info.acctinfo = info->acctinfo;
        } else {
                obj->info.userinfo = 0;
                obj->info.acctinfo = 0;
        }

out:	call->sysmsg.sysret.object.addr = addr;
	return syscall_return(prq);
}


/* This allows mcs to work (until mcs is fixed) */
#define INSECURE_OBJINFO 1

#if !defined(INSECURE_OBJINFO) || (INSECURE_OBJINFO == 0)
static access_t 
create_access_reqs( int flags ) 
{
        access_t access = 0;

        if (flags & O_SET_MODIFY)
                access |= M_OWNER;
        if (flags & O_SET_ACCESS)
                access |= M_OWNER;
        if (flags & O_SET_ACCNT)
                access |= M_OWNER; /* FIXME: The API says this should be R-OT */
        if (flags & O_TCH_MODIFY)
                access |= M_WRITE;
        if (flags&O_TCH_ACCESS)
                access |= 0; /* FIXME: R or X is a pain to handle! */
        if (flags&O_TCH_ACCNT)
                access |= M_OWNER; /* FIXME: The API says this should be R-OT */
	if (flags & O_SET_TYPE)
		access |= M_OWNER;
        return access;
}
#endif

static struct pending_request *
sys_object_info(struct pending_request *prq)
{
	object_t * obj, * info_obj;
	int ret = ST_PROT; /* to match API */
	int flags;
	objinfo_t * info;
	void * addr;
 	struct pending_syscall *call = &prq->data.syscall;
#ifndef INSECURE_OBJINFO
	access_t access;
#endif

	/* syscall parameters */
	flags = call->sysmsg.syscall.data.object.info.flags;
	info = call->sysmsg.syscall.data.object.info.info;
	addr = (void *)call->sysmsg.syscall.data.object.info.obj;

#ifdef INSECURE_OBJINFO
        obj = object_find(addr);
#else
	/* validate access to object */
	access = M_READ | create_access_reqs(flags);
	if (flags & O_TCH_ACCESS)
                access |= M_READ;
	obj = validate_obj(addr, APD(call->thread), access, NULL, prq);

	/* if it failed and we have O_TCH_ACCESS have to redo with execute */
	if ((obj == NULL) && (flags & O_TCH_ACCESS)){
	        access = M_READ | create_access_reqs(flags);
                access |= M_EXECUTE;
	        obj = validate_obj(addr, APD(call->thread), access, NULL, prq);
	}
#endif

	if (obj == NULL)
		goto out;

	/* validate access to information storage location */
	info_obj = validate_obj(info, APD(call->thread), (M_READ|M_WRITE),
				NULL, prq);
	if (info_obj == NULL)
		goto out;

	/* make sure information object is not length challenged */
	if (validate_range(info_obj, info, sizeof(*info)) == false)
		goto out;

	/* finally set object information */
	ret = object_info(obj, info, flags);

out:	call->sysmsg.sysret.retval = ret;
	return syscall_return(prq);
}

static struct pending_request *
sys_object_delete(struct pending_request *prq)
{
	object_t *obj;
	int retval = ST_PROT;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate DESTROY access to object */
	obj = validate_obj(call->sysmsg.syscall.data.object.delete,
			   APD(call->thread), M_DESTROY, NULL, prq);
	if (obj == NULL) 
		goto out;

	/* attempt to destroy object */
	retval = object_delete(obj->base);

out:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_object_resize(struct pending_request *prq)
{
	object_t * obj;
	int ret = fail;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate OWNER access to object */
	obj = validate_obj(call->sysmsg.syscall.data.object.delete,
			   APD(call->thread), M_OWNER, NULL, prq);
	if (obj == NULL)
		goto out;

	/* resize object */
	ret = object_resize(obj,
			    call->sysmsg.syscall.data.object.resize.new_size);

out:	call->sysmsg.sysret.retval = ret;
	return syscall_return(prq);
}

static struct pending_request *
sys_object_pdx(struct pending_request *prq)
{
	object_t * obj;
	cap_t * clist, clist_cap;
	uint n_entrypt;
	pdx_t * pdx;
	int retval = ST_PROT;
 	struct pending_syscall *call = &prq->data.syscall;

	/* convenience variables */
	n_entrypt = call->sysmsg.syscall.data.object.pdx.n_entrypt;
	pdx = call->sysmsg.syscall.data.object.pdx.entry_pnts;
	clist_cap.address = (void *)call->sysmsg.syscall.data.object.pdx.clist;

	/* validate READ access to entry points list */
	obj = validate_obj(pdx, APD(call->thread), M_READ, NULL, prq);
	if (obj == NULL)
		goto error;

	/* validate size of entry points */
	if ((n_entrypt > O_MAX_ENTPT) ||
	    (validate_range(obj, pdx, sizeof(pdx_t)*n_entrypt)) == false)
		goto error;

	/* validate READ access to clist */
	obj = validate_obj(clist_cap.address, APD(call->thread), M_READ, &clist,
			   prq);
	if (obj == NULL)
		goto error;

	clist_cap.passwd = clist->passwd; /* password of clist stored in pdx */

	/* FIXME find the size of the clist header
	if (validate_range(obj, clist_cap.address, ))
		goto error;
		*/

	/* is valid clist? */
	if (IS_NOT_CLIST((clist_t *)clist_cap.address)) {
		retval = ST_CLIST;
		goto error;
	}

	/* validate OWNER access to object */
	obj = validate_obj(call->sysmsg.syscall.data.object.pdx.cap.address, 
			   APD(call->thread), M_OWNER, NULL, prq);
	if (obj == NULL)
		goto error;

	/* finally attempt to create the PDX object */
	retval = object_pdx(obj,
			call->sysmsg.syscall.data.object.pdx.cap.passwd,
			clist_cap, n_entrypt, pdx);

error:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_object_passwd(struct pending_request *prq)
{
	object_t * obj;
	int retval = fail;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate OWNER access to object */
	obj = validate_obj(call->sysmsg.syscall.data.object.passwd.cap.address, 
			   APD(call->thread), M_OWNER, NULL, prq);
	if (obj != NULL) {
		/* add/change a password */
		retval = object_passwd(obj, 
			    call->sysmsg.syscall.data.object.passwd.cap.passwd,
			    call->sysmsg.syscall.data.object.passwd.mode);
	}

	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_apd_get(struct pending_request *prq)
{
	int retval = fail;
	object_t * obj;
        bool vrange;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate access to the buffer */
	obj = validate_obj(call->sysmsg.syscall.data.apd.get.buffer,
			   APD(call->thread), M_WRITE, NULL, prq);
	if (obj == NULL)
		goto out;

	/* check the size of the object */
        vrange = validate_range(obj, call->sysmsg.syscall.data.apd.get.buffer, 
		                sizeof(apddesc_t));
                
        if (vrange == false)
                goto out;
        

	/* copy callers APD */
	apd_get(APD(call->thread), call->sysmsg.syscall.data.apd.get.buffer);
	retval = success;

out:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_apd_insert(struct pending_request *prq)
{
	object_t *obj;
	cap_t *cap;
	int retval = fail;
 	struct pending_syscall *call = &prq->data.syscall;

	/* Validate access to clist FIXME what permisions here? */
	obj = validate_obj(call->sysmsg.syscall.data.apd.insert.clist,
			   APD(call->thread), M_EXECUTE, &cap, prq);
	if (obj == NULL)
		goto out;

	/* FIXME: validate that it is a clist & size of obj */

	/* try to insert clist into APD */
	retval = apd_slot_insert(APD(call->thread),
				 call->sysmsg.syscall.data.apd.insert.clist,
				 cap->passwd,
				 call->sysmsg.syscall.data.apd.insert.pos);

out:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_apd_delete(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	/* attempt to delete slot */
	call->sysmsg.sysret.retval = apd_slot_delete(APD(call->thread),
				call->sysmsg.syscall.data.apd.delete.pos);
	return syscall_return(prq);
}

static struct pending_request *
sys_apd_flush(struct pending_request *prq)
{
	/* flush callers APD. This system call is useless. */
        /* No, it's not -- GH */
	apd_flush(APD(prq->data.syscall.thread));
	return syscall_return(prq);
}

static struct pending_request *
sys_apd_lock(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	/* lock APD slots */
	call->sysmsg.sysret.retval = apd_slot_lock(APD(call->thread), 
					call->sysmsg.syscall.data.apd.lock.pos);
	return syscall_return(prq);
}

static struct pending_request *
sys_apd_lookup(struct pending_request *prq)
{
	object_t * obj;
	cap_t * cap = NULL;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate given access */
	obj = validate_obj(call->sysmsg.syscall.data.apd.lookup.address,
			   APD(call->thread),
			   call->sysmsg.syscall.data.apd.lookup.minrights,
			   &cap, prq);
	if (obj == NULL) {
	        cap = NULL;	/* according to API spec */
		goto out;
	}

out:	call->sysmsg.sysret.apd.addr = cap;
	return syscall_return(prq);
}

static struct pending_request *
sys_excpt_reg(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	/* register new exception handler */
	call->sysmsg.sysret.excpt.handler = excp_reg(EXCPT(call->thread),
				call->sysmsg.syscall.data.excpt.reg.exception,
				call->sysmsg.syscall.data.excpt.reg.handler);
	return syscall_return(prq);
}

static struct pending_request *
sys_last_error(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;

	/* get callers last error value */ 
	call->sysmsg.sysret.retval = ERROR(call->thread);
	return syscall_return(prq);
}

static struct pending_request *
sys_sem_create(struct pending_request *prq)
{
	object_t * obj;
	void * addr;
	int retval = fail;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate READ & WRITE access to object */
	addr = call->sysmsg.syscall.data.sem.create.address;
	obj = validate_obj(addr, APD(call->thread),
			   (M_READ|M_WRITE), NULL, prq);
	if (obj == NULL)
		goto out;

	/* create semaphore */
	retval = sem_create(obj, addr,
			    call->sysmsg.syscall.data.sem.create.value);

out:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_sem_delete(struct pending_request *prq)
{
	object_t * obj;
	void * addr;
	int retval = fail;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate OWNER access to object */
	addr = call->sysmsg.syscall.data.sem.delete.address;
	obj = validate_obj(addr, APD(call->thread), M_OWNER, NULL, prq);
	if (obj == NULL)
		goto out;

	 /* attempt to delete semaphore */
	retval = sem_delete(obj, addr);

out:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_sem_wait(struct pending_request *prq)
{
	object_t * obj;
	void * addr;
	int retval = fail;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate READ access to object */
	addr = call->sysmsg.syscall.data.sem.wait.address;
	obj = validate_obj(addr, APD(call->thread), M_READ, NULL, prq);
	if (obj == NULL)
		goto out;

	/* wait on semaphore */
	if (sem_wait(obj, addr, call->thread, &retval)) {
                return syscall_wait(prq);
        } else {
out:	        call->sysmsg.sysret.retval = retval;
	        return syscall_return(prq);
        }
}

static struct pending_request *
sys_sem_signal(struct pending_request *prq)
{
	object_t * obj;
	void * addr;
	int retval = fail;
 	struct pending_syscall *call = &prq->data.syscall;

	/* validate READ access to object */
	addr = call->sysmsg.syscall.data.sem.signal.address;
	obj = validate_obj(addr, APD(call->thread), M_READ, NULL, prq);
	if (obj == NULL)
		goto out;

	/* attempt to signal semaphore */
	retval = sem_signal(obj, addr);

out:	call->sysmsg.sysret.retval = retval;
	return syscall_return(prq);
}

static struct pending_request *
sys_pdx_call(struct pending_request *prq)
{
	object_t * obj;
	apd_t * apd = NULL;
	const void * pd;
	cap_t clist;
	cap_t * cap;
	pdx_t proc;
 	struct pending_syscall *call = &prq->data.syscall;
	struct apd_cache_id pdxc;

	proc = call->sysmsg.syscall.data.pdx.call.proc;
	pd = call->sysmsg.syscall.data.pdx.call.pd;

	clock_stop(POST_CR_IPC);

	/* check the PDX cache */
	apd = pdxcache_lookup( APD(call->thread), proc, pd );

	clock_stop(POST_PDX_CACHE_LOOKUP);

	/* not in the PDX cache */
	if( apd == NULL ) {
		/* validate PDX cap to proc */

		obj = validate_obj_pdx(proc, APD(call->thread), M_PDX, &cap,
				       prq);
		if (obj == NULL)
			goto error;
		
		/* get object PDX clist */
		clist = object_pdx_get_clist(obj, proc, cap->passwd);
		if (clist.address == NULL)
			goto error;
		
		/* get APD for PDX execution */
		if (pd == PD_MERGE) {
			apd = apd_pdx_merge(APD(call->thread), clist, &pdxc);
		} else if (pd == PD_EMPTY) {
			apd = apd_pdx_merge(NULL, clist, &pdxc);
		} else {
			/* validate access to PD object */
			obj = validate_obj(pd, APD(call->thread), M_EXECUTE, 
					   NULL, prq);

			/* FIXME: should use IS_NOT_PDOBJECT() 
			 *   However there is no way to set PDOBJECT flag */
			if (obj == NULL) 
				goto error;
			
			apd = apd_pdx_find_or_create(pd, clist, &pdxc);
		}
		if (apd == NULL)
			goto error;
		
		pdxcache_add( APD(call->thread), proc, pd, &pdxc );
	}

	clock_stop(PRE_PDX_THREAD_CREATE);

	/* do the actual PDX call */
	if (thread_pdx_create(proc, call->sysmsg.syscall.data.pdx.call.param,
			      apd, call->thread, prq, &prq) == success) {
		return prq;
	} else {
error:		call->sysmsg.sysret.retval = fail;
		return syscall_return(prq);
	}
}

static struct pending_request *
sys_thread_return(struct pending_request *prq)
{
 	struct pending_syscall *call = &prq->data.syscall;
	mthread_t *thread = call->thread;
	cap_t retcap = call->sysmsg.syscall.data.thread.retcap;

	if (THREAD_IN_PDX(thread)) {
		clock_stop(PRE_PDX_IPC);
		thread_return(retcap, thread);
		call->sysmsg.sysret.retval = success;
		call->sysmsg.sysret.pdx.cap = retcap;
		call->caller = L4ID(thread->PDX);
		clock_stop(PRE_RET_IPC);
		return syscall_return(prq);
	} else {
		thread_return(retcap, thread);
		return syscall_wait(prq);
	}
}

/* AE - syscall not implemented before */
/* FIXME: Should be done by CoW - GH */
static struct pending_request *
sys_page_copy(struct pending_request *prq)
{
       void *src, *dest, *srclimit, *destlimit;
       uint n_pages;
       object_t *objsrc, *objdest;
       int retval = ST_PROT, i;
 	struct pending_syscall *call = &prq->data.syscall;

       /* 1. get the parameters */
       src = (void *)call->sysmsg.syscall.data.page.copy.from;
       dest = call->sysmsg.syscall.data.page.copy.to;
       n_pages = call->sysmsg.syscall.data.page.copy.n_pages;

       /* 2. validate the start of the litte suckers */
       objsrc = validate_obj(src, APD(call->thread), M_READ, NULL, prq);
       if (objsrc == NULL)
               goto out;

       objdest = validate_obj(dest, APD(call->thread), M_WRITE, NULL, prq);
       if (objdest == NULL)
               goto out;

       /* 3. Make sure the regions given are within a single object */
       srclimit = (void*)((long long)src + n_pages*L4_PAGESIZE);
       destlimit = (void*)((long long)dest + n_pages*L4_PAGESIZE);

       if( srclimit > objsrc->end || destlimit > objdest->end ) {
               retval = ST_RNG;
               goto out;
       }

       /* 4. Do the actual copy - always a multiple of the pagesize */
       for( i = 0; i < ((n_pages*L4_PAGESIZE)/sizeof(long long)); i++ )
               ((long long*)dest)[i] = ((long long*)src)[i];
       retval = ST_SUCC;

       /* 5. return from the syscall */
out:
       call->sysmsg.sysret.retval = retval;
       return syscall_return(prq);
}

/* AE - syscall not implemented before */
/* there is no stable storage device, so flushing doesn't need to do anything */
static struct pending_request *
sys_page_flush(struct pending_request *prq)
{
       void *ptr, *ptrlimit;
       uint n_pages;
       object_t *obj;
       int retval;
	struct pending_syscall *call = &prq->data.syscall;

       /* get params */
       ptr = (void *)call->sysmsg.syscall.data.page.flush.page;
       n_pages = call->sysmsg.syscall.data.page.flush.n_pages;

       kprintf( "PAGEFLUSH:\tptr = 0x%p.\n", ptr );

       /* validate the object - M_READ|M_WRITE access */
       obj = validate_obj(ptr, APD(call->thread), (M_READ|M_WRITE), NULL, prq);
       if (obj == NULL) {
               retval = ST_PROT;
               goto out;
       }

       /* make sure range is ok */
       ptrlimit = (void*)((long long)ptr + n_pages*L4_PAGESIZE);
       if( ptrlimit > obj->base ) {
               retval = ST_RNG;
               goto out;
       }
       retval = ST_SUCC;
out:
       call->sysmsg.sysret.retval = retval;
       return syscall_return(prq);
}

static struct pending_request *
sys_not_implemented(struct pending_request *prq)
{
	prq->data.syscall.sysmsg.sysret.retval = ST_NOIMP; /* Not implemented */
	return syscall_return(prq);
}

static struct pending_request *
sys_invalid(struct pending_request *prq)
{
	DEBUG("Invalid system call: %d\n",
	      (int)prq->data.syscall.sysmsg.syscall.number);
	return syscall_wait(prq);
}
