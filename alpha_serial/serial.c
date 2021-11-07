/****************************************************************************
 *
 *      $Id: serial.c,v 1.5 2002/07/22 10:17:31 cgray Exp $
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

/*
 *        Project:  L4/Alpha 
 *        Created:  21/03/2000 17:26:51 by Simon Winwood (sjw)
 *  Last Modified:  06/12/2000 11:04:18 by Simon Winwood (sjw)
 *   Version info:  $Revision: 1.5 $ 
 *    Description:
 *           Serial driver for L4/Alpha.  Loosely based on L4/MIPS.
 *
 *       Comments:
 *           Need to seperate into a few files.
 *           Need to also fix the timeouts up.
 */

#include <stdarg.h>
#include <klibc.h>
#include <l4/assert.h>
#include <l4/types.h>
#include <l4/ipc.h>
#include <l4/syscalls.h>
#include <l4/sigma0.h>
#include <l4/kernel.h>

#include "io.h"
#include "serial.h"

#define STACK_SIZE 2048
uint64_t _stack[STACK_SIZE];
uint64_t recv_stack[STACK_SIZE];
uint64_t *_sp = &_stack[STACK_SIZE -1];

l4_threadid_t serial_tid;

/*
 * Apparently the multia uses a '82378IB', 
 *
 *
 */

volatile l4_threadid_t receiver = L4_INVALID_ID;

/* This is the IRQ that we want to register for.  Note that there
 * is only _one_ PCI IRQ, so this sux when we have an OS on top of
 * L4 
 */
#define PCI_IRQ 3

#define SERIAL_INT   (1 << 4)
#define CASCADE_INT  (1 << 2)

#define MASKED_INTS (~(SERIAL_INT | CASCADE_INT))

#define LEVEL_TRIGGERED (1 << 5 | 1 << 9 | 1 << 10 | 1 << 14 | 1 << 15)

extern void l4dbg_write_string(char *);

void console_out(char* msg)
{
	l4dbg_write_string(msg);
}

static void init_sio(void)
{

    /* Setup edge/level triggered */
    kprintf("*** PCI/Serial: Setting edge/level control register. "
	    "Current value: 0x%x\n\r",
	    ((inb(0x4d1) << 8) | inb(0x4d0)) & 0xffff);

    outb((unsigned char)LEVEL_TRIGGERED, 0x4d0);
    outb(LEVEL_TRIGGERED >> 8, 0x4d1);

    kprintf("*** PCI/Serial: Set edge/level control register. "
	    "Current value: 0x%x\n\r",
	    ((inb(0x4d1) << 8) | inb(0x4d0)) & 0xffff);

    outb(MASKED_INTS & 0xFF, 0x21);
    outb((MASKED_INTS >> 8) & 0xFF, 0xA1);

    kprintf("*** PCI/Serial: Current INT mask: 0x%x.  Current IRR: 0x%x. ", 
	    ((inb(0xA1) << 8) | inb(0x21)) & 0xffff, 
	    ((inb(0xA0) << 8) | inb(0x20)) & 0xffff);

    /* guess ... clobber any pending interrupts */
    outb(0x0f, 0x20);
    outb(0x0f, 0xA0);
    
    kprintf("Current ISR: 0x%x\n\r",  ((inb(0xA0) << 8) | inb(0x20)) & 0xffff);
    
    /* Set IRR read mode again */
    outb(0x0a, 0x20);
    outb(0x0a, 0x20);
    
    kprintf("*** PCI/Serial: Current IRR: 0x%x\n\r", 
	    ((inb(0xA0) << 8) | inb(0x20)) & 0xffff);
}

