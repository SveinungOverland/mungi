/****************************************************************************
 *
 *      $Id: clist.h,v 1.3 2002/05/31 05:49:36 danielp Exp $
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

/* clist stuff */
#ifndef __M_CLIST_H_
#define __M_CLIST_H_

#include "mungi/kernel.h"

#define CLIST(c)	((clist_t *)(c))
#define CLIST_INIT(c)	(*(clist_t *)(c) = system_clist)
#define IS_NOT_CLIST(c)	(((c) == NULL) || ((c)->type != 'c') || \
			((c)->rel_ver != 1))
#define ADDRESS_MASK	L4_PAGEMASK

extern clist_t system_clist;

#endif /* __M_CLIST_H_ */
