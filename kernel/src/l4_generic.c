/****************************************************************************
 *
 *      $Id: l4_generic.c,v 1.23.2.1 2002/08/30 06:00:00 cgray Exp $
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

/* (semi) generic L4 initialisation stuff */

#include "mungi/kernel.h"
#include "mungi/timing.h"
#include "mungi/l4_generic.h"
#include "mungi/mm.h"
#include "mungi/lock.h"
#include <clock.h>

#ifdef ALPHAENV
#include <l4/dit.h>
#else
#include <kernel/dit.h>
#endif

/* for the alpha lazy clock */
#ifdef ALPHAENV
extern l4_threadid_t kthread_start(void *, l4_threadid_t *, l4_threadid_t *);
#endif

struct l4_generic l4_setup;
Dit_Dhdr *dit_hdr;

void
l4_init(void)
{
	l4_ipc_reg_msg_t msg;
	l4_taskid_t me, task;
	l4_kernel_info *kinfo;
	l4_msgdope_t result;
        l4_threadid_t rpager;
	uintptr_t ptr;
	size_t size;
	int i, r;

#ifdef USE_NONTRIVIAL_TIMEOUTS

#ifdef MIPSENV
	/* timeout for kernel IPC */
	syscall_timeout = L4_IPC_TIMEOUT((byte_t)0, (byte_t)1, /* snd mant/exp*/
				       (byte_t)0, (byte_t)0, /* rcv man/exp */
				       (byte_t)5, (byte_t)15 /* page fault */
				       );
	pagefault_timeout = syscall_timeout;
	/* 100ms timeout */
	upcall_timeout = L4_IPC_TIMEOUT((byte_t)98, (byte_t)10,/* snd man/exp */
					(byte_t)0, (byte_t)0,/* rcv */
					(byte_t)0, (byte_t)0 /* page fault */
					);
#elsif ALPHAENV /* !MIPSENV  - L4/Alpha has different timeout structure to L4/MIPS */

	/* FIXME - These values need double checking */

	syscall_timeout = L4_IPC_TIMEOUT((byte_t)0, (byte_t)1, /* snd mant/exp*/
				       (byte_t)0, (byte_t)0, /* rcv man/exp */
				       (byte_t)98, (byte_t)2 /* page fault */
				       );
	pagefault_timeout = syscall_timeout;
	/* 100ms timeout */
	upcall_timeout = L4_IPC_TIMEOUT((byte_t)98, (byte_t)2,/* snd man/exp */
					(byte_t)0, (byte_t)0,/* rcv */
					(byte_t)0, (byte_t)0 /* page fault */
					);

#else
#error "Not ported to this Arch: Bored? Feel free to port :)"

#endif /* MIPSENV */
#else  /* USE_NONTRIVIAL_TIMEOUTS */
	syscall_timeout = L4_IPC_NEVER;
	pagefault_timeout = L4_IPC_NEVER;
	upcall_timeout = L4_IPC_NEVER;
#endif /* USE_NONTRIVIAL_TIMEOUTS */

	/* ask sigma0 to map in the kernel information page */
	kprintf( "mungi: about to get KIP\n" );
	msg.reg[0] = SIGMA0_KERNEL_INFO_MAP;

	/* This line has broken before due to L4 kernel bugs on both
	 * MIPS and Alpha (for different reasons!). You can change it
	 * for a send/recv pair, but that has also failed us before! cx
	 */
        l4_id_nearest(SIGMA0_TID,&rpager);
	r = l4_ipc_call( rpager, 
			 L4_IPC_SHORT_MSG, &msg,
			 L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE), &msg,
			 L4_IPC_NEVER, &result );
	kprintf("mungi: got KIP\n");
	if (r != 0)
		panic("Could not map in the kernel information page");

	kinfo = (l4_kernel_info *)(msg.reg[0] & L4_PAGEMASK);

	kprintf("mungi: msg.reg[0] is 0x%lx\n", msg.reg[0]);
	kprintf("mungi: kinfo      is 0x%p\n", kinfo);

	/* sanity check on the info page */
	if (strncmp((char *)&kinfo->magic, "L4uK", 4))
		panic("L4 kernel information magic mismatch");

	VERBOSE("Memory size: %d bytes\n", (int)kinfo->memory_size);
	VERBOSE("kernel end: 0x%p\n", (void*)(kinfo->kernel));
	VERBOSE("kernel_data start: 0x%p\n", (void*)(kinfo->kernel_data));



	/* map all available memory into our address space */ 
	/* Not needed unless stand alone 
	for (ptr = (uintptr_t)kinfo->kernel; ptr < (uintptr_t)kinfo->kernel_data
			; ptr += L4_PAGESIZE) {
		msg.reg[0] = ptr;
		r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
				L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE), &msg, 
				L4_IPC_NEVER, &result);
		_assert(r == 0);
	}
	*/
	
        /* parse the dit header(MIPS L4 specific) */
	VERBOSE("kinfo %p\n",kinfo);
        l4_setup.kinfo = kinfo;
	dit_hdr = (Dit_Dhdr *)kinfo->dit_hdr;
	
        /* fill in the memory map structure */
	/* FIXME: Should init this from rpager data */
        l4_setup.memory_start = (void *)((uint64_t)(dit_hdr->d_vaddrend) &
                                         L4_PAGEMASK) + L4_PAGESIZE;
        l4_setup.memory_end = (void *)kinfo->kernel_data;

	VERBOSE("DIT header is at %p\n",dit_hdr);	
	VERBOSE("dit ident is %4s\n",dit_hdr->d_ident);
	if (strncmp((char *)dit_hdr->d_ident, "dhdr", DIT_NIDENT))
	        panic("DIT header magic mismatch");
	VERBOSE("dit ident exists\n");
	ptr = (uintptr_t)dit_hdr + (uintptr_t)dit_hdr->d_phoff;
	size = (size_t)dit_hdr->d_phsize;
        /* work out the dit start & size */
	l4_setup.dit_start = dit_hdr;
	l4_setup.dit_len = dit_hdr->d_phoff + size * dit_hdr->d_phnum;

	VERBOSE("searching dit...\n");
	for (i = 0; i < (int)dit_hdr->d_phnum; i++) {
		Dit_Phdr *dit_p = ((Dit_Phdr *)(ptr + (uintptr_t)(i*size)));
		char *p_name = dit_p->p_name;

		VERBOSE("DIT program %s found.\n", p_name);

		VERBOSE("base 0x%lx, entry 0x%lx, size 0x%lx\n",
			(long unsigned int)dit_p->p_base,
			(long unsigned int)dit_p->p_entry,
			(long unsigned int)dit_p->p_size);


		if (!strncmp(p_name, "init", 4)) {
			l4_setup.object_tbl.base = (void *)(uintptr_t)
								dit_p->p_base;
			l4_setup.object_tbl.entry = (void *)(uintptr_t)
								dit_p->p_entry;
                        l4_setup.object_tbl.size = (size_t)(dit_hdr->d_vaddrend
                                        - dit_p->p_base);
		} else if (!strncmp(p_name, "mungi", 5)) {
			l4_setup.mungi.base = (void *)(uintptr_t)dit_p->p_base;
			l4_setup.mungi.entry = (void *)(uintptr_t)
								dit_p->p_entry;
			l4_setup.mungi.size = (size_t)dit_p->p_size;
		} else if (!strncmp(p_name, "upcall", 6)) {
			l4_setup.upcall.base = (void *)(uintptr_t)dit_p->p_base;
			l4_setup.upcall.entry = (void *)(uintptr_t)
								dit_p->p_entry;
                        l4_setup.upcall.size = (size_t)dit_p->p_size;
		} else {
			VERBOSE("Unknown dit section %s.\n", p_name);
		}
	}


	VERBOSE("l4_setup: %p\n", (void*)l4_setup.mungi.entry);
	VERBOSE("l4_setup: %p\n", (void*)l4_setup.upcall.entry);
	VERBOSE("l4_setup: %p\n", (void*)l4_setup.object_tbl.entry);


	/* reserve all L4 tasks with a greater id */
	task.ID = 0;
	task.id.task = -1; /* fill out the task field */
	l4_setup.l4_task_end = (int)task.id.task;
	task = me = l4_myself();
	l4_setup.l4_task_start = (int)task.id.task + 1;
	VERBOSE("L4 task id range %d to %d\n", l4_setup.l4_task_start,
		l4_setup.l4_task_end);

