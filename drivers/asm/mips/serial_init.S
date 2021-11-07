/****************************************************************************
 *
 *      $Id: serial_init.S,v 1.2 2002/05/31 05:10:16 danielp Exp $
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
#include <kernel/indy.h>
#include <r4kc0.h>

PROC(reset_serial_io)
	/* assume called from panic and can trash s0 */
	move	s0, ra

	/* disable interrupts */
	li	a0, 9
	li	a1, 0
	jal	wrreg
	li	a0, 1
	li	a1, 0
	jal	wrreg
	
	jr	s0
END(reset_serial_io)
	

PROC(wrreg)
	move	t8, ra
	dli	t0, HPC3_SERIAL1_CMD | 3
	dli	t1, HPC3_SERIAL1_DATA | 3
	dli	t2, HPC3_INTSTAT_ADDR
	jal	dl
	sb	a0, (t0)
	sync
	lw	zero, (t2)
	jal	dl
	sb	a1, (t0)
	sync
	lw	zero, (t2)
	jr	t8
END(wrreg)

PROC(rdreg)
	move	t8, ra
	dli	t0, HPC3_SERIAL1_CMD | 3
	dli	t1, HPC3_SERIAL1_DATA | 3
	dli	t2, HPC3_INTSTAT_ADDR
	jal	dl
	sb	a0, (t0)
	sync
	lw	zero, (t2)
	jal	dl
	lbu	v0, (t0)
	jr	t8
END(rdreg)

PROC(rdzero)
	move	t8, ra
	dli	t0, HPC3_SERIAL1_CMD | 3
	jal	dl
	lbu	v0, (t0)
	jr	t8
END(rdzero)

PROC(dl)
	dli	t9, 100
	
1:	daddiu	t9, t9, -1
	bne	t9, zero, 1b

	
	jr	ra
END(dl)
