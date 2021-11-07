/****************************************************************************
 *
 *      $Id: c_memcpy.c,v 1.3 2002/07/22 10:17:43 cgray Exp $
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

#include <sys/types.h>


/* This exists as the mips asm version seesm to have
   some alignment problems.  Cause unknown */
#ifdef MIPSENV

void *memcpy(void *dest,const void *src,size_t n);
void *memset(void *dest,int set,size_t n);

void *memcpy(void *dest,const void *src,size_t n){
	while(n --)
		((char *)dest)[n] = ((char *)src)[n];
	return dest;
}

void *memset(void *dest,int set,size_t n){
	while(n --)
		((char *)dest)[n] = (unsigned char)set;
	return dest;
}


#endif
