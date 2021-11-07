/****************************************************************************
 *
 *      $Id: trampoline.S,v 1.7 2002/07/22 10:17:46 cgray Exp $
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


PROC(initstart)
	.set noreorder
        ld      a1,8(sp)                /* password part of argument */
	ld	a0,16(sp)		/* address part of argument */
	ld	t9,24(sp)		/* get new thread IP */
	ld	t8,32(sp)		/* get new stack pointer */
        la	ra,_thread_return	/* set the return address */

	/* Allow space for PDX call data */
	daddiu sp, sp, -64
	
	j	t9			/* jump to the IP */
	move	sp,t8			/* replace the stack pointer */
	.set reorder
END(initstart)

PROC(trampoline)
        .set noreorder
        ld      a1,8(sp)                /* load the argument from the stack */
        ld      a0,16(sp)               /* second part of cap */
        ld      t9,24(sp)               /* get new thread IP */
        la      ra,_thread_return       /* set the return address */

	/* Allow space for PDX call data */
	daddiu sp, sp, -64
	
        j       t9                      /* jump to the IP */
        daddiu  sp,sp,24                /* replace the stack pointer */
        .set reorder
END(trampoline)
