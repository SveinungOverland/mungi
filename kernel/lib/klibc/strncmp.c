/****************************************************************************
 *
 *      $Id: strncmp.c,v 1.5 2002/05/31 06:14:11 danielp Exp $
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

#include "mungi/kernel.h"

/* simple unoptimised string compare */


int
strcmp(const char *s1, const char *s2)
{
        for (;*s1 == *s2; s1++,s2++)
                if (!*s1 || !*s2)
                        break;

        return ((unsigned char)*s1 - (unsigned char)*s2);
}



#if 0
int
strncmp(const char *s1, const char *s2, size_t n)
{
        for (;(n>0) && (*s1 == *s2); s1++,s2++,n--)
                if (!*s1 || !*s2)
                        break;

        return ((unsigned char)*s1 - (unsigned char)*s2);
}

#else
int
strncmp(const char *s1, const char *s2, size_t n)
{
        for (;(n>0) && (*s1 == *s2); s1++,s2++,n--)
	    if (*s1 == 0)
		return 0;

	if (n)
	    return ((unsigned char)*s1 - (unsigned char)*s2);
	return 0;
}
#endif

