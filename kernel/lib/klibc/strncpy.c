/****************************************************************************
 *
 *      $Id: strncpy.c,v 1.2 2002/05/31 06:14:11 danielp Exp $
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

/** char *strncpy(dst,src,n) copy n chars from src to dst */
char *strncpy(char *dst, const char *src, size_t n)
{
    char           *d;

    if (!dst || !src)
	return (dst);
    d = dst;
    for (; *src && n; d++, src++, n--)
	*d = *src;
    while (n--)
	*d++ = '\0';
    return (dst);
}
