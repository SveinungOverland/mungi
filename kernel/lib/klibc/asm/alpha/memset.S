/****************************************************************************
 *
 *      $Id: memset.S,v 1.2 2002/05/31 06:14:15 danielp Exp $
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

/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   Contributed by Richard Henderson (rth@tamu.edu)
   Addapted for L4/Alpha/Mungi by Daniel Potts (danielp@cse.unsw.edu.au)
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Fill a block of memory with a character.  Optimized for the Alpha
   architecture:

   - memory accessed as aligned quadwords only
   - destination memory not read unless needed for good cache behaviour
   - basic blocks arranged to optimize branch prediction for full-quadword
     aligned memory blocks.
   - partial head and tail quadwords constructed with byte-mask instructions

   This is generally scheduled for the EV5 (got to look out for my own
   interests :-), but with EV4 needs in mind.  There *should* be no more
   stalls for the EV4 than there are for the EV5.
*/


//#include <sysdep.h>

# include <asm.h>
# include <regdef.h>
	
	.set noat
	.set noreorder

	.text

/* There is a problem with either gdb (as of 4.16) or gas (as of 2.7) that
   doesn't like putting the entry point for a procedure somewhere in the
   middle of the procedure descriptor.  Work around this by putting the main
   loop in its own procedure descriptor.  */

	/* On entry to this basic block:
	   t3 == loop counter
	   t4 == bytes in partial final word
	   a0 == possibly misaligned destination pointer
	   a1 == replicated source character  */

	.ent memset_loop
	.align 3
memset_loop:
	.frame sp, 0, ra, 0
	.prologue 0

	beq	t3, $tail	#
	blbc	t3, 10f		# skip single store if count even

	stq_u	a1, 0(a0)	# e0    : store one word
	subq	t3, 1, t3	# .. e1 :
	addq	a0, 8, a0	# e0    :
	beq	t3, $tail	# .. e1 :

10:	stq_u	a1, 0(a0)	# e0    : store two words
	subq	t3, 2, t3	# .. e1 :
	stq_u	a1, 8(a0)	# e0    :
	addq	a0, 16, a0	# .. e1 :
	bne	t3, 10b		# e1    :

$tail:	bne	t4, 1f		# is there a tail to do?
	ret			# no

	.align 3
1:	ldq_u	t0, 0(a0)	# e1    : yes, load original data
	mskql	a1, t4, t1	# .. e0 :
	mskqh	t0, t4, t0	# e0    :
	or	t0, t1, t0	# e1 (stall)
	stq_u	t0, 0(a0)	# e0    :
	ret			# .. e1 :

	.end memset_loop

	.globl memset
	.align 3
	.ent memset, 0			
memset:
	.frame sp, 0, ra
#ifdef PROF
	ldgp	gp, 0(pv)
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.prologue 1
#else
	.prologue 0
#endif

	zapnot	a1, 1, a1	# e0    : zero extend input character
	mov	a0, v0		# .. e1 : move return value in place
	sll	a1, 8, t0	# e0    : begin replicating the char
	beq	a2, $done	# .. e1 : early exit for zero-length store
	or	t0, a1, a1	# e0    :
	and	a0, 7, t1	# .. e1 : dest misalignment
	sll	a1, 16, t0	# e0    :
	addq	a2, t1, a2	# .. e1 : add dest misalignment to count
	or	t0, a1, a1	# e0    :
	srl	a2, 3, t3	# .. e1 : loop = count >> 3
	sll	a1, 32, t0	# e0    :
	and	a2, 7, t4	# .. e1 : find number of bytes in tail
	or	t0, a1, a1	# e0    : character replication done

	beq	t1, memset_loop	# .. e1 : aligned head, jump right in

	ldq_u	t0, 0(a0)	# e1    : load original data to mask into
	mskqh	a1, a0, t1	# .. e0 :

	cmpult	a2, 8, t2	# e0    : is this a sub-word set?
	bne	t2, $oneq	# .. e1 (zdb)

	mskql	t0, a0, t0	# e0    : we span words.  finish this partial
	subq	t3, 1, t3	# .. e1 :
	addq	a0, 8, a0	# e0    :
	or	t0, t1, t0	# .. e1 :
	stq_u	t0, -8(a0)	# e0    :
	br 	memset_loop	# .. e1 :

	.align 3
$oneq:
	mskql	t1, a2, t1	# e0    : entire operation within one word
	mskql	t0, a0, t2	# e0    :
	mskqh	t0, a2, t3	# e0    :
	or	t1, t2, t0	# .. e1 :
	or	t0, t3, t0	# e1    :
	stq_u	t0, 0(a0)	# e0 (stall)

$done:	ret

	.end memset