#ifdef MIPSENV
                /* Alpha does not need to reserve tasks, they are already
                 * owned by us. We could do this operation but it would be
                 * pointless.
                 */

	do {
	        l4_taskid_t ret;
		task.id.task++;

		ret = l4_task_new(task, *(uintptr_t *)&me, 0, 0, L4_NIL_ID, 
				  L4_NIL_ID);
		assert(l4_is_nil_id(ret) == 0);
	} while (task.id.task != l4_setup.l4_task_end);
#endif

}


time_t
gettime(void)
{
#ifdef MIPSENV
	return l4_setup.kinfo->clock / 1000;
#else /* ALPHA */
	return l4_sys_time() / 1000 / 1000;
#endif
}

#ifdef ALPHAENV

/* on alpha we need a syscall, so only do this once in a while */
lock_t    clock_lock;
unsigned long clock_time;
 
static void lazy_clock_thread(void);
static void 
lazy_clock_thread(void)
{
	l4_threadid_t myself;
 	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
	l4_timeout_t timeout;
	unsigned long tmp_time;
	myself = l4_myself();
	/* about 1 sec timeout */
	timeout = L4_IPC_TIMEOUT((byte_t)0, (byte_t)0, /* snd man/exp */
				 1, 5,   /* rcv */
				 (byte_t)0, (byte_t)0  /* pagefault */
				 );

	for(;;)
	{
		l4_ipc_receive( myself, L4_IPC_SHORT_MSG, &msg, 
				timeout, &result );


		tmp_time = gettime();
		_lock_lock( &clock_lock );
		clock_time = tmp_time;
		_lock_unlock( &clock_lock );
	}
}

void
init_lazy_clock(void)
{
	clock_time = 0;

	_lock_init( &clock_lock );
	
	kthread_start( lazy_clock_thread, NULL, NULL );
}

time_t
gettime_lazy(void)
{
	unsigned long ret;

	_lock_lock( &clock_lock );
	ret = clock_time;
	_lock_unlock( &clock_lock );

	return ret;
}

#endif