static void 
recv_thread(void)
{
    int r;
    l4_threadid_t sender;
    l4_ipc_reg_msg_t msg;
    l4_msgdope_t result;
    int intnum, c;

    /* register for interrupt */
    sender = (l4_threadid_t) {0x80 | PCI_IRQ};

    kprintf("*** PCI/Serial: Registering for IRQ %d(0x%lx) ***\n\r", PCI_IRQ, sender.ID);

    r = l4_alpha_ipc_receive(sender, L4_IPC_SHORT_MSG, &msg, 
		       L4_IPC_NEVER, &result);
    assert(r == 0);

    init_sio();
#if 0
    l4_alpha_ipc_send(serial_tid, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER, &result);
#endif
    sender = (l4_threadid_t) {PCI_IRQ};

    for(;;) {
	r = l4_alpha_ipc_receive(sender, L4_IPC_SHORT_MSG, &msg, 
				 L4_IPC_NEVER, &result);
	assert(r == 0);
	
	intnum = msg.reg[1] >> 32;
	intnum &= 0xff;
	if(intnum > 7) {
	    /* ack slave */
	    outb(0xE0 | (intnum - 8), 0xa0);
	    intnum = 2;
	}
	outb(0xE0 | intnum, 0x20);
	
	/* Don't ask ;) */
	c = dev_getc(0);

	/* break into the L4 kernel debugger */
	if( c == 0 || c == 27 )
	{
		asm( "call_pal 0xbf" );

		/* in case they continue */
		continue;
	}

	/* Yeah, this is seedy, but this is sort of how mips does it */
	if(receiver.thread_id != L4_INVALID_ID.thread_id) {
                msg.reg[0] = (qword_t)c;
	        r = l4_alpha_ipc_send(receiver, L4_IPC_SHORT_MSG, &msg, 
				  L4_IPC_NEVER, &result);
	}
    }
}

/* get main to do the init and receive to avoid races */
int 
main(void)
{
    l4_threadid_t sender, thread, dummy;
    l4_ipc_reg_msg_t msg;
    l4_msgdope_t result;
    unsigned long unused;
    char *cptr;
    int r, i;

    serial_tid = thread = l4_myself();

    kprintf( "*** L4/Alpha Serial driver started\n\r" );
    kprintf( "serial: my id is 0x%lx\n\r", thread.ID );

    dev_init();

    /* start up the interrupt handler */
    dummy = L4_INVALID_ID;
    
    thread.id.lthread++;
    l4_thread_ex_regs(thread, (unsigned long) recv_thread, 
		      (unsigned long) &(recv_stack[STACK_SIZE - 1]), 
		      &dummy, &dummy, &unused, &unused);


    /* Receive loop */
    while(1) {
	r = l4_alpha_ipc_wait(&sender, L4_IPC_SHORT_MSG, &msg, 
			L4_IPC_NEVER, &result);

	if(r != 0) {
                kprintf("*** PCI/Serial: Wait error %d(0x%lx)\n\r", r, 
                                result.msgdope);
	    continue;
	}
	
	cptr = (char *) &msg.reg[0];

	/* Send an empty message to register */
	if(*cptr == 0) {
	    if(receiver.thread_id == L4_INVALID_ID.thread_id) {
		receiver = sender;
		kprintf( "*** PCI/Serial: Registered receiver 0x%lx\n\r", 
			 receiver.thread_id);
		/* reply to sender with OK */
		msg.reg[0] = 0;
	    } else {
		/* reply to sender with UnOK */
		msg.reg[0] = 1;
	    }

	    msg.reg[1] = thread.thread_id;
	    l4_alpha_ipc_send(sender, L4_IPC_SHORT_MSG, &msg, 
			      L4_IPC_NEVER, &result);
	    continue;
	}

	for( i = 0; i < 64; i++ )
	{
		if( cptr[i] == '\0' )
			break;

		/* do we really want \r translation? -ceg */
		if( cptr[i] == '\r' )
			dev_putc(0, '\n');
		
		dev_putc(0, cptr[i]);

		if( cptr[i] == '\n' )
			dev_putc(0, '\r');
	}
    }
}

