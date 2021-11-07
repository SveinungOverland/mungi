/****************************************************************************
 *
 *      $Id: objects.c,v 1.10.2.1 2002/08/29 04:32:08 cgray Exp $
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

/* object related system calls */

#include "syscalls.h"
#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallbits.h"


l4_threadid_t object_tid; /* object server thread id */


void * 
ObjCreate(size_t size, passwd_t passwd, const objinfo_t *info)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_OBJECT_CREATE;
	sysmsg.syscall.data.object.create.size = size;
	sysmsg.syscall.data.object.create.passwd = passwd;
	sysmsg.syscall.data.object.create.info = info;

	l4_ipc_call(object_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.object.addr;
}

int 
ObjDelete(void *obj)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_OBJECT_DELETE;
	sysmsg.syscall.data.object.delete = obj;

        l4_ipc_call(object_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
                        L4_IPC_SHORT_MSG,&sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ObjResize(void * obj, size_t new_size)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_OBJECT_RESIZE;
	sysmsg.syscall.data.object.resize.obj = obj;
	sysmsg.syscall.data.object.resize.new_size = new_size;

	l4_ipc_call(object_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ObjPasswd(cap_t cap, access_t mode)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_OBJECT_PASSWD;
	sysmsg.syscall.data.object.passwd.cap = cap;
	sysmsg.syscall.data.object.passwd.mode = mode;

	l4_ipc_call(object_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int 
ObjInfo(const void * obj, int flags, objinfo_t *info)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_OBJECT_INFO;
	sysmsg.syscall.data.object.info.obj = obj;
	sysmsg.syscall.data.object.info.flags = flags;
	sysmsg.syscall.data.object.info.info = info;

	l4_ipc_call(object_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
			&sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ObjCrePdx(cap_t cap, const clist_t * clist, uintptr_t unused1,
	  uintptr_t unused2, uintptr_t unused3, uint n_entrypt,
	  pdx_t entry_pnts[])
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_OBJECT_PDX;
	sysmsg.syscall.data.object.pdx.cap = cap;
	sysmsg.syscall.data.object.pdx.clist = clist;
	sysmsg.syscall.data.object.pdx.n_entrypt = n_entrypt;
	sysmsg.syscall.data.object.pdx.entry_pnts = entry_pnts;
        
	l4_ipc_call(object_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

int
ObjNewPager(void *obj, pager_t pager)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_OBJECT_PAGER;
	sysmsg.syscall.data.object.pager.obj = obj;
	sysmsg.syscall.data.object.pager.pager = pager;

	l4_ipc_call(object_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_SHORT_MSG,
		    &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}
