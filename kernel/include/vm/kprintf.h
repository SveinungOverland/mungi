/****************************************************************************
 *
 *      $Id: kprintf.h,v 1.3 2002/05/31 05:49:43 danielp Exp $
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

/* Kernel printf (usable only by OS task) */

#ifndef _KPRINTF_H
#define _KPRINTF_H
/* so startup can listen for input */
extern const l4_threadid_t SERIAL_TID1;

int kprintf(const char *fmt, ...);

#endif
