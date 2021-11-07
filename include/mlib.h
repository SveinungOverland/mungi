/****************************************************************************
 *
 *      $Id: mlib.h,v 1.3 2002/07/22 10:17:36 cgray Exp $
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

#ifndef __MLIB_H_
#define __MLIB_H_

/* FIXME: make this better */
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif


/* Add to a clist at a known position. ceg
 */
void add_to_clist_pos( cap_t cap, void *clist, int pos ) ;

/* Adda a cap to the next stop in our APD
 */
int apd_insert_cap( cap_t obj );

/* Create an object, a clist with a cap for the object, and a PD which has
 * the clist. Adds caps for all 3 objects to the APD */
int create_object_and_pd( int size, cap_t *obj, cap_t *pd );

/* Delete all the objects */
int delete_object_and_pd( void *obj, void *pd );

/* Create an object and put it into the APD */
int create_simple_object( int size, cap_t *obj );

/* Initialise the entries for a clist */
int init_clist( clist_t *clist );

/* Returns a random number */
unsigned long create_new_passwd(void);

/* create and init a clist - cap in APD */
int create_clist( int size, cap_t *clist );

/* create an empty clist and add it to the APD */
int create_clist_and_add( int size, cap_t *clist );

/* add cap to the clist */
void add_to_clist( cap_t cap, void *clist );

/* simple implementation of the std memory functions */
/*
void *memset( void *, int, int );
void *malloc( int );
void free( void* );
*/

/* prototypes for mungilib functions that don't have a header */
void _a_memcpy( char *dst, char *src, int s );
int atob64( void **vp, char *p, int base );
/* void start_timer( int start_value ); */
int get_time(void);

#endif /* __MLIB_H_ */

