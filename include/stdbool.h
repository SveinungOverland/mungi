/****************************************************************************
 *
 *      $Id: stdbool.h,v 1.2 2002/05/31 05:20:11 danielp Exp $
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

#ifndef __M_STDBOOL_H__
#define __M_STDBOOL_H__

#ifndef __bool_true_false_are_defined

#define __bool_true_false_are_defined 1
#define bool _Bool
#define true 1
#define false 0

#endif /* __bool_true_false_are_defined */

#endif /* __M_STDBOOL_H__ */
