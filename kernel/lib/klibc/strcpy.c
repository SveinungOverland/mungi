/****************************************************************************
 *
 *      $Id: strcpy.c,v 1.2 2002/05/31 06:14:11 danielp Exp $
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

#include <mungi/klibc.h>

char *strcpy (char *dstp, const char *srcp)
{
    char           *dp = dstp;

    if (!dstp)
	return (0);
    *dp = 0;
    if (!srcp)
	return (dstp);

    while ((*dp++ = *srcp++) != 0);
    return (dstp);
}
