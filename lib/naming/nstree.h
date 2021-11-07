/****************************************************************************
 *
 *      $Id: nstree.h,v 1.2 2002/05/31 07:56:36 danielp Exp $
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

/* nstree.h  - header information for a 'NameSpace Tree'. */

#ifndef _NSTREE_H_
#define _NSTREE_H_

struct BTree;  /* forward decl */

typedef union
{
	struct 
	{
		unsigned long all;
	} flags;

	struct
	{
		/* things that can be modfied internally */
		struct
		{
			unsigned int  dir  : 1;
			unsigned int  _pad : 31;
		} treedat;
		
		/* stuff we don't care about - 
		   let the user set it to what they like */
		struct
		{
			unsigned int type  : 3;  /* obj, semaphore, cap, 
						    other ?? */
			unsigned int  _pad : 29;
		} userdat;
	} dat;

} nsentflag_t;

struct sBTree;
typedef struct nsent_s
{
	void           *value;  /* pointer to actual object */
	nsentflag_t     flags;  /* flags */
	struct sBTree  *child;  /* ptr. to child btree */
	char           *name;   /* ptr. to the actual name */
	long            hash;   /* stored hash value to make 
				 comparisons faster */
	struct nsent_s* next; /* ptr. to next item w/ same hash */
} nsent_t;

typedef union
{
	struct
	{
		long num;
		long next_start;
		long next_overflow;
	} first;
	
	struct
	{
		void *value;
		nsentflag_t flags;
		int arrnext;   /* array offset of next */

		/* string will follow */
	} other;
} nsenum_t;

/* return values */
#define NST_OK      0
#define NST_FAIL    1
#define NST_MOREMEM 2    /* need a bigger buffer */

/* interface functions */
int ns_create( void **tree, cap_t* cap );
int ns_add( void *tree, char *name, nsentflag_t flags, void* value );
int ns_del( void *tree, char *name );
int ns_modify( void *tree, char *name, nsent_t *newdata, char *newname );
int ns_lookup( void *tree, char * name, nsent_t **data );
int ns_rlookup( void *tree, char *dir, unsigned long start, 
		unsigned long overflow, void *outdat, unsigned long outlen, 
		unsigned int num, void *value );
int ns_enum( void *tree, char *dir, unsigned long start, 
	     unsigned long overflow, void *outdat, unsigned long outlen, 
	     unsigned int num );



#endif  /* !_NSTREE_H_ */
