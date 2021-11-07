/****************************************************************************
 *
 *      $Id: security.h,v 1.7.2.1 2002/08/29 04:31:57 cgray Exp $
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

#ifndef __MUNGI_K_SECURITY_H
#define __MUNGI_K_SECURITY_H

#include "mungi/kernel.h"
#include "mungi/apd.h"
#include "mungi/objects.h"

extern cap_t NULL_CAP;

#define ALLOWED(access,rights) (((access) & (rights)) == (access))

void security_init(void);

/* validate access to a given address */
object_t * validate_obj(const void *, apd_t *, access_t, cap_t **,
			struct pending_request *);
object_t * validate_obj_pdx(const void *, apd_t *, access_t, cap_t **,
			    struct pending_request *);
 

/* validate access given by cap */
bool validate_cap(cap_t, access_t);

/* validate object range */
bool validate_range(object_t *, const void *, size_t);

#endif /* __M_SECURITY_H */ 
