/****************************************************************************
 *
 *      $Id: serial_reg.c,v 1.3 2002/05/31 05:00:49 danielp Exp $
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
 *        Project:  L4/Serial
 *        Created:  28/04/2000 17:18:47 by Simon Winwood (sjw)
 *  Last Modified:  06/12/2000 12:19:45 by Simon Winwood (sjw)
 *   Version info:  $Revision: 1.3 $ 
 *    Description:
 *          This file contains the device specific part for the SuperIO III (PC87312)
 *       chip found in Multia and Ruffian (PC164UX).
 *
 *       Comments:
 *
 *          See National Semiconductor documentation for PC87311A/PC87312
 *
 * $Log: serial_reg.c,v $
 * Revision 1.3  2002/05/31 05:00:49  danielp
 * License header. GPL.
 *
 * Revision 1.2  2002/04/09 07:31:24  brettn
 * Removed MSDS
 * Fixed page touching in pagers.
 * rpager now asserts before it dereferences the pointer.
 * Added -Werror to standard flags (yeah!).  All code must obey this from
 *         now...
 * Thousands of cleanups to make it compile... (cosmetic changes and casts
 *         only, no significant code changes)
 * Fixed (the award-winning) getenv.c
 *
 * Revision 1.1  2002/02/05 03:47:35  brettn
 * Mammoth update:
 * Added pager mappings for MIPS clock into pager
 * Removed dodgy PRINT macro in kernel/src/clock.c
 * Split user and kernel clocks.
 *                                                 (from andrewb)
 * Added multiple user task support (Makefile and subdir.mk)
 * Moved init.c
 * Console driver update/rewrite for alpha.
 * Fix type of pdx_t function
 * Removed \n => \n\r
 * Fixed upcall UserPrint for mips and alpha
 * Add handling for 'mishell' and 'init' instead of mungiapp
 * Added more capabilities for init.
 * Death of aprintf, moved to kprintf.
 * Add character input to startup.
 * Fstat removed from mungi and added to POSIX.
 * Added new apps and mishell.
 * Added alpha serial driver, posix compatibility library and mlib.
 *                                                 (from alexs and cgray)
 * Fixed allocation bug in rpager (will not alloc kernel memory now)
 * Misc rpager cleanups - more needed.
 *                                                 (benno, cgray & me)
 *
 * Revision 1.1  2001/03/27 12:10:35  sjw
 * Added some existing code
 *
 */

#include <l4/types.h>
#include <klibc.h>
#include <l4/assert.h>
#include "io.h"
#include "serial_reg.h"
#include "serial.h"

#define SIO_INDEX_PORT_0      0x398
#define SIO_DATA_PORT_0      0x399
#define SIO_INDEX_PORT_1       0x26E 
#define SIO_DATA_PORT_1       0x26F

/* With the SuperIO chip, these can be 1 of 2 addresses */
static int index_port;
static int data_port;

#define N_UARTS               2

static int uart_port[N_UARTS] = {0x3f8, 0x2f8};

int dev_init()
{
    index_port = SIO_INDEX_PORT_1;
    data_port = SIO_DATA_PORT_1;

    inb(uart_port[0] + UART_LSR);
    inb(uart_port[0] + UART_RX);
    inb(uart_port[0] + UART_IIR);
    inb(uart_port[0] + UART_MSR);

    /* FIXME (sjw Thu Aug 24 00:09:06 2000 ) --- Only enable ints on COMA */
#if 0
    /* Generic UART stuff */
    for(i = 0; i < N_UARTS; i++) { 
	/* Enable receive interrupts */
	outb(UART_IER_RDI, uart_port[i] + UART_IER);
    }
#endif

    outb(UART_IER_RDI, uart_port[0] + UART_IER);

    return 0;
}

void dev_putc(int uart, char c)
{
    while((inb(uart_port[uart] + UART_LSR) & UART_LSR_THRE) == 0) {}
    outb(c, uart_port[uart] + UART_TX);
}

int dev_getc(int uart)
{
    /* Wait for data to be ready */
    while((inb(uart_port[uart] + UART_LSR) & UART_LSR_DR) == 0) {}
    return inb(uart_port[uart] + UART_RX);
}


