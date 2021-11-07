/****************************************************************************
 *
 *      $Id: upcall.h,v 1.4.2.1 2002/08/30 05:59:57 cgray Exp $
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

#ifndef __M_UPCALL_H
#define __M_UPCALL_H

#include "sys/types.h"
#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include <exception.h>

void upcall_init(void);
int upcall_sleep(l4_threadid_t, time_t);
int upcall_resume(l4_threadid_t);
int upcall_create(l4_threadid_t, void *);
int upcall_send(l4_threadid_t, int);
int upcall_waitret(l4_threadid_t, mthreadid_t, int);
int upcall_excp(l4_threadid_t, void *, void *, excpt_t);

enum {UPCALL_DELETE, UPCALL_CREATE, UPCALL_EXCPT, UPCALL_RESUME, UPCALL_SLEEP};

union _upcall_data { 
	struct {
		l4_threadid_t	l4id;
		void		*stack;
	} create;

	struct {
		l4_threadid_t	l4id;
	} resume;

	struct {
		l4_threadid_t	l4id;
	} delete;

	struct {
		l4_threadid_t	l4id;
		time_t		time;
	} sleep;

	struct {
		excpthndlr_t	handler;
		void		*data;
		l4_threadid_t	l4id;
		excpt_t         exception;
	} excpt;
};

struct _upcall_args {
	union _upcall_data	data;
	int			type;
};

typedef union {
	l4_ipc_reg_msg_t msg;
	struct _upcall_args upcall;
} upcall_t;

#endif /* __M_UPCALL_H */
