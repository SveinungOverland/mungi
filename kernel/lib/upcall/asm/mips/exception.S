/****************************************************************************
 *
 *      $Id: exception.S,v 1.2 2002/05/31 06:14:31 danielp Exp $
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

/*
 * return from an exception
 */
PROC(_expret)
        .set    noreorder

        .set    reorder
END(_expret)


/*
 * This will do the argument passing to the exception handler
 */
PROC(_expstub)
        .set    noreorder
	daddiu	sp,sp,-192

	sd	a0,(sp)		/* save registers on stack */
	sd	a1,8(sp)
	sd	a2,16(sp)
	sd	a3,24(sp)
	sd	a4,32(sp)
	sd	a5,40(sp)
	sd	a6,48(sp)
	sd	a7,56(sp)
	sd	t0,64(sp)
	sd	t1,72(sp)
	sd	t2,80(sp)
	sd	t3,88(sp)
	sd	t8,96(sp)
	sd	t9,104(sp)
	sd	ra,112(sp)
	sd	gp,120(sp)
	sd	v0,128(sp)
	sd	v1,136(sp)

	ld	a3,160(sp)
	ld	a0,168(sp)
	ld	a1,176(sp)
	la	ra,_expret
	jr	a3
	ld	a2,184(sp)
        .set    reorder
END(_expstub)
