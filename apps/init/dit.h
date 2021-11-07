/****************************************************************************
 *
 *      $Id: dit.h,v 1.3 2002/05/31 04:57:09 danielp Exp $
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

/* User-land functions to find the DIT from the KIP and
 * read records/entry points from it 
 */

void *dit_find_dit(void* kip);
void *dit_find_item(void *vdit, char* name);
void *dit_find_entry_pt(void* dit, char* name);
int   dit_find_region(void* dit, char* name, 
		       uintptr_t* start, unsigned int* len);
