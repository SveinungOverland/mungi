/****************************************************************************
 *
 *      $Id: exception.c,v 1.5 2002/05/31 06:27:39 danielp Exp $
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

/* exception processing */

#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/exception.h"


void
exception(void)
{
	l4_threadid_t faulter;
	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
	excpthndlr_t handler;
	mthread_t *thread;
	void *fault_ip;
	upcall_t upcall;
	int r;

	VERBOSE( "EXCPT:\tlocal_stack: 0x%p.\n", (void*)&r );

	for(;;) {
		r = l4_ipc_wait(&faulter, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
				&result);
		if (r) continue;
got_fault:

		/* find faulting thread */
		thread = thread_find_from_l4id(faulter);
		if (thread == NULL)
			continue; /* illegitimite thread */

		fault_ip = (void *)msg.reg[1];
		VERBOSE("Got an exception from %p\n", (void*)(faulter.ID));
		VERBOSE("EPC %p\nBVA %p\n", (void*)fault_ip, (void*)(msg.reg[2]));

		/*
		 * signal exception
		 */
		handler = EXCPT(thread)[E_ILL];	/* FIXME proper exception # */
		if (handler) {

			exception_msg(&upcall, handler, faulter, E_ILL, 
					fault_ip);
			faulter.id.lthread = 0;
			r = l4_ipc_reply_and_wait(faulter, L4_IPC_SHORT_MSG, 
					&upcall.msg, &faulter, L4_IPC_SHORT_MSG
					, &msg, L4_IPC_NEVER, &result);
			if (r == 0) goto got_fault;			
		} else {
			thread_die(thread);
			continue;
		}
	}
}

void
exception_msg(upcall_t *msg, excpthndlr_t handler, l4_threadid_t l4id, 
		excpt_t exp, void *data)
{
	msg->upcall.type = UPCALL_EXCPT;
	msg->upcall.data.excpt.handler = handler;
	msg->upcall.data.excpt.data = data;
	msg->upcall.data.excpt.exception = exp;
	msg->upcall.data.excpt.l4id = l4id;
}

excpthndlr_t
excp_reg(excpthndlr_t table[], excpt_t exp, excpthndlr_t handler)
{
	excpthndlr_t old;

	if (exp < EXP_MAX) {
		old = table[exp];
		table[exp] = handler;
	} else 
		old = NULL;

	return old;
}
