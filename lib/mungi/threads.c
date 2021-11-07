/****************************************************************************
 *
 *      $Id: threads.c,v 1.11 2002/08/23 08:24:35 cgray Exp $
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

/* thread system calls */

#include "syscalls.h"
#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallbits.h"

#include "mungi/clock.h"

extern void trampoline(void);
void thread_return(cap_t cap);


l4_threadid_t thread_tid; /* the mungi thread server L4 thread id */


mthreadid_t
ThreadCreate(thread_t ip, void *param, const threadinfo_t *info, 
                const apddesc_t *pd)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_THREAD_CREATE;
	sysmsg.syscall.data.thread.create.ip = ip;
	sysmsg.syscall.data.thread.create.param = param;
	sysmsg.syscall.data.thread.create.info = info;
	sysmsg.syscall.data.thread.create.pd = pd;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);
	
	/* possibly do a local thread creation */
	if (!l4_is_nil_id(sysmsg.sysret.thread.create.l4id)) {
		l4_threadid_t same;
		uintptr_t dummy;

		same.ID = L4_INVALID_ID.ID;

		l4_thread_ex_regs(sysmsg.sysret.thread.create.l4id, 
				  (uintptr_t)trampoline,
				  (uintptr_t)sysmsg.sysret.thread.create.stack
				  , &same, &same, &dummy, &dummy);
	}

	return sysmsg.sysret.thread.create.tid;
}

int
ThreadDelete(mthreadid_t tid, int status, bool adopt)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_THREAD_DELETE;
	sysmsg.syscall.data.thread.delete.tid = tid;
	sysmsg.syscall.data.thread.delete.status = status;
	sysmsg.syscall.data.thread.delete.adopt = adopt;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.thread.delete.retval;
}

int
ThreadSleep(mthreadid_t tid, time_t time)
{
	syscall_t sysmsg;
	l4_msgdope_t result;
	l4_timeout_t timeout = L4_IPC_NEVER;

	if (tid == THREAD_MYSELF) {
		byte_t mantissa, exponent;

		if (time == 0) {
                        exponent = 0;
			mantissa = 0;
		} else {
			long tmp;
			long tmp_time;

			tmp_time = time*1000; /* convert to µs*/

			for (exponent = 14; exponent >0; exponent--) {
				tmp = (2 << (15 - exponent));
				tmp *= tmp;
				if ((tmp_time/tmp) < 255)
					break;
			}
			mantissa = (byte_t)(tmp_time/tmp);
		}

		/* CEG's seconds only format :) */
		if (time){
		        mantissa = time / 1000000000;
		        exponent = 5;
		}
                
		timeout = L4_IPC_TIMEOUT((byte_t)0, (byte_t)0, /* snd man/exp */
					 mantissa, exponent,   /* rcv */
					 (byte_t)0, (byte_t)0  /* pagefault */
					 );
                /* this is a dodgey hack added by CEG. Just sleep in here
		   so Tony's test code works OK! */
		l4_ipc_receive(l4_myself(), L4_IPC_SHORT_MSG, &sysmsg.msg, 
			       timeout, &result );
		return 0;

	}

	sysmsg.syscall.number = SYS_THREAD_SLEEP;
	sysmsg.syscall.data.thread.sleep.tid = tid;
	sysmsg.syscall.data.thread.sleep.time = time;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ThreadResume(mthreadid_t tid)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_THREAD_RESUME;
	sysmsg.syscall.data.thread.resume.tid = tid;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);
	
	return sysmsg.sysret.retval;
}

mthreadid_t
ThreadWait(mthreadid_t tid, int *status)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_THREAD_WAIT;
	sysmsg.syscall.data.thread.wait.tid = tid;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	*status = sysmsg.sysret.thread.wait.status;
	
	return sysmsg.sysret.thread.wait.tid;
}

mthreadid_t
ThreadMyID(void)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_THREAD_MYID;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);
	
	return sysmsg.sysret.thread.tid;
}

int
ThreadInfo(mthreadid_t tid, threadinfo_t *info)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_THREAD_INFO;
	sysmsg.syscall.data.thread.info.tid = tid;
	sysmsg.syscall.data.thread.info.info = info;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
			&sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

void
thread_return(cap_t cap)
{
        syscall_t sysmsg;
        l4_msgdope_t result;

        sysmsg.syscall.number = SYS_THREAD_RETURN;
        sysmsg.syscall.data.thread.retcap = cap;

        l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
                    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);
	assert(0);
}

int
PdxCall( pdx_t proc, cap_t param, cap_t *ret, const apddesc_t *pd )
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_PDX_CALL;
	sysmsg.syscall.data.pdx.call.proc = proc;
	sysmsg.syscall.data.pdx.call.param = param;
	sysmsg.syscall.data.pdx.call.pd = pd;

	/* restart the clock timer */
	clock_start();
	clock_stop(PRE_CR_IPC);

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	*ret = sysmsg.sysret.pdx.cap;

	clock_stop(FINAL_MEASURE);
	clock_dump();

	return sysmsg.sysret.retval;
}

excpthndlr_t
ExcptReg(excpt_t exception, excpthndlr_t handler)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_EXCPT_REG;
	sysmsg.syscall.data.excpt.reg.handler = handler;
	sysmsg.syscall.data.excpt.reg.exception = exception;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.excpt.handler;
}

int
GetLastError(void)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_LAST_ERROR;

	l4_ipc_call(thread_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}
