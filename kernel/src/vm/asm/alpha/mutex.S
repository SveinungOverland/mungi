/****************************************************************************
 *
 *      $Id: mutex.S,v 1.2 2002/05/31 06:28:15 danielp Exp $
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

	.text
	.align 3
	.globl	testandset
	.ent	testandset
testandset:
1:	ldq_l	v0, 0(a0)		# a0 contains address
	bne	v0, 1b			# seedy busy wait until *addr == 0
	lda	v0, 0x1(zero)
	stq_c	v0, 0(a0)
	beq	v0, 1b
	ret	zero, (ra)

	.end	testandset

	
	.align 3
	.globl	bittestandset
	.ent	bittestandset	
bittestandset:
1:	ldq_l	a2, 0(a0)
	and	a2, a1, v0
	bne	v0, 1b			# busy wait until *addr == 0
	or	a2, a1, v0
	stq_c	v0, 0(a0)
	beq	v0, 1b
	ret	zero, (ra)
	
	.end	bittestandset
