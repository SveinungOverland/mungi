/****************************************************************************
 *
 *      $Id: setjmp.h,v 1.2 2002/05/31 05:49:18 danielp Exp $
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


#ifndef _SETJMP_
#define _SETJMP_

/* defines for longjmp buffer */
#define JB_S0		0
#define JB_S1		1
#define JB_S2		2
#define JB_S3		3
#define JB_S4		4
#define JB_S5		5
#define JB_S6		6
#define JB_S7		7
#define JB_FP		8
#define JB_SP		9
#define JB_RA		10
#define JB_SIZ		11

#ifdef LANGUAGE_C
#if __mips >= 3
typedef long long jmp_buf[JB_SIZ];
#else
typedef int jmp_buf[JB_SIZ];
#endif
#endif

#endif /* _SETJMP_ */
