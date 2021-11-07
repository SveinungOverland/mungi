/****************************************************************************
 *
 *      $Id: l4_generic.h,v 1.14 2002/07/22 10:17:38 cgray Exp $
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

/* (semi) generic L4 defines */
#ifndef __M_L4_GENERIC_H
#define __M_L4_GENERIC_H

/* FIXME: this is *sooo* wrong */
#define write __dummy__
#include <l4/types.h>
#undef write


#include <l4/ipc.h>
#include <l4/syscalls.h>
#include <l4/sigma0.h>
#include "mungi/types.h"


/* system data */
struct range {
	void	*base;
	void	*entry;
	size_t	size; 
};
struct l4_generic {
	void *memory_start;
	void *memory_end;
	int l4_task_start;
	int l4_task_end;
	struct range upcall;
	struct range mungi;
	struct range object_tbl;
        struct range auth;
        struct range pdx;
        l4_kernel_info *kinfo;
        void *dit_start;
	int   dit_len;
};

extern struct l4_generic l4_setup;

/* arch (semi)independent defines */
#ifdef MIPSENV
#define L4_THREADS_PER_TASK	128

#define l4_ipc_receive l4_mips_ipc_receive
#define l4_ipc_call l4_mips_ipc_call
#define l4_ipc_reply_and_wait l4_mips_ipc_reply_and_wait 
#define l4_ipc_reply_deceiving_and_wait l4_mips_ipc_reply_deceiving_and_wait
#define l4_ipc_send l4_mips_ipc_send
#define l4_ipc_send_deceiving l4_mips_ipc_send_deceiving
#define l4_ipc_send_deceiting l4_mips_ipc_send_deceiving
#define l4_ipc_wait l4_mips_ipc_wait
#define l4_ipc_receive l4_mips_ipc_receive
#define l4_ipc_sleep l4_mips_ipc_sleep
#endif

#ifdef ALPHAENV
#define L4_THREADS_PER_TASK	256

#include <l4/generic.h>
L4_INLINE int l4_alpha_ipc_sleep(l4_timeout_t t, l4_msgdope_t *result);
L4_INLINE int l4_alpha_ipc_sleep(l4_timeout_t t, l4_msgdope_t *result) {
    return l4_alpha_ipc_receive(l4_myself(), NULL, NULL, t, result);
}
#endif

void l4_init(void);
time_t gettime(void);

/* get the time lazily (only seconds resolution) */
#ifdef MIPSENV
#define gettime_lazy()    gettime()
#define init_lazy_clock() ((void)0) 
#else
void init_lazy_clock(void);
time_t gettime_lazy(void);
#endif

/* these should be added to the L4/Alpha Tree */
#define L4_FPAGE_RW_MASK 2

#endif /* __M_L4_GENERIC_H */
