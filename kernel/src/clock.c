/****************************************************************************
 *
 *      $Id: clock.c,v 1.5 2002/07/22 10:17:47 cgray Exp $
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
 * simple clock driver for mungi profiling
 * provides clock_start() and clock_stop() functions for kernel code
 *
 * MIPS:
 * uses the 50MHz timer of the GT chip
 * GT device mapping taken from mpager (for user threads) or rpager in kernel
 * requires extra code in mpager to handle the device mapping
 *
 * ALPHA:
 * uses the builtin CPU clock via the rpcc instruction
 * the kernel side init sets up an object with a known address and password
 * that is used to store the start value of the timer (for cross-domain timing)
 * depends on read_pcc in kernel/src/asm/alpha/clock.S
 */

#include "mungi/kernel.h"
#include "mungi/objects.h"
#include "mungi/mm.h"
#include "mungi/clock.h"
#include <l4_generic.h>


/* this is the same as the user-land clock.c */
#define KERNEL_CLOCK

#include "../../lib/mlib/clock.c"
