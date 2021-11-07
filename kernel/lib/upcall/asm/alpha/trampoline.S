/****************************************************************************
 *
 *      $Id: trampoline.S,v 1.8 2002/07/22 10:17:45 cgray Exp $
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

#include <l4/../../pal/l4pal.h>
	
	.globl _thread_return
	.globl initstart
	.globl trampoline


_thread_return:	
	// FIXME - look at other trampoline.S
	ldgp	gp, 0(ra) // Altered by ceg from 0(pv)
	mov	v0, a0
	mov	t0, a1
	lda	pv, thread_return
	jmp	zero, (pv), 0x1

	
initstart:
        br	pv, 1f	                /* get pv */
1:	ldgp	gp, 0(pv)
	ldq	t0, 32(sp)		/* get new stack pointer */
	ldq	pv, 24(sp)		/* get new thread IP */
	ldq	a0, 16(sp)		/* address part of argument */
        ldq	a1, 8(sp)		/* password part of argument */
	lda	ra, _thread_return	/* set the return address */
	mov	t0, sp			/* replace the stack pointer */

	/* Allow space for PDX calling data */
	subq	sp,120, sp		/* replace the stack pointer */

	jmp	zero, (pv), 0x1

	
trampoline:	
        br	pv, 1f			/* Get pv */
1:      ldgp	gp, 0(pv)
        ldq	pv, 24(sp)		/* get the new thread IP */
        ldq     a1, 8(sp)               /* password part of argument */
        ldq     a0, 16(sp)              /* address part of argument */
	lda	ra, _thread_return	/* set the return address */

	/* Allow space for PDX calling data */
	subq	sp,120, sp		/* replace the stack pointer */
		
	jmp	zero, (pv), 0x1		/* jump to the IP */
