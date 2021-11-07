/****************************************************************************
 *
 *      $Id: syscallbits.h,v 1.15.2.2 2002/08/30 05:59:57 cgray Exp $
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

#ifndef __M_SYSCALLBITS_H__
#define __M_SYSCALLBITS_H__

#include "sys/types.h"
#include "mungi/l4_generic.h"
#include "mungi/upcall.h"
#include <exception.h>


#define SYSCALLS_MAX		64

/* syscall numbers */
#define SYS_THREAD_RETURN	1
#define SYS_THREAD_CREATE	2
#define SYS_THREAD_INFO		3
#define SYS_THREAD_DELETE	4
#define SYS_THREAD_SLEEP	5
#define SYS_THREAD_RESUME	6
#define SYS_THREAD_WAIT		7
#define SYS_THREAD_MYID		8
#define SYS_OBJECT_CREATE	9
#define SYS_OBJECT_DELETE	10	
#define SYS_OBJECT_RESIZE	11
#define SYS_OBJECT_PAGER	12
#define SYS_OBJECT_INFO		13
#define SYS_OBJECT_PDX		14
#define SYS_OBJECT_PASSWD	15	
#define SYS_APD_INSERT		16
#define SYS_APD_DELETE		17
#define SYS_APD_GET		18
#define SYS_APD_FLUSH		19
#define SYS_APD_LOCK		20
#define SYS_APD_LOOKUP		21
#define SYS_PDX_CALL		22
#define SYS_EXCPT_REG		23
#define SYS_LAST_ERROR		24
#define SYS_PAGE_COPY		25
#define SYS_PAGE_MAP		26
#define SYS_PAGE_UNMAP		27
#define SYS_PAGE_FLUSH		28
#define SYS_SEM_CREATE		29
#define SYS_SEM_DELETE		30
#define SYS_SEM_WAIT		31
#define SYS_SEM_SIGNAL		32


union _thread_data {
	struct {
		thread_t	ip;
		void 		*param;
		const threadinfo_t *info;
		const apddesc_t	*pd;
	} create;

	struct {
		mthreadid_t	tid;
		time_t		time;
	} sleep;

	struct {
		mthreadid_t	tid;
	} wait;

	struct {
		mthreadid_t	tid;
	} resume;

	struct {
		mthreadid_t	tid;
		int		status;
		bool		adopt;
	} delete;

	struct {
		mthreadid_t	tid;
		threadinfo_t	*info;
	} info;

	int retval;
	cap_t retcap;
};


union _object_data {
	struct {
		size_t		size;
		passwd_t	passwd;
		const objinfo_t *info;
	} create;

	struct {
		cap_t		cap;
		access_t	mode;
	} passwd;

	struct {
		void 		*obj;
		pager_t 	pager;
	} pager;

	struct {
		void		*obj;
		size_t		new_size;
	} resize;

	struct {
		const void	*obj;
		int		flags;
		objinfo_t 	*info;
	} info;

	struct {
		cap_t		cap;
		const clist_t 	*clist;
		uint		n_entrypt;
		pdx_t		*entry_pnts;
	} pdx;

	void *delete;
};

union _apd_data {
	struct {
		apddesc_t *buffer;
	} get;

	struct {
		const clist_t	*clist;
		apdpos_t	pos;
	} insert;

	struct {
		apdpos_t	pos;
	} delete;

	struct {
		apdpos_t	pos;
	} lock;

	struct {
	        void	        * address;
		access_t	minrights;
	} lookup;
};

union _sem_data {
	struct {
		void		* address;
		int		value;
		int		flags;
	} create;

	struct {
		void		* address;
	} delete;

	struct {
		void		* address;
	} wait;

	struct {
		void		* address;
	} signal;
};

union _page_data {
	struct {
		const void 	*from;
		void 		*to;
		uint		n_pages;
	} copy;

	struct {
		const void 	*from;
		void 		*to;
		uint		n_pages;
		access_t	mode;
		bool		fault_in;
	} map;

	struct {
		void 		*page;
		uint		n_pages;
		pagedisp_t	disp;
	} unmap;

	struct {
		const void 	*page;
		uint		n_pages;
	} flush;
};

union _misc_data {
	struct {
		excpthndlr_t	handler;
		excpt_t 	exception;
	} reg;

	struct {
		pdx_t		proc;
		cap_t           param;
		const apddesc_t	*pd;
	} call;
};

/* structures used for syscall ipc to mungi */
union _syscall_data {
	union _thread_data thread;
	union _object_data object; 
	union _apd_data apd;
	union _misc_data excpt;
	union _misc_data pdx;
	union _page_data page;
	union _sem_data sem;
};

union _thread_return {
	struct {
		mthreadid_t	tid;
		l4_threadid_t	l4id;
		void	       *stack;
	} create;

	struct {
		l4_threadid_t	l4id;
		int		retval;
	} delete;

	struct {
		int		status;
		mthreadid_t	tid;
	} wait;

	mthreadid_t tid;
};

union _object_return {
	void		*addr;
};

union _misc_return {
	excpthndlr_t	handler;
	void		*addr;
	struct {
		cap_t		* cap_addr;
	} lookup;
	cap_t		* cap_addr;
	cap_t		cap;
};

struct _pdx_return {
	int		retval;  /* Swapped to make _retval union work - ceg */
	cap_t	cap;
};

/* return data from the syscall */
union _return_data {
	union _thread_return thread;
	union _object_return object;
	union _misc_return excpt;
	struct _pdx_return pdx;
	union _misc_return apd;
	int retval;
};

struct _sys_params {
	union _syscall_data data; /* data that can be passed in registers */
	unsigned long number; /* syscall number */
};

typedef union _syscall {
	l4_ipc_reg_msg_t msg;
	struct _sys_params syscall;
	struct _upcall_args upcall;
	union _return_data sysret;
} syscall_t;

#endif /* __M_SYSCALLBITS_H__ */
