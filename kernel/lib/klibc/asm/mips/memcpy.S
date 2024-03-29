/****************************************************************************
 *
 *      $Id: memcpy.S,v 1.3 2002/05/31 06:14:20 danielp Exp $
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

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "mips/mips.h"


#if __mips >= 3
#define L	ld
#define LL	ldl
#define LR	ldr
#define S	sd
#define SL	sdl
#define SR	sdr
#define RS	8
#else
#define L	lw
#define LL	lwl
#define LR	lwr
#define S	sw
#define SL	swl
#define SR	swr
#define RS	4
#endif

#ifdef MIPSEL
#	define	LHI	LR
#	define	LLO	LL
#	define	SHI	SR
#	define	SLO	SL
#endif
#ifdef MIPSEB
#	define	LHI	LL
#	define	LLO	LR
#	define	SHI	SL
#	define	SLO	SR
#endif

/* nash:	 FIXME:
		The Function formerlly known as memcpy.
		This generates an exception when called.
		Generally the address is something like:
			Cause is 0x10
			EPC is 0x6b2dc
			BVA is 0xfffffffffc412019
		[10 -> Load exeception, PC is this function.
	*/

	
	
/* asm_memcpy(to, from, n) */
	.globl	memcpy
	.ent	memcpy
asm_memcpy:
	.set	noreorder
	move	v0,a0			# save to for return
	beq	a2, zero, .ret
	sltu	t2, a2, 12		# check for small copy

#if 0
/* memcpy() doesn't have to handle overlap! */	
	addu	t0, a1, a2		# t0 = end of from region
	sltu	t1, a0, t0
	sltu	t2, a1, a0
	and	t1, t2			# t1 = true if from < to < (from+len)
	beq	t1, zero, .forward	# non overlapping, do forward copy
	addu	t1, a0, a2		# t1 = end of to region

1:	lb	v1, -1(t0)		# copy bytes backwards,
	subu	t0, 1			#   doesnt happen often so do slow way
	subu	t1, 1
	bne	t0, a1, 1b
	sb	v1, 0(t1)

	b	.ret
	nop
#endif

.forward:
	bne	t2, zero, .smallcpy	# do a small bcopy
	xor	v1, a1, a0		# compare low bits of addresses
	and	v1, RS-1
	subu	a3, zero, a0		# compute # bytes to word align address
	beq	v1, zero, .aligned	# addresses can both be word aligned
	and	a3, RS-1

	beq	a3, zero, 1f
	subu	a2, a3			# subtract from remaining count

	LHI	v1, 0(a1)		# get next RS bytes (unaligned)
	LLO	v1, RS-1(a1)
	addu	a1, a3
	SHI	v1, 0(a0)		# store 0..RS-1 bytes to align a0
	addu	a0, a3

	/* Try a 4X unrolled unaligned block copy */
1:	and	v1, a2, (RS*4)-1	# remaining size % blocksize
	subu	a3, a2, v1		# size of remaining blocks
	beq	a3, zero, 1f		# none?
	move	a2, v1			# bytes remaining after block copy
	addu	a3, a1			# compute ending address

2:	LHI	v1, RS*0(a1)		# copy block a1 unaligned, a0 aligned
	LLO	v1, RS*0+RS-1(a1)
	LHI	t0, RS*1(a1)
	LLO	t0, RS*1+RS-1(a1)
	LHI	t1, RS*2(a1)
	LLO	t1, RS*2+RS-1(a1)
	LHI	t2, RS*3(a1)
	LLO	t2, RS*3+RS-1(a1)
	S	v1, RS*0(a0)
	S	t0, RS*1(a0)
	S	t1, RS*2(a0)
	addu	a1, RS*4
	addu	a0, RS*4
	bne	a1, a3, 2b
	S	t2, -RS(a0)
	
1:	and	v1, a2, RS-1		# compute number of words left
	subu	a3, a2, v1		
	beq	a3, zero, .smallcpy	# none?
	move	a2, v1			# bytes remaining after word copy
	addu	a3, a1			# compute ending address

2:	LHI	v1, 0(a1)		# copy words a1 unaligned, a0 aligned
	LLO	v1, RS-1(a1)
	addu	a1, RS
	addu	a0, RS
	bne	a1, a3, 2b
	S	v1, -RS(a0)
	
	b	.smallcpy
	nop

.aligned:
	/* Both addresses have the same alignment: do initial bytes to align */
	beq	a3, zero, 1f
	subu	a2, a3			# subtract from remaining count
	LHI	v1, 0(a1)		# copy 1, 2, or 3 bytes to align
	addu	a1,  a3
	SHI	v1, 0(a0)
	addu	a0, a3

	/* Try a 4X unrolled block copy */
1:	and	v1, a2, (RS*4)-1	# remaining size % blocksize
	subu	a3, a2, v1		# size of remaining blocks
	beq	a3, zero, 1f		# none?
	move	a2, v1			# bytes remaining after block copy
	addu	a3, a1			# compute ending address

2:	L	v1, RS*0(a1)
	L	t0, RS*1(a1)
	L	t1, RS*2(a1)
	L	t2, RS*3(a1)
	S	v1, RS*0(a0)
	S	t0, RS*1(a0)
	S	t1, RS*2(a0)
	addu	a1, RS*4
	addu	a0, RS*4
	bne	a1, a3, 2b
	S	t2, -RS(a0)

	/* Try a word at a time */
1:	and	v1, a2, RS-1		# remaining size % word size
	subu	a3, a2, v1		# size of remaining words
	beq	a3, zero, .smallcpy	# none?
	move	a2, v1			# bytes remaining after word copy
	addu	a3, a1			# compute ending address

2:	L	v1, 0(a1)		# copy words
	addu	a1, RS
	addu	a0, RS
	bne	a1, a3, 2b
	S	v1, -RS(a0)

.smallcpy:
	/* Last resort: byte at a time */
	beq	a2, zero, .ret
	addu	a3, a2, a1		# compute ending address

1:	lbu	v1, 0(a1)		# copy bytes
	addu	a1, 1
	addu	a0, 1
	bne	a1, a3, 1b
	sb	v1, -1(a0)

.ret:	j	ra
	nop
	.set	reorder
	
	.end	memcpy
