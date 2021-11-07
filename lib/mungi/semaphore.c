/****************************************************************************
 *
 *      $Id: semaphore.c,v 1.3 2002/05/31 07:51:18 danielp Exp $
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

/* semaphore system calls */

#include "syscalls.h"
#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallbits.h"


l4_threadid_t semaphore_tid; /* semaphore server thread id */


int
SemCreate(void * address, int value, int flags)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_SEM_CREATE;
	sysmsg.syscall.data.sem.create.address = address;
	sysmsg.syscall.data.sem.create.value = value;
	sysmsg.syscall.data.sem.create.flags = flags;

	l4_ipc_call(semaphore_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
SemDelete(void * address)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_SEM_DELETE;
	sysmsg.syscall.data.sem.delete.address = address;

	l4_ipc_call(semaphore_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
SemWait(void * address)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_SEM_WAIT;
	sysmsg.syscall.data.sem.wait.address = address;

	l4_ipc_call(semaphore_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
SemSignal(void *address)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_SEM_SIGNAL;
	sysmsg.syscall.data.sem.signal.address = address;

	l4_ipc_call(semaphore_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}
