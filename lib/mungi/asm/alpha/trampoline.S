/****************************************************************************
 *
 *      $Id: trampoline.S,v 1.6 2002/07/22 10:18:00 cgray Exp $
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

/* L4/Alpha stub
 * 
 * Notes: On the Alpha, when we return a 128-bit value, it gets allocated space on the stack
 * and manipulated via a pointer.
 * readCap(uint128_t v) - v is passed as arguments
 * uint128_t v buildCap() - v is on the stack, pointer to it passed as an argument	
 */

#include <asm.h>
#include <regdef.h>

	.globl _thread_return
	.globl trampoline

_thread_return:
	// FIXME
	// I've got no idea where to pick a0 and a1 from, they are somewhere on the stack
        ldgp	gp, 0(ra)
	mov	v0, a0
	mov	t0, a1
	lda	pv, thread_return
	jmp	zero, (pv), 0x1
	

trampoline:	
        br      pv, 1f                  /* Set a pv */
1:      ldgp	gp, 0(pv)
        
	ldq	a1, 8(sp)		/* password part of argument */
        ldq	a0, 16(sp)		/* address part of argument */
	ldq	pv, 24(sp)		/* get the new thread IP */
	lda	ra, _thread_return	/* set the return address */

	/* Give PDX space for call */
	subq	sp,120, sp
	
	jmp	zero, (pv), 0x1		/* jump to the IP */
