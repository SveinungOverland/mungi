/****************************************************************************
 *
 *      $Id: strichr.c,v 1.3 2002/05/31 07:44:21 danielp Exp $
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

#include "compat.h"

char *strichr (char *p, int c);

/** char *strichr(p,c) inserts c as the first char of the string p */
char *strichr (char *p, int c)
{
    char           *t;

    if (!p)
	return (p);
    for (t = p; *t; t++);
    for (; t >= p; t--)
	*(t + 1) = *t;
    *p = c;
    return (p);
}
