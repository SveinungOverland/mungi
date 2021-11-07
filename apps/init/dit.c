/****************************************************************************
 *
 *      $Id: dit.c,v 1.5 2002/08/23 08:24:10 cgray Exp $
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

/* Functions for init to read the DIT */
#include <mungi.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* For DIT stuff */
#include <l4/sigma0.h>
#ifdef ALPHAENV
#include <l4/dit.h>
#else
#include <kernel/dit.h>
#endif

#include "dit.h"

/* given the address of the KIP, find the DIT */
void *dit_find_dit(void* kip)
{
	return (Dit_Dhdr *)((l4_kernel_info*)kip)->dit_hdr;       
}

/* find an item in the DIT */
void *dit_find_item(void *vdit, char* name)
{
	uintptr_t ptr;
	size_t size;
	int i;
	Dit_Dhdr *dit;

	assert(vdit != NULL);
	assert(name != NULL);

	dit = vdit;

	ptr = (uintptr_t)dit + (uintptr_t)dit->d_phoff;
	size = (size_t)dit->d_phsize;

	printf("dfi: searching dit...\n");
	for (i = 0; i < (int)dit->d_phnum; i++) 
	{
		Dit_Phdr *dit_p = ((Dit_Phdr *)(ptr + (uintptr_t)(i*size)));
		char *p_name = dit_p->p_name;
    
/*
		printf("DIT program '%s' found.\n", p_name);
		printf("base 0x%lx, entry 0x%lx, size 0x%lx\n",
		       (long unsigned int)dit_p->p_base,
		       (long unsigned int)dit_p->p_entry,
		       (long unsigned int)dit_p->p_size);
*/
		if (!strncmp(p_name, name, strlen(name))) 
		{
			printf( "Found program!\n" );
			return dit_p;
		}
	}

	printf("Program not found!\n");
	return NULL;
}

/* find the start and length of a DIT entry */
int
dit_find_region(void* dit, char* name, 
		       uintptr_t* start, unsigned int* len)
{
	Dit_Phdr *p;

	assert(dit != NULL);
	assert(name != NULL);

	p = (Dit_Phdr*)dit_find_item(dit, name);

	/* not found */
	if (p == NULL) 
		return -1;

	if (start != NULL) 
		*start = p->p_base;

	if (len != NULL) 
		*len = p->p_size;

	/* 0 == OK! */
	return 0;
}

void *
dit_find_entry_pt(void* dit, char* name)
{
	Dit_Phdr *p;

	assert(dit != NULL);
	assert(name != NULL);

	p = (Dit_Phdr*)dit_find_item(dit, name);

	/* not found */
	if (p == NULL)
		return NULL;

	return (void *)(uintptr_t)p->p_entry;
}
