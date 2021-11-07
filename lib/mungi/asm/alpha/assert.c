/****************************************************************************
 *
 *      $Id: assert.c,v 1.2 2002/05/31 07:51:21 danielp Exp $
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
 * $Id: assert.c,v 1.2 2002/05/31 07:51:21 danielp Exp $
 *
 * Defines useful assert() function that can be used to invoke the debugger.
 */

#include <l4/types.h>
#include <l4/assert.h>
#include <l4/kdebug.h>

#include <mungi/klibc.h>

void printf(const char *, ...) __attribute__ ((format (printf, 1, 2)));

void __assert(const char *msg, const char *file, int line) {
  /* Does nothing special at the moment except enter the debugger.
   * Would be nice to save register state perhaps..
   */
    printf("ASSERT: %s, %s:%d\n\r", msg, file, line);
    
    enter_kdebug();
}
