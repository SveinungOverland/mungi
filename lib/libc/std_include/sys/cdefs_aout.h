/****************************************************************************
 *
 *      $Id: cdefs_aout.h,v 1.2 2002/05/31 07:44:10 danielp Exp $
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

/*	$NetBSD: cdefs_aout.h,v 1.1 1999/03/20 01:39:22 thorpej Exp $	*/

/*
 * Written by J.T. Conklin <jtc@wimsey.com> 01/17/95.
 * Public domain.
 */

#ifndef _SYS_CDEFS_AOUT_H_
#define	_SYS_CDEFS_AOUT_H_

#define	_C_LABEL(x)	__CONCAT(_,x)

#ifdef __GNUC__
#ifdef __STDC__
#define	__indr_reference(sym,alias)					\
	__asm__(".stabs \"_" #alias "\",11,0,0,0");			\
	__asm__(".stabs \"_" #sym "\",1,0,0,0");

#define	__warn_references(sym,msg)					\
	__asm__(".stabs \"" msg "\",30,0,0,0");				\
	__asm__(".stabs \"_" #sym "\",1,0,0,0");
#else /* __STDC__ */
#define	__indr_reference(sym,alias)					\
	__asm__(".stabs \"_/**/alias\",11,0,0,0");			\
	__asm__(".stabs \"_/**/sym\",1,0,0,0");

#define	__warn_references(sym,msg)					\
	__asm__(".stabs msg,30,0,0,0");					\
	__asm__(".stabs \"_/**/sym\",1,0,0,0");
#endif /* __STDC__ */
#else /* __GNUC__ */
#define	__warn_references(sym,msg)
#endif /* __GNUC__ */

#define __IDSTRING(name,string)						\
	static const char name[] __attribute__((__unused__)) = string

#define __RCSID(_s)	__IDSTRING(rcsid,_s)
#define __COPYRIGHT(_s)	__IDSTRING(copyright,_s)

#define	__KERNEL_RCSID(_n, _s) __IDSTRING(__CONCAT(rcsid,_n),_s)
#define	__KERNEL_COPYRIGHT(_n, _s) __IDSTRING(__CONCAT(copyright,_n),_s)

#endif /* !_SYS_CDEFS_AOUT_H_ */
