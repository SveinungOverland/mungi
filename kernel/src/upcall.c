/****************************************************************************
 *
 *      $Id: upcall.c,v 1.10.2.1 2002/08/30 06:00:02 cgray Exp $
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

/* upcall handling */

#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallbits.h"
#include "mungi/upcall.h"

l4_timeout_t upcall_timeout;

static int do_ipc(l4_threadid_t, l4_ipc_reg_msg_t *);


int
upcall_sleep(l4_threadid_t l4id, time_t time)
{
	upcall_t msg;

	msg.upcall.type = UPCALL_SLEEP;
	msg.upcall.data.sleep.l4id = l4id;
	msg.upcall.data.sleep.time = time;
	l4id.id.lthread = 0;

	return do_ipc(l4id, &msg.msg);
}

int
upcall_resume(l4_threadid_t l4id)
{
	upcall_t msg;

	msg.upcall.data.resume.l4id = l4id;
	msg.upcall.type = UPCALL_RESUME;
	l4id.id.lthread = 0;

	return do_ipc(l4id, &msg.msg);
}

int
upcall_create(l4_threadid_t l4id, void *stack)
{
	upcall_t msg;

	msg.upcall.data.create.l4id = l4id;
	msg.upcall.data.create.stack = stack;
	msg.upcall.type = UPCALL_CREATE;
	l4id.id.lthread = 0;

	return do_ipc(l4id, &msg.msg);
}

int
upcall_send(l4_threadid_t l4id, int retval)
{
	syscall_t msg;

	msg.sysret.retval = retval;

	return do_ipc(l4id, &msg.msg);
}

int
upcall_waitret(l4_threadid_t l4id, mthreadid_t tid, int status)
{
	syscall_t msg;

	msg.sysret.thread.wait.status = status;
	msg.sysret.thread.wait.tid = tid;

	return do_ipc(l4id, &msg.msg);
}

int
upcall_excp(l4_threadid_t l4id, void *handler, void *data, excpt_t exception)
{
	upcall_t msg;

	msg.upcall.data.excpt.handler = handler;
	msg.upcall.data.excpt.data = data;
	msg.upcall.data.excpt.l4id = l4id;
	msg.upcall.data.excpt.exception = exception;
	msg.upcall.type = UPCALL_EXCPT;
	l4id.id.lthread = 0;

	return do_ipc(l4id, &msg.msg);
}

static int
do_ipc(l4_threadid_t l4id, l4_ipc_reg_msg_t *msg)
{
	l4_msgdope_t result;
	int r;

	r = l4_ipc_send(l4id, L4_IPC_SHORT_MSG, msg, upcall_timeout, &result);
	return r;
}
