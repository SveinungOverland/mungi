/****************************************************************************
 *
 *      $Id: upcall.c,v 1.16.2.1 2002/08/30 05:59:59 cgray Exp $
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

/* L4 thread upcall */

#include "mungi/kernel.h"
#include "mungi/upcall.h"
#include "mungi/syscallbits.h"
#include "mungi/l4_generic.h"
#include "mungi/clock.h"

/* prototypes */
void upcall(void *);
void trampoline(void);
void initstart(void);
static void thread_wait(void);
void thread_return(cap_t cap);
static void task_init(void *);
void upcall_thread_sleep(time_t time);
void UserPrint(const char *, ...);

/* in asm/$ARCH/stop.S */
extern void save_me(void);
extern void restore_me(void);


/* L4 id of mungi */
static l4_threadid_t mungi_tid;

/* FIXME: This is here for compilers benefit */
extern void l4dbg_debug(void);
extern void l4dbg_write_string(const char *);

/*
 * local thread that handles all the exceptions/upcalls for a L4 task
 */
void
upcall(void * stack)
{
	upcall_t msg;
	l4_msgdope_t result;
	uintptr_t ip, sp, old_ip, old_sp, dummy;
	l4_threadid_t same;
	int r;
        static int clock_inited = 0;


	/* Find Mungi's L4 thread id */
        l4_id_nearest(SIGMA0_TID, &mungi_tid);

	/* init clock driver */
	clock_init();
	clock_inited = 1;

	/* start the user thread */
	task_init(stack);


        
	/*
	 * Handle upcalls from Mungi.
	 */
	while (1) {
	 	/* wait for a message from mungi */
		r = l4_ipc_wait(&same, L4_IPC_SHORT_MSG, &msg.msg, L4_IPC_NEVER
				, &result);
		/* check for valid IPC */
		if ( (r != 0) || (!task_equal(same, mungi_tid)))
			continue;


		switch (msg.upcall.type) {
		case UPCALL_RESUME:
			same = L4_INVALID_ID;
			l4_thread_ex_regs(msg.upcall.data.resume.l4id,
					  (uintptr_t)-1, (uintptr_t)-1, &same,
					  &same, &dummy, &dummy);
			break;

		case UPCALL_DELETE:
			same = L4_INVALID_ID;
			l4_thread_ex_regs(msg.upcall.data.delete.l4id,
					  (uintptr_t)thread_wait, (uintptr_t)-1,
					  &same, &same, &dummy, &dummy);
			break;

		case UPCALL_CREATE:
			same = L4_INVALID_ID;
			l4_thread_ex_regs(msg.upcall.data.create.l4id,
					  (uintptr_t)trampoline, (uintptr_t)
					  msg.upcall.data.create.stack, &same,
					  &same, &dummy, &dummy);
			break;

		case UPCALL_EXCPT:
			ip = sp = (uintptr_t)-1;
			same = L4_INVALID_ID;
			l4_thread_ex_regs(msg.upcall.data.excpt.l4id, ip
					, sp, &same, &same, &old_ip, &old_sp);

			*(uintptr_t *)old_sp = 
				(uintptr_t)msg.upcall.data.excpt.handler;
                        
                        /* should we push info onto the stack? */
			UserPrint( "Handling UPCALL_EXCPT!\n" );
			UserPrint( "old_sp: 0x%llx\n", (long long)old_sp );
			UserPrint( "old_ip: 0x%llx\n", (long long)old_ip );
                        
			l4_thread_ex_regs(msg.upcall.data.excpt.l4id,
                                  (uintptr_t)msg.upcall.data.excpt.handler, 
                                  sp,&same, &same, &old_ip, &old_sp);

			break;

		case UPCALL_SLEEP:
                        upcall_thread_sleep( msg.upcall.data.sleep.time);
			break;
		default:
                        UserPrint("Invalid upcall\n");
			assert(!"invalid upcall");
		}
	}
}

void
upcall_thread_sleep(time_t time)
{
	l4_msgdope_t result;
	l4_timeout_t timeout;

	if (time == SLEEP_INFINITY) {
		timeout = L4_IPC_NEVER;
	} else {
		byte_t mantissa = 0, exponent = 0;
                timeout = L4_IPC_TIMEOUT((byte_t)0, 
                                         (byte_t)0, /* snd man/exp */
					 mantissa, exponent,   /* rcv */
					 (byte_t)0, (byte_t)0  /* pagefault */
					 );
	}

	l4_ipc_sleep(timeout, &result);
}

static void
thread_wait(void)
{
	l4_msgdope_t result;

	l4_ipc_sleep(L4_IPC_NEVER, &result);
	assert(0); /* should never get here */
}

void
thread_return(cap_t cap)
{
        syscall_t sysmsg;
        l4_msgdope_t result;
	int r;

        sysmsg.syscall.number = SYS_THREAD_RETURN;
        sysmsg.syscall.data.thread.retcap = cap;

	clock_stop(PRE_CE_IPC);

        /* l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &sysmsg.msg,  */
	/* FIXME: hard-coded thread ID!! */
        r = l4_ipc_call(mungi_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
			L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);
	assert(r == 0);

	clock_stop(POST_PDX_IPC);

	/* Here we set sp and */
#ifdef ALPHAENV
	asm volatile("mov         %0, $30\n"
		     "jmp         trampoline\n"
		     :
		     :
		     "r" (sysmsg.upcall.data.create.stack)
		     );
#elif MIPSENV
	asm volatile("ori      $29, %0, 0\n"
		     "j        trampoline\n"
		     :
		     :
		     "r" (sysmsg.upcall.data.create.stack)
		     );
#endif

	assert(0);
}

static void
task_init(void * stack)
{
	l4_threadid_t thread, same;
	uintptr_t dummy;

	/*
	 * Start the user thread
	 */
	thread = l4_myself();
	thread.id.lthread ++; 
	same = L4_INVALID_ID;
	l4_thread_ex_regs(thread, (uintptr_t)initstart, (uintptr_t)stack, 
			  &same, &same, &dummy, &dummy);
}

#ifdef ALPHAENV
void 
__assert(const char *msg, const char *file, int line) 
{
	/* Does nothing special at the moment except enter the debugger.
	 * Would be nice to save register state perhaps..
	 */
/*	printf("ASSERT: %s, %s:%d\n\r", msg, file, line); */
	l4dbg_write_string( "assertion failed in upcall!\n" );
    
	l4dbg_debug();
}
#endif

/* for debugging in the upcall thread */
void
UserPrint(const char *s, ...)
{
	va_list ap;
	syscall_t sysmsg;
	l4_msgdope_t result;

	va_start(ap, s);
	vsprintf((char *)&sysmsg.msg, s, ap);
	va_end(ap);

	sysmsg.syscall.number = 0;
	l4_ipc_call(mungi_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
			&sysmsg.msg, L4_IPC_NEVER, &result);
}
