/****************************************************************************
 *
 *      $Id: pager.c,v 1.35.2.1 2002/08/29 04:32:02 cgray Exp $
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

/* Mungi thread default pager */

#undef _VERBOSE_ /* turn off debugging */

#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/timing.h"
#include "mungi/threads.h"
#include "mungi/mm.h"
#include "mungi/security.h"
#include "mungi/pager.h"
#include "mungi/pending_requests.h"

#ifdef ALPHAENV
#include <l4/dit.h>
#else
#include <kernel/dit.h>
#endif

/* this should probably moved somewhere else {nash} */
#include "vm/vm_types.h"
#include "vm/ptable.h"

l4_timeout_t pagefault_timeout;

page_table_t *ptable, *ctable;
frame_table_t *ftable;
unsigned int ctable_size, ptable_size;
extern Dit_Dhdr *dit_hdr;

extern l4_threadid_t mungi_kernel;
extern l4_threadid_t kthread_start(void *ip,l4_threadid_t *,l4_threadid_t *);
void do_touch( int *pvar, int var );
static struct pending_request * get_fault(struct pending_request *);

/* Thread Ids */
l4_threadid_t mpagerid;
l4_threadid_t rpagerid;
l4_threadid_t apagerid;

void 
apager(void){
	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
	int r;
	uintptr_t addr;

	VERBOSE("apager: I'm alive - %llx\n", (long long) l4_myself().ID);

	while (1){
		VERBOSE("apager: waiting for rpager...\n");
		r = 0;
		do {
			/*if (r){
				VERBOSE("\e[35mapager: tick!\e[0m\n");
				l4_thread_switch(L4_NIL_ID);
			}*/
			r = l4_ipc_receive(rpagerid, L4_IPC_MAPMSG(0,
					L4_WHOLE_ADDRESS_SPACE),&msg,
					/*L4_IPC_TIMEOUT(0, 1, 0, 1, 15, 15),*/
                                        L4_IPC_NEVER,
					&result);

		} while (r);

		assert(r == 0);

		VERBOSE("apager: reg0 is (%llx)\n",
			        (long long) msg.reg[0]);
		VERBOSE("apager: reg1 is (%llx)\n",
			        (long long) msg.reg[1]);

		addr = msg.reg[0];
		VERBOSE("apager: sending to %llx\n",
			(long long) msg.reg[2]);


		msg.reg[4] = 0;
		msg.reg[1] = (uintptr_t) l4_fpage(addr, L4_LOG2_PAGESIZE,
					 (msg.reg[1] & L4_FPAGE_RW_MASK)
					 ? L4_FPAGE_RW : L4_FPAGE_RO,
					 L4_FPAGE_MAP).fpage;

		/* pretend to be mpager! */
		r = l4_ipc_send_deceiting((l4_threadid_t) msg.reg[2],
					  mpagerid,
					  L4_IPC_SHORT_FPAGE,
					  &msg, L4_IPC_NEVER, &result);

		assert(r == 0);
		VERBOSE("apager: IPC sent OK!\n");
	}

}


void 
pager(void){
	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
        l4_threadid_t startupid;
        struct pending_request *prq = NULL;

	mpagerid = l4_myself();
	VERBOSE("mpager: I'm alive - 0x%lx\n",mpagerid.ID);

	l4_ipc_receive(mungi_kernel, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
		       &result);
	VERBOSE("pager: received data structure message from mungi server");

	ctable = (page_table_t *) msg.reg[1];
	ctable_size = (unsigned int) msg.reg[2];
	ftable = (frame_table_t *) msg.reg[3];
	ptable = (page_table_t *) msg.reg[4];
	ptable_size = (unsigned int) msg.reg[5];

	VERBOSE("mpager: I'm 0x%016lx\n", l4_myself().ID);
	VERBOSE("mpager: I'm being paged by 0x%016lx\n", rpagerid.ID);
        startupid = rpagerid;
        startupid.id.lthread ++;

        VERBOSE("mpager: starting apager\n");
        apagerid = kthread_start(apager,&rpagerid,&startupid);
        
	for (;;) {
                prq = get_fault(prq);
		prq = handle_fault(prq);
	}
}

