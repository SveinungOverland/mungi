/****************************************************************************
 *
 *      $Id: startup.c,v 1.24.2.1 2002/08/30 06:00:04 cgray Exp $
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

#include "mungi/l4_generic.h"
#include "mungi/mm.h"
#include "dit.h"
#include "sys/types.h"
#include "mungi/btree.h"
#include "mungi/exception.h"
#include "mungi/threads.h"
#include "mungi/security.h"
#include "mungi/klibc.h"
#include "mungi/pager.h"

#include "vm/startup.h"
#include "vm/vm.h"
#include "vm/vm_types.h"

#include "vm/ramdisk.h"

#define SERIAL			// define if serial driver in DIT image
#define ADBG

#ifdef ADBG
#define debug(x...) kprintf(x)
#define pause()		l4_ipc_sleep(L4_IPC_TIMEOUT(0,0,2,6,0,0), &result)
#else				/* !ADBG */
#define pause()
#define debug(x...)
#endif				/* ADBG */

l4_threadid_t serverid;		/* Mungi server id */

l4_kernel_info *k;
Dit_Dhdr *dhdr;

void debug_lvl(int);
int io_init(void);
int io_status = IO_NONE;

extern l4_threadid_t SERIAL_TID1;

/*
 * This is the system startup thread.
 * It does some system initialisation and handshakes with Mungi server 
 */
void startup(void)
{
	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
	Dit_Phdr *phdr, *server = 0;
	l4_threadid_t myid, caller;
	uintptr_t serverip, serversp, addr;
	unsigned int i, r;
	l4_taskid_t task;
	myid = l4_myself();

	debug("startup: started, tid = 0x%lx\r\n", myid.ID);

    /*************************************************************************
     * map the kernel info page and print out stuff inside
     *************************************************************************/
	msg.reg[0] = SIGMA0_KERNEL_INFO_MAP;
	r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
			L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
			&msg, L4_IPC_NEVER, &result);
	assert(r == 0);

	k = (l4_kernel_info *) (msg.reg[0] & L4_PAGEMASK);

	/* L4 Info */
	debug("startup:\t%4s version %hd build %hd\n",
	      (char *) &(k->magic), k->version, k->build);
	debug("startup:\tMemory size %dMB\n", (int)(k->memory_size >> 20));

	debug("startup:\tL4 reserved below 0x%lx and above 0x%lx\n",
	      k->kernel, k->kernel_data);

	dhdr = (Dit_Dhdr *) (k->dit_hdr);

	/* find mungi */
	phdr = (Dit_Phdr *) ((uintptr_t) dhdr + dhdr->d_phoff);
	for (i = 0; i < dhdr->d_phnum; phdr++, i++) {
		debug("startup:\t%-12s addr 0x%-8lx size 0x%-8lx\n",
                                phdr->p_name, (uint64_t)phdr->p_base,
                                (uint64_t)phdr->p_size);
		if (!strcmp(phdr->p_name, "mungi.kernel")) {
			debug("startup: Found Mungi kernel :)\n");
			server = phdr;
		}
	}

	if (server == NULL)
		assert(!"Mungi not found\n");


	/* request all non kernel-reserved frames from sigma0 */
	phdr = (Dit_Phdr *) ((uintptr_t) dhdr + dhdr->d_phoff);

#ifdef SERIAL
	phdr++;			/* skip serial driver's pages */
#endif
	i = 0;
#ifdef ALPHAENV
	debug_lvl(0);
        /* FIXME: What the hell is this address?! */
	for (addr = (uintptr_t)0x1600000; addr < k->kernel_data;
	     addr += L4_PAGESIZE) {
#else
	for (addr = phdr->p_base; addr < k->kernel_data;
	     addr += L4_PAGESIZE) {
#endif
		if (!(i++ % 101))
			debug(".");	/* progress... */
		msg.reg[0] = addr;
		r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
				L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
				&msg, L4_IPC_NEVER, &result);
		assert(r == 0);

	}
	VERBOSE("\nstartup: mem touching done:\n");
	debug_lvl(1);

	VERBOSE("startup: calling vm_init()\n");
	vm_init();

	io_status = io_init();


	debug("startup: Sending to rpager:\n");
	l4_ipc_send(rpagerid, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
		    &result);

	/* wait for the MPR to start */
        l4_ipc_receive(mprid,L4_IPC_SHORT_MSG,&msg,L4_IPC_NEVER,&result);

	/* Start OS Server - Mungi */
	serverid.ID = 0LL;
        serverid.id.task = myid.id.task + 2;/* pick task number higher 
                                               than me - add another for
                                               the alpha serial driver. */
	serverip = server->p_entry;
	serversp = k->kernel_data - 8;
	debug("startup: starting mungi: IP 0x%016lx\n", serverip);
#ifdef MIPSENV
        serverid = l4_task_new(serverid, 0xf0, serversp, serverip, rpagerid,
                        myid);	/* excption handling thread is 
                                   this thread */
#else
        serverid = l4_task_new(serverid, 0xf0, serversp, serverip, rpagerid);
#endif

	assert(serverid.ID != 0);
#ifdef ADBG
	debug("startup:\tStarted '%s' server\t0x%lx\n",
	      server->p_name, serverid.ID);
#endif

	/* Grant Tasks to Mungi, but don't grant Mungi to itself!! */
	task.ID = 0LL;
	task.id.task = serverid.id.task + 1;

	/* FIXME: Task numberss... */
