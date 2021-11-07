/****************************************************************************
 *
 *      $Id: namehash.h,v 1.2 2002/05/31 07:56:35 danielp Exp $
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

/* NameHash stuff */

#ifndef NAMEHASH_H
#define NAMEHASH_H

typedef struct hashent_s
{
	struct hashent_s* next;
	char *name;
	void *child;
} hashent_t;

typedef struct
{
	/* malloc data */
	void  *hmalloc_dat;
	cap_t  hmalloc_cap;

	/* the actual hash table */
	int size;
	hashent_t **table;

} namehash_t;


#define NH_OK   0
#define NH_FAIL 1
#define NH_MEM  2

/* functions requiring a hash table */
namehash_t *nh_init( int size, int entries );
char       *nh_absolute( char *name, char *cwd );

int nh_modify( namehash_t *nhash, char *name, void *mod, char *cwd );
int nh_delete( namehash_t *nhash, char *name, char *cwd );
int nh_insert( namehash_t *nhash, char *name, void *value, char *cwd );
int   nh_find( namehash_t *nhash, char *name, void **value, char *cwd );


/* so we can allocate in the same block as the hash */
void *nh_alloc  ( namehash_t *nhash, size_t size );
void *nh_realloc( namehash_t *nhash, void *ptr, size_t size );
void  nh_free   ( namehash_t *nhash, void* mem );

#endif /* NAMEHASH_H */
