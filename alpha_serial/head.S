/****************************************************************************
 *
 *      $Id: head.S,v 1.2 2002/05/31 05:00:48 danielp Exp $
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

/*
**	Copyright (c) 1995, Sebastian Schoenberg 
**	E-mail : ss10@irz.inf.tu-dresden.de
**
**	University of Technology Dresden, Department of Computer Science
**	Inistute for Operating Systems, Databases and Computer Networks
**
**	$Id: head.S,v 1.2 2002/05/31 05:00:48 danielp Exp $
**
**	$Log: head.S,v $
**	Revision 1.2  2002/05/31 05:00:48  danielp
**	License header. GPL.
**	
**	Revision 1.1  2002/02/05 03:47:34  brettn
**	Mammoth update:
**	Added pager mappings for MIPS clock into pager
**	Removed dodgy PRINT macro in kernel/src/clock.c
**	Split user and kernel clocks.
**	                                                (from andrewb)
**	Added multiple user task support (Makefile and subdir.mk)
**	Moved init.c
**	Console driver update/rewrite for alpha.
**	Fix type of pdx_t function
**	Removed \n => \n\r
**	Fixed upcall UserPrint for mips and alpha
**	Add handling for 'mishell' and 'init' instead of mungiapp
**	Added more capabilities for init.
**	Death of aprintf, moved to kprintf.
**	Add character input to startup.
**	Fstat removed from mungi and added to POSIX.
**	Added new apps and mishell.
**	Added alpha serial driver, posix compatibility library and mlib.
**	                                                (from alexs and cgray)
**	Fixed allocation bug in rpager (will not alloc kernel memory now)
**	Misc rpager cleanups - more needed.
**	                                                (benno, cgray & me)
**	
**	Revision 1.1  2001/03/27 12:10:35  sjw
**	Added some existing code
**	
**	Revision 1.1.1.1  2000/03/22 13:19:11  danielp
**	New L4/Alpha main tree supporting all processors
**	
**	Revision 1.1  1999/11/09 00:49:29  danielp
**	ECDL (code cracking) demo.
**
**	Revision 5.0  1999/03/01 05:27:23  danielp
**	Added tree to CVS, set version to 5.
**	Includes preliminary l4_id_nearest code.
**	Includes UX/Ruffian bug fix.
**
**	Revision 1.1.1.1  1999/03/01 05:09:44  danielp
**	Imported sources
**
**	Revision 4.0  1998/01/12 16:07:19  schoenbg
**	Revision 4.0
**
**	Revision 3.0  1998/01/09 15:02:54  schoenbg
**	PALcall numbers modified
**	New Release
**
**	Revision 1.3  1997/11/07 16:17:19  vu2
**	Memory mapping database
**
**	Revision 1.2  1997/06/10 12:35:56  schoenbg
**	... lots of changes ;-)
**
**	Revision 1.1  1997/04/14 13:24:32  schoenbg
**	Same sample applications
**
**	flexpage : send a flexpage from sigma 0 to an application in response of
**	a pagefault
**
**	idt: create an interrupt table and raise an exception
**
**	pingpong: create two threads playing ping pong using short messages
**
**	simple: ... hello world on l4 alpha
**
**	Revision 1.1.1.1  1997/01/13 15:56:48  schoenbg
**	L4 for ALpha 21164
**	
**
*/


#include <regdef.h>
#include <pal/palcalls.h>

	.text
	.align	3

        .globl  start
	
        .ent    start
 
start:
	bis	zero, zero, zero
	br	pv, 1f
	
1:	ldgp    gp, 0(pv)		# Need a gp, for bootstrap
	nop
	lda	sp, _sp
	ldq	sp, 0(sp)
	bis	a0, zero, a1
	bis	sp, zero, a0
	lda     pv, main
	jsr     ra, (pv), main
	
2:	
	.long	UDBG_DEBUG
	br	zero, 2b

	.end	start
	
	







