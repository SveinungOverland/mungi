/****************************************************************************
 *
 *      $Id: memset.S,v 1.3 2002/05/31 06:14:20 danielp Exp $
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

#include <mips/mips.h>

#if __mips >= 3
#define S	sd
#define SL	sdl
#define SR	sdr
#define RS	8
#else
#define S	sw
#define SL	swl
#define SR	swr
#define RS	4
#endif

#ifdef MIPSEL
#	define	SHI	SR
#	define	SLO	SL
#endif
#ifdef MIPSEB
#	define	SHI	SL
#	define	SLO	SR
#endif

/* memset(to, val, n) */
	.globl	memset
	.ent	memset
memset_asm:
	.set	noreorder
	move	v0, a0			# save to for return
	beq	a2, zero, .ret
	sltu	t2, a2, 16
	bne	t2, zero, .small	# do small blocks byte at a time
	
	/* replicate fill byte into register */
	and	a1, 0xff
#if __mips >= 3	
	dsll	t2, a1, 8
	or	a1, t2
	dsll	t2, a1, 16
	or	a1, t2
	dsll	t2, a1, 32
	or	a1, t2
#else	
	sll	t2, a1, 8
	or	a1, t2
	sll	t2, a1, 16
	or	a1, t2
#endif	
	
	and	v1, a0, RS-1		# get unaligned address
	beq	v1, zero, 1f		# skip if already aligned
	li	a3, RS			# calculate...
	subu	a3, v1			#  number of bytes to align
	
	subu	a2, a3			# subtract from remaining count
	SHI	a1, 0(a0)		# store 1..RS bytes to align a0
	addu	a0, a3			# bump pointer

	/* Try a 4X unrolled block fill */
1:	and	v1, a2, (RS*4)-1	# remaining size % blocksize
	subu	a3, a2, v1		# size of remaining blocks
	beq	a3, zero, 1f		# none?
	move	a2, v1			# bytes remaining after block copy
	addu	a3, a0			# compute ending address

2:	S	a1, RS*0(a0)
	S	a1, RS*1(a0)
	S	a1, RS*2(a0)
	addu	a0, RS*4
	bne	a0, a3, 2b
	S	a1, -RS(a0)

	/* Try a word at a time */
1:	and	v1, a2, RS-1		# remaining size % word size
	subu	a3, a2, v1		# size of remaining words
	beq	a3, zero, .small	# none?
	move	a2, v1			# bytes remaining after word copy
	addu	a3, a0			# compute ending address

2:	addu	a0, RS
	bne	a0, a3, 2b
	S	a1, -RS(a0)

.small:
	/* Last resort: byte at a time */
	beq	a2, zero, .ret
	addu	a3, a2, a0		# compute ending address

1:	addu	a0, 1
	bne	a0, a3, 1b
	sb	a1, -1(a0)

.ret:	j	ra
	nop
	.set	reorder

	.end	memset
