/****************************************************************************
 *
 *      $Id: pending_requests.h,v 1.3 2002/07/22 10:17:39 cgray Exp $
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

#ifndef __MUNGI_PENDING_REQUESTS_H__
#define __MUNGI_PENDING_REQUESTS_H__

#include <sys/types.h>
#include "mungi/syscallbits.h"
#include "mungi/threads.h"
#include "mungi/upcall.h"

struct pending_syscall {
	mthread_t	*thread;
	l4_threadid_t	caller;
	syscall_t	sysmsg;
};

struct pending_pagefault {
	mthread_t	*thread;
	l4_threadid_t	faulter;
	l4_ipc_reg_msg_t msg;
};

struct pending_request {
	bool slot_used;
	bool is_syscall;
	union {
		struct pending_syscall   syscall;
		struct pending_pagefault pagefault;
	} data;
};

void pending_requests_init(void);
struct pending_request *pending_requests_get_slot(void);
void pending_requests_free_slot(struct pending_request *);

#endif /* __MUNGI_PENDING_REQUESTS_H__ */
