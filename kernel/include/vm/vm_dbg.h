/****************************************************************************
 *
 *      $Id: vm_dbg.h,v 1.2 2002/05/31 05:49:44 danielp Exp $
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

#ifndef _VM_DBG_H
#define _VM_DBG_H

#include <vm/vm_types.h>


extern void pt_dump(const page_table_t *ptable, unsigned int max_ptable, 
		    const page_table_t *ctable, unsigned int max_ctable);
extern void ct_dump(const page_table_t *ctable, unsigned int max_ctable);
extern void ft_dump(const frame_table_t *ftable, unsigned int max_ftable);

#endif
