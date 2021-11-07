/****************************************************************************
 *
 *      $Id: apd.c,v 1.11 2002/08/23 08:24:34 cgray Exp $
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

/* APD related system calls */

#include "syscalls.h"
#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallbits.h"


l4_threadid_t apd_tid; /* APD server thread id */


int
ApdInsert(apdpos_t pos, const clist_t *clist)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_APD_INSERT;
	sysmsg.syscall.data.apd.insert.clist = clist;
	sysmsg.syscall.data.apd.insert.pos = pos;

	l4_ipc_call(apd_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ApdDelete(apdpos_t pos)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_APD_DELETE;
	sysmsg.syscall.data.apd.delete.pos = pos;

	l4_ipc_call(apd_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ApdGet(apddesc_t *buffer)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_APD_GET;
	sysmsg.syscall.data.apd.get.buffer = buffer;

	l4_ipc_call(apd_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG, 
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ApdLock(apdpos_t pos)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_APD_LOCK;
	sysmsg.syscall.data.apd.lock.pos = pos;

	l4_ipc_call(apd_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

cap_t *
ApdLookup(const void *address, access_t minrights)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_APD_LOOKUP;
	sysmsg.syscall.data.apd.lookup.address = (void *)address;
	sysmsg.syscall.data.apd.lookup.minrights = minrights;

	l4_ipc_call(apd_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.apd.lookup.cap_addr;
}

/*
 * useless system call
 */
void
ApdFlush(void)
{
	syscall_t sysmsg;
	l4_msgdope_t result;
#ifndef ALPHAENV
       l4_fpage_t address_space;
#endif

	sysmsg.syscall.number = SYS_APD_FLUSH;

	l4_ipc_call(apd_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	/* flush our entire address space */
        /* Thie unmap code is broken on alpha and takes a few minutes
	 * to complete... aparently it crashes if called multiple times, too
	 * strangely enough I've commented it out. ceg 
	 */
#ifndef ALPHAENV        
	address_space = l4_fpage(0, L4_WHOLE_ADDRESS_SPACE, 0, 0); 
	l4_fpage_unmap(address_space, L4_FP_FLUSH_PAGE|L4_FP_ALL_SPACES);
#endif
}
