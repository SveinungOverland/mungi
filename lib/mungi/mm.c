/****************************************************************************
 *
 *      $Id: mm.c,v 1.7 2002/05/31 07:51:18 danielp Exp $
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

/* page and memory system calls */

#include "syscalls.h"
#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallbits.h"


l4_threadid_t mm_tid; /* page/memory server thread id */


int
PageCopy(const void *from, void *to, uint n_pages)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_PAGE_COPY;
	sysmsg.syscall.data.page.copy.from = from;
	sysmsg.syscall.data.page.copy.to = to;
	sysmsg.syscall.data.page.copy.n_pages = n_pages;

	l4_ipc_call(mm_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
PageMap(const void *from, void *to, uint n_pages, access_t mode, bool fault_in)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number =	SYS_PAGE_MAP; 
	sysmsg.syscall.data.page.map.from = from;
	sysmsg.syscall.data.page.map.to = to;
	sysmsg.syscall.data.page.map.n_pages = n_pages;
	sysmsg.syscall.data.page.map.mode = mode;
	sysmsg.syscall.data.page.map.fault_in = fault_in;

	l4_ipc_call(mm_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
PageUnMap(void *page, uint n_pages, pagedisp_t disp)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_PAGE_UNMAP;
	sysmsg.syscall.data.page.unmap.page = page;
	sysmsg.syscall.data.page.unmap.n_pages = n_pages;
	sysmsg.syscall.data.page.unmap.disp = disp;

	l4_ipc_call(mm_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
PageFlush(const void *page, uint n_pages)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_PAGE_FLUSH;
	sysmsg.syscall.data.page.flush.page = page;
	sysmsg.syscall.data.page.flush.n_pages = n_pages;

	l4_ipc_call(mm_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

void SeedyPrint(char *msg)
{
        syscall_t sysmsg;
        l4_msgdope_t result;
        int len,pos = 0;

        len = strlen(msg);
        while(pos < len) {
                strncpy((char*) &sysmsg.msg, &msg[pos], 40);

                sysmsg.syscall.number = 0;
  
                l4_ipc_call(mm_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
                                L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, 
                                &result);
                pos += 40;
        }

        return;

}



