/****************************************************************************
 *
 *      $Id: crt0.S,v 1.2 2002/05/31 06:27:44 danielp Exp $
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
#include <pal/palcalls.h>

#define STACKSIZE	4096
		
	.text
	.align	3

        .globl  __start
	
        .ent    __start
 
__start:
	bis	zero, zero, zero
	br	pv, 1f
	
1:	ldgp    gp, 0(pv)		# Need a gp, for bootstrap
	nop
	lda	sp, _stack
	lda	sp, STACKSIZE (sp)
	bis	a0, zero, a1
	bis	sp, zero, a0
	lda     pv, main
	jsr     ra, (pv), main
	
2:	
	.long	UDBG_DEBUG
	br	zero, 2b

	.end	__start


	
	.align	3
	.lcomm	_stack, STACKSIZE
	.lcomm	_stack_end, 8
	
	







