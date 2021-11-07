/****************************************************************************
 *
 *      $Id: timing.h,v 1.2 2002/05/31 05:49:38 danielp Exp $
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

#ifndef __M_TIMING_H_
#define __M_TIMING_H_

#include "mungi/l4_generic.h"

/* L4 timeouts initialised in l4_generic.c */

extern l4_timeout_t upcall_timeout;
extern l4_timeout_t syscall_timeout;
extern l4_timeout_t pagefault_timeout;

#endif /* __M_TIMING_H_ */
