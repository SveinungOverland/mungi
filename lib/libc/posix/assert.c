/****************************************************************************
 *
 *      $Id: assert.c,v 1.6 2002/07/22 10:17:54 cgray Exp $
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

#ifdef ALPHAENV

#include <l4/types.h>
#include <l4/assert.h>
#include <l4/kdebug.h>

void printf(const char *, ...) __attribute__ ((format (printf, 1, 2)));
void UserPrint( const char *, ... );

void 
__assert(const char *msg, const char *file, int line) 
{
	/* Does nothing special at the moment except enter the debugger.
	 * Would be nice to save register state perhaps..
	 */
	/* Use UserPrint to avoid assertion problems in printf() */
	UserPrint("ASSERT: %s, %s:%d\n\r", msg, file, line);
    
	enter_kdebug();
}

#endif /* ALPHAENV */
