/****************************************************************************
 *
 *      $Id: kprintf.c,v 1.5 2002/05/31 06:27:57 danielp Exp $
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

/* Kernel printf (usable only by OS task) */

#include <assert.h>
#include "mungi/l4_generic.h"

#include <mungi/stdarg.h>

#include <vm/kprintf.h>
#include <vm/serial.h>
#include <vm/mutex.h>

#ifdef MIPSENV
const l4_threadid_t SERIAL_TID1 = {0x1002000000060001};
#else
const l4_threadid_t SERIAL_TID1 = {0x802000000180001};
#endif

extern l4_threadid_t serialid;

union kbuf
{
  uintptr_t align; /* ensure start of buffer is double word aligned */
  char buf[256];
} kprintbuf;

static int mutex=0;

int vsprintf(char *str, const char *format, va_list ap);

int 
kprintf(const char *fmt, ...)
{ /* Note: This will only print strings <=64b correctly! */
        va_list ap;
        int len, r;
        l4_msgdope_t result;

        testandset(&mutex);
        va_start(ap, fmt);
        len = vsprintf(kprintbuf.buf, fmt, ap);
        va_end(ap);
  
        r = l4_ipc_send(SERIAL_TID1, L4_IPC_SHORT_MSG,
		       (l4_ipc_reg_msg_t *)(void *) kprintbuf.buf,
                        L4_IPC_NEVER, &result);
        mutex=0;
        assert(r == 0);
        return len;
}
