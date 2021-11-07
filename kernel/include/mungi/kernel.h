/****************************************************************************
 *
 *      $Id: kernel.h,v 1.9 2002/07/22 10:17:38 cgray Exp $
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

/* kernel.h - this header file should be included by every kernel C file*/

#ifndef __MUNGI_K_KERNEL_H
#define __MUNGI_K_KERNEL_H

#include "mungi/types.h"
#include "mungi/klibc.h"
#include "stddef.h"
#include "stdbool.h"
#include <assert.h>

#ifdef _VERBOSE_
#define VERBOSE(format, args...) kprintf(format, ##args)
#else 
#define VERBOSE(format, args...)
#endif

#ifdef _DEBUG_
#define DEBUG(format, args...) kprintf(format, ##args)
#else 
#define DEBUG(format, args...)
#endif

#define PAGESIZE	((uintptr_t)L4_PAGESIZE)
#define PAGEMASK	(PAGESIZE - 1)
#define PAGEOFFSETMASK	(~PAGEMASK)

void panic(const char *, ...);

#define page_round_up(address) ((void*)(((uintptr_t)(address) + PAGEMASK) \
                                & PAGEOFFSETMASK))
#define page_round_down(address) ((void*)((uintptr_t)(address) & \
					  PAGEOFFSETMASK))

#define max(x,y) (((x)>(y)) ? (x) : (y))
#define min(x,y) (((x)<(y)) ? (x) : (y))

#endif /* __MUNGI_K_KERNEL_H */
