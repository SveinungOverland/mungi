/****************************************************************************
 *
 *      $Id: trampoline.S,v 1.4 2002/05/31 07:51:23 danielp Exp $
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

#include <asm.h>
#include <regdef.h>

PROC(_thread_return)
	.set noreorder
        move a0,v0
        j thread_return
        move a1,v1
	.set reorder
END(_thread_return)

PROC(trampoline)
        .set noreorder
        ld      a1,8(sp)                /* password part of argument */
        ld	a0,16(sp)		/* address part of argument */
        ld      t9,24(sp)               /* get new thread IP */
        la      ra,_thread_return       /* set the return address */
        j       t9                      /* jump to the IP */
        daddiu  sp,sp,24                /* replace the stack pointer */
        .set reorder
END(trampoline)