static struct pending_request *
get_fault(struct pending_request *prq)
{
	struct pending_pagefault *fault;
	l4_msgdope_t result;
	int r;

	if (prq == NULL) {
		prq = pending_requests_get_slot();
		prq->is_syscall = false;
	}
	assert(prq != NULL && prq->slot_used && !prq->is_syscall);
	fault = &prq->data.pagefault;

	/* Wait for a fault */
	r = l4_ipc_wait(&fault->faulter,
			L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
			&fault->msg, L4_IPC_NEVER, &result);
	assert(r == 0);

        VERBOSE("mpager: fault occured\n");
        VERBOSE("mpager: Thread ID is 0x%lx\n", fault->faulter.ID);
	VERBOSE("mpager: (bva) msg.reg[0] = 0x%lx\n", fault->msg.reg[0]);
	VERBOSE("mpager: (ip)  msg.reg[1] = 0x%lx\n", fault->msg.reg[1]);

        /* Get the Mungi thread ID */
	fault->thread = thread_find_from_l4id(fault->faulter);

        /* Sanity */
        if (fault->thread == NULL) {
		kprintf("mpager: ignoring fault from unknown thread\n");
		kprintf("mpager: faulter ID is 0x%lx\n", fault->faulter.ID);
		return get_fault(prq);	/* illegitimite thread */        
        }

	return prq;
}

struct pending_request *
handle_fault(struct pending_request *prq)
{
	l4_msgdope_t result;
	page_table_t *pte;
	void *fault_addr, *fault_ip;
	bool write_fault;
	access_t access;
	object_t *obj;
	int r;

	/* convenience variables */
	l4_ipc_reg_msg_t *msg = &prq->data.pagefault.msg;
	l4_threadid_t faulter = prq->data.pagefault.faulter;
	mthread_t *thread = prq->data.pagefault.thread;

        /* Extract everything from the IPC */
	fault_addr = (void *) (msg->reg[0] & ~(uintptr_t) 3);
	write_fault = (bool) (msg->reg[0] & 2);
	fault_ip = (void *) msg->reg[1];
	access = write_fault ? M_WRITE :
			((fault_addr == fault_ip) ? M_EXECUTE : M_READ);

        if ((uintptr_t)fault_addr < L4_PAGESIZE) {
		kprintf("mpager: NULL POINTER DEREFERENCE!\n");
		kprintf("mpager: Thread ID is 0x%lx\n", faulter.ID);
		kprintf("mpager: msg.reg[0] = 0x%lx\n", msg->reg[0]);
		kprintf("mpager: msg.reg[1] = 0x%lx\n", msg->reg[1]);
		return prq;
	}


#if defined(MIPSENV) && defined(CLOCK)
	/* device mappings to userspace (REMOVEME!) */
	if (msg->reg[0] == SIGMA0_DEV_MAP) {
		VERBOSE("mpager: device mapping, passing to rpager\n");
		r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, msg,
				L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE), msg,
 				L4_IPC_NEVER, &result);
		assert(r == 0 && result.md.fpage_received);
		do_map(faulter, (void *)msg->reg[1], (M_READ | M_WRITE));
		return prq;
	}
