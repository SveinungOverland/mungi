/****************************************************************************
 *
 *      $Id: pax_register.h,v 1.2 2002/05/31 07:56:41 danielp Exp $
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

#ifndef __PAX_REGISTER_H_
#define __PAX_REGISTER_H_

#define MAX_PDX_ENTRY_PTS 16
#define MAX_TOTAL_ENTRIES 32

#include <mungi.h>

/* the different types of entries that can be in a component registration descriptor */
typedef enum{ CRE_ENTRY_PT, CRE_CONTEXT, CRE_ENDLIST } entry_type_t;

/* info structures for the different entries */
typedef struct {
    long data0;     /* cid, obj_addr */
    long data1;     /* iid, access */
    long data2;     /* mid, N/A */
    long data3;     /* proc_addr, N/A */
} entry_data_t;

/* general entry structure */
typedef struct {
    entry_type_t entry_type;
    entry_data_t data;
} cr_data_t;

/* registration descriptor */
typedef struct {
	void *obj_addr;
	int flags;
	char *name;
	cr_data_t entries[MAX_TOTAL_ENTRIES];
} cmpt_reg_desc_t;

/* flags */
#define PREG_NO_REGISTER	0x1

/* the register function */
int pax_register_component( cmpt_reg_desc_t *crd, cap_t *pdc );

#endif /* __PAX_REGISTER_H_ */



