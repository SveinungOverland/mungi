/****************************************************************************
 *
 *      $Id: mungi.c,v 1.20.2.1 2002/08/29 04:32:01 cgray Exp $
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

/* mungi initialisation */

#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/objects.h"
#include "mungi/syscalls.h"
#include "mungi/security.h"
#include "mungi/threads.h"
#include "mungi/mm.h"
#include "mungi/version.h"
#include "drivers/console.h"
#include "mungi/clock.h"

extern l4_threadid_t default_pager,mungi_kernel;
l4_threadid_t startupid;

int
main(void){
        l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
	l4_threadid_t myid;
	int r;

	r = l4_ipc_wait(&startupid,L4_IPC_SHORT_MSG,&msg,L4_IPC_NEVER,&result);

	if(msg.reg[0] != 12){
	    assert(!"Bad magic on init message\n");
	}
	console_init((l4_threadid_t)msg.reg[1]);
	kprintf("\n**************************************\n*\n*\n");
	kprintf("*  Mungi Kernel (c) 1996-2000 UNSW\n");
	kprintf("*  All rights reserved\n");
	kprintf("*  UNSW Distributed Systems Group\n");
	kprintf("*  Kernel version %d.%d\n", MUNGIVERSIONH, MUNGIVERSIONL ); 
	kprintf("*\n*\n**************************************\n");

	myid = l4_myself();
	kprintf("mungi: my id is 0x%lx\n", myid.ID);

	l4_init();
	
	kprintf("mungi: still here\n");

	mm_init((l4_threadid_t)msg.reg[2],  /* the pager */
		(void *)msg.reg[4],         /* heap */
		(void *)msg.reg[5]);        /* free list */ 

	kprintf("mungi: still here\n");

	/* devices_init(); */ /* Commented out by Brett */

	kprintf("mungi: about to objtable_init\n");
	objtable_init((void *)msg.reg[6]); /* The object table */
	kprintf("mungi: about to security_init\n");
	security_init();
	kprintf("mungi: about to apd_init\n");
        apd_init();
	kprintf("mungi: about to thread_init\n");
	thread_init((l4_threadid_t)msg.reg[3]);
        kprintf("mungi: threadinit done\n");
	kprintf("mungi: about to clock_init\n");
	clock_init();
	kprintf("mungi: clock_init done\n");

	/* start the lazy clock thread */
	kprintf("mungi: initialising the lazy clock\n");
        init_lazy_clock();
	kprintf("mung: lazy clock thread initialised\n");

        kprintf("mungi: waiting for 0x%016lx\n",startupid.ID);
        r =l4_ipc_wait(&startupid,L4_IPC_SHORT_MSG,&msg,L4_IPC_NEVER,&result);
	//r = l4_ipc_receive(startupid,L4_IPC_SHORT_MSG,&msg,L4_IPC_NEVER,
	//			&result);
        kprintf("mungi: sending to pager...\n");
	r = l4_ipc_send(default_pager,L4_IPC_SHORT_MSG,&msg,L4_IPC_NEVER,
				&result);
	kprintf("mungi:sent messages\n");
	
	syscall_loop(); /* start processing system calls. Does not return. */
}
