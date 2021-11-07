/****************************************************************************
 *
 *      $Id: lcall.S,v 1.2 2002/05/31 06:14:31 danielp Exp $
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

/****************************************************************************
 *
 *     Jump to a specified memory location
 *     Load all the args into the args regs and then goto address
 *
 ****************************************************************************/

PROC(_lcall)
        .set    noreorder
	ld	a0,(sp)
	ld	a1,-16(sp)
	ld	a2,-24(sp)
	ld	a3,-32(sp)
	ld	a4,-40(sp)
	ld	a5,-48(sp)
	ld	a6,-56(sp)
	daddiu	sp,sp,-8
        j	a0
        ld	a0,(sp)
        .set    reorder
END(_lcall)