#ifdef MIPSENV
	for (; task.id.task < 255; task.id.task++) {
		l4_task_new(task, (uintptr_t) serverid.ID, 0, 0, L4_NIL_ID,
			    L4_NIL_ID);
	}
#else
	for (; task.id.task < 255; task.id.task++) {
		l4_task_new(task, (uintptr_t) serverid.ID, 0, 0,
			    L4_NIL_ID);
	}
#endif
	/* send (null) IPC to Mungi server (to give it my id) */
	debug("startup: Mungi server IPC\n");
	debug("startup: rpagerid.ID is 0x%016lx\n", rpagerid.ID);
	msg.reg[0] = 12;	/* Magic */
	msg.reg[1] = (uintptr_t)kprintfdid.ID;
	msg.reg[2] = (uintptr_t)rpagerid.ID;
	if (io_status == IO_NONE) {
		msg.reg[4] = -1;
		msg.reg[5] = -1;
		msg.reg[6] = 0;	/* Saved objtable */
	} else if (io_status == IO_NORMAL) {
		/* FIXME: Need the saved values..  */
		assert(!"IO_Normal");
	} else if (io_status != 0) {
		msg.reg[4] = 0;
		msg.reg[5] = 0;
		msg.reg[6] = 0;	/* Saved Objtable (0 = reset) */
	} else {
		assert(!"Unknown io_status\n");
	}

	r = l4_ipc_send(serverid, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
			&result);

	debug("startup: sending mungi stuff for pager\n");
	msg.reg[0] = 12;	/* Magic */
	msg.reg[1] = (uintptr_t) ctable;
	msg.reg[2] = (uintptr_t) ctable_size;
	msg.reg[3] = (uintptr_t) ftable;
	msg.reg[4] = (uintptr_t) ptable;
	msg.reg[5] = (uintptr_t) ptable_size;
	l4_ipc_send(serverid, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
		    &result);

	debug("startup: Mungi Pager IPC\n");

	while (1) {
		l4_ipc_wait(&caller, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
			    &result);
		debug("startup: We seem to have an exception\n");
		debug("startup: id is 0x%lx\n", caller.ID);
		debug("startup: Cause is 0x%x\n", (int) msg.reg[0]);
		debug("startup: EPC is 0x%p\n", (void *)msg.reg[1]);
		debug("startup: BVA is 0x%p\n", (void *)msg.reg[2]);
		debug("startup: waiting for the next one...\n");

	}
}
int io_init(void)
{
	l4_threadid_t excpt, tpager;
	uintptr_t oip, osp;

	mprid = l4_myself();
	mprid.id.lthread = 7;
 	excpt.ID = -1LL;
 	tpager.ID = -1LL;
	l4_thread_ex_regs(mprid,(uintptr_t)ramdisk_main,
			  (uintptr_t)&mpr_stack[STACK_SIZE - 1], 
			  &excpt, &tpager, &oip, &osp);

	return IO_NORMAL_RESTART;
}

/* This is the gateway for printing.  All communication to the serial driver
   should go through this thread.
*/
void kprintfd(void)
{
        int r;
        l4_ipc_reg_msg_t msg;
        l4_msgdope_t dope;
        l4_threadid_t caller, listener = L4_INVALID_ID;

        kprintf("kpd: Acting as serial relay for Mungi\n");
        kprintf("kpd: My id is 0x%016llx\n", (long long) l4_myself().ID);

        /* setup character input relay, too.
           this *could* have problems with any upper serial drivers
           printing and receiving input at the same time, but the
           recv. thread should only need to output for debug, so
           I don't care. ceg */
        kprintf( "kpd: Registering for character input\n" );
        msg.reg[1] = (uintptr_t) l4_myself().ID;
        msg.reg[0] = 0;

#ifdef SERIAL
        r = l4_ipc_send(SERIAL_TID1, L4_IPC_SHORT_MSG, &msg,
                        L4_IPC_NEVER, &dope);
	assert (r == 0);

#ifndef MIPSENV /* MIPS driver doesn't reply when registering */
        r = l4_ipc_receive(SERIAL_TID1, L4_IPC_SHORT_MSG, &msg,
                           L4_IPC_NEVER, &dope);
         assert( r == 0 );
#endif
#endif

        /* FIXME: check return value from call? */
        kprintf( "kpd: registered\n" );

        while (1) {
                r = l4_ipc_wait(&caller, L4_IPC_SHORT_MSG, &msg,
                                L4_IPC_NEVER, &dope);
                assert(r == 0);

                /* does this person want to listen? */
                if (msg.reg[0] == 0) {
                        listener = caller;
                        continue;
                }

#ifdef SERIAL
                /* decide if it's input or output */
                if (caller.id.task != SERIAL_TID1.id.task) {
                        // Force null termination
                        ((char *) (&msg.reg[0]))[63] = '\0';
                        kprintf("%s",(char *)&msg.reg[0]);
                } else {
                        if (listener.ID != L4_INVALID_ID.ID)
                                r = l4_ipc_send(listener, L4_IPC_SHORT_MSG,
                                                 &msg, L4_IPC_NEVER, &dope);
                }
#else
                ((char *)(&msg.reg[0]))[63] = '\0';
		kprintf("%s",(char *)&msg.reg[0]);
#endif /* SERIAL */
        }
}


void debug_lvl(int lvl)
{
#ifdef ALPHAENV
#ifdef DEBUG
	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;

	VERBOSE("startup: setting debug level to %d\n", lvl);
	msg.reg[0] = SIGMA0_DEBUG_LEVEL;
	msg.reg[1] = lvl;
	l4_ipc_send(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
		    &result);
#endif
#endif
}
