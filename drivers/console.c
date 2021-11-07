/****************************************************************************
 *
 *      $Id: console.c,v 1.7 2002/05/31 05:10:14 danielp Exp $
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

#include "mungi/l4_generic.h"
#include "mungi/kernel.h"
#include "drivers/console.h"

#ifdef MIPSENV
#include "drivers/mips/timing.h"
#endif

static l4_threadid_t serial_id, listener_id = L4_INVALID_ID;

extern l4_threadid_t kthread_start(void *, l4_threadid_t *, l4_threadid_t *);
static void console_in(void);

void
console_init(l4_threadid_t sid)
{
	serial_id = sid;
	VERBOSE("\r\n\n\n\nKernel printing services started\r\n");
}

void
devices_init(void)
{
#ifdef MIPSENV
	/* Initialise the timerchip */
	inittimer(); 
#endif
}

void
start_console_in(void)
{
	kthread_start(console_in, NULL, NULL);
}

void
console_out(const char *buffer, size_t size)
{
	l4_msgdope_t result;
	l4_ipc_reg_msg_t msg;
	int p, r;

	for (p = 0; p < size; p += sizeof(msg.reg)) {
		memcpy(&msg.reg[0], &buffer[p],
			size-p < sizeof(msg.reg) ? size-p+1 : sizeof(msg.reg));
		r = l4_ipc_send(serial_id, L4_IPC_SHORT_MSG, &msg, 
				L4_IPC_NEVER, &result);
		assert(r == 0);
	}
}

int
console_setreader(l4_threadid_t new_listener)
{
	VERBOSE("Setting caller to 0x%llx\n", (long long) new_listener.ID);
	listener_id = new_listener;
	return 1; /* OK */
}

static void
console_in(void)
{
	int r;
	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;

	/* say hello */
	kprintf("cin: I'm alive - %llx\n", (long long) l4_myself().ID);

	/* register with rpager kprintfd to listen for characters */

	/* setup the magic value */
	msg.reg[0] = 0;

	/* send */
	r = l4_ipc_send(serial_id, L4_IPC_SHORT_MSG, &msg, 
			 L4_IPC_NEVER, &result);
	assert(r == 0);

	/* now listen for characters */
	while (1)
	{
		r = l4_ipc_receive(serial_id, L4_IPC_SHORT_MSG, &msg,
				   L4_IPC_NEVER, &result);
		assert(r == 0);

		/* forward it? */
		/* FIXME: do we want a lock on listener? it's once
		   off but we could get some *nasty* results */
		if (listener_id.ID != L4_INVALID_ID.ID) {
			r = l4_ipc_send(listener_id, L4_IPC_SHORT_MSG,
					&msg, L4_IPC_NEVER, &result);
			assert(r == 0);
		}
	}
}

