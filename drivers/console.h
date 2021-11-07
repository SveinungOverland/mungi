/****************************************************************************
 *
 *      $Id: console.h,v 1.8 2002/08/21 06:10:55 luked Exp $
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

/* console.h */

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "mungi/types.h"
#include "mungi/l4_generic.h"

/*
 * This function initialises the destination for
 * printf messages
 */
void console_init(l4_threadid_t);
void devices_init(void);
void start_console_in(void);
 
void console_out(const char *, size_t);
int console_setreader(l4_threadid_t);
 
#endif // __CONSOLE_H




