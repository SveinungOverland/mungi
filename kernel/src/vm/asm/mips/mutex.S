/****************************************************************************
 *
 *      $Id: mutex.S,v 1.2 2002/05/31 06:28:17 danielp Exp $
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

#include <regdef.h>
#include <asm.h>

PROC(testandset)
1:      ll      v0, (a0)        /* a0 contains addr */
        bne     v0, zero, 1b    /* busy wait until *addr = 0 */
        addiu   v0, v0, 1
        sc      v0, (a0)        /* set *addr = 1 */
        beq     v0, zero, 1b    /* do whole thing again if sc fails */
        jr      ra              /* return from function call */
END(testandset)


PROC(bittestandset)
1:	lld	a2, (a0)        /* a0 contains addr */
	and	v0, a2, a1  
	bne	v0, zero, 1b    /* busy wait until *addr = 0 */
	or	v0, a2, a1
	scd	v0, (a0)        /* set *addr = 1 */
	beq	v0, zero, 1b    /* do whole thing again if sc fails */
	jr	ra              /* return from function call */
END(bittestandset)

PROC(bittestandset32)
1:	ll	a2, (a0)        /* a0 contains addr */
	and	v0, a2, a1  
	bne	v0, zero, 1b    /* busy wait until *addr = 0 */
	or	v0, a2, a1
	sc	v0, (a0)        /* set *addr = 1 */
	beq	v0, zero, 1b    /* do whole thing again if sc fails */
	jr	ra              /* return from function call */
END(bittestandset)
	
