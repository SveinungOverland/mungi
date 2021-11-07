/****************************************************************************
 *
 *      $Id: hmalloc.h,v 1.2 2002/05/31 07:56:35 danielp Exp $
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

/* hmalloc function calls - like malloc, but with 
 * the heap data passed in 
 */

/* initialise an hmalloc object buffer about 'size' bytes. */
void * hmalloc_init( size_t size, cap_t* cap );

/* insert caps needed to use stuff in this malloc */
void * hmalloc_access( void *vdat );

/* regular functions + 1 param */
void * hmalloc( void* hdat, size_t size );
void   hfree( void* hdat, void *ptr );
void * hrealloc( void* hdat, void *ptr, size_t size );