#endif


        VERBOSE("mpager: validating\n");
        /* Is it a valid access? */
        obj = validate_obj(fault_addr, APD(thread), access, NULL, prq);
        if (obj == NULL) {
                /* Not found - illegal access */
                protection_exception(thread, faulter, fault_ip, fault_addr);
                return prq;
        } else {
                VERBOSE("mpager: validated OK\n");
        }
        /* Find it in the page table */
        pte = find_pte((uintptr_t) fault_addr >> L4_LOG2_PAGESIZE);
	if (pte == NULL) {
                VERBOSE("mpager: talking to rpager about it\n");

                if (msg->reg[0] == MPAGER_MAGIC && msg->reg[1] == MPAGER_MAGIC)
                        assert(!"mpager: User thread faulted on MPAGER_MAGIC");

                /* Ask the rpager to do something about it */

                /* Save fault registers */
                msg->reg[5] = msg->reg[0];
                msg->reg[6] = msg->reg[1];

                /* Send page mappings */
                msg->reg[0] = MPAGER_MAGIC; 
                msg->reg[1] = MPAGER_MAGIC; 
		msg->reg[2] = (uintptr_t) apagerid.ID;
		msg->reg[3] = (uintptr_t) faulter.ID;
		msg->reg[4] = access;

                /* Send to rpager - get it to reply to apager */
                r = l4_ipc_send(rpagerid, L4_IPC_SHORT_FPAGE, msg,
                                L4_IPC_NEVER, &result);
        } else {
                /* In the Page table */

                /* just touch to be on the safe side */
                /* Hmmm... gcc 2.8.1 seems a wee bit broken */
                int dummy;
                if (access == M_WRITE)
                        do_touch(fault_addr, *(int *)fault_addr);
                else 
                        do_touch(&dummy,*(int *)fault_addr);

                do_map(faulter, fault_addr, access);
 	}
	return prq;
}

/*
 * protection_exception
 *
 * Throws a protection exception for the specified thread.
 * Note this could be optimised to use reply and wait IPC, but since it
 * is an exception the performance is not considered critical
 */
void
protection_exception(mthread_t *thread,l4_threadid_t faulter,void *ip,
                void *addr){
	excpthndlr_t handler;
	upcall_t upcall;
        l4_msgdope_t result;

        VERBOSE("mpager: Exception\n");
        
        /* Some sanity checking */
        if (faulter.ID == L4_NIL_ID.ID)
                return;
        if (thread == NULL){
		thread = thread_find_from_l4id(faulter);
        }

        /* Tell the world */ 
        kprintf("Throwing ST_PROT exception for %lx for \n",faulter.ID);
        kprintf("\tfault_addr %p, from ip %p\n", addr,ip);

        /* Find the handler */
	handler = EXCPT(thread)[E_PROT];
        if (handler) {
		exception_msg(&upcall, handler, faulter,
					      E_PROT, addr);
		faulter.id.lthread = 0;
                l4_ipc_send(faulter,L4_IPC_SHORT_MSG,&upcall.msg,L4_IPC_NEVER,
                                &result);
	} else {
		thread_die(thread);
        }	
}

/* 
 * do_map
 * 
 * Sends a mapping to a Mungi task and waits for the next message
 */
void 
do_map(l4_threadid_t faulter,void *addr,access_t acc){
        l4_ipc_reg_msg_t msg;
        l4_msgdope_t result;
        unsigned char write = 0;
	
	/* set read/write correctly for MIPS */
	if ((acc & M_WRITE) != 0)
		write = 1;

        msg.reg[0] = ((uintptr_t)addr) & L4_PAGEMASK;
        msg.reg[1] = l4_fpage((uintptr_t)addr,
                        L4_LOG2_PAGESIZE,write,L4_FPAGE_MAP).fpage;
        /* Sanity */
        msg.reg[2] = 0;
        msg.reg[3] = 0;

        /* deceiving send, because we might be called from outside the mpager */
	l4_ipc_send_deceiving(faulter, mpagerid, L4_IPC_SHORT_FPAGE,
			      &msg, L4_IPC_NEVER, &result);        
}


/* do_touch: used to touch a variable
 * usage   : do_touch( ptr, *(int*)var )
 * gcc  2.8.?? was giving odd results and optimising out some volatile
 * touches. Hopefully this functions fixes it. gcc 2.9 seems to fix it.
 * Maybe one day we can re-optimise this :)  ceg
 */
void 
do_touch( int *pvar, int var ){
       *pvar = var;
}

