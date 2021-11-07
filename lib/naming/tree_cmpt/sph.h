/****************************************************************************
 *
 *      $Id: sph.h,v 1.3 2002/07/31 07:04:44 cgray Exp $
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

#ifndef __SPH_H__
#define __SPH_H__

#include <mungi.h>
#include "../lib/paxlib.h"
#include <mlib.h>

#define MAX_EPTS (10)
 
#define validate_cicap(a,b)  (1)
#define CSGC_MINOR_VERSION   (3)
#define generate_password()  (4)
#define IID_CLASS_INTERFACE  (5)
#define MID_CI_CONSTRUCTOR   (6)
#define MID_CI_DESTRUCTOR    (7)

#define IS_NOT_VALID(inst)       (((inst->flags)&MCS_IF_VALID) == 0)
#endif /* __SPH_H__ */

