/****************************************************************************
 *
 *      $Id: abort_test.h,v 1.2 2002/05/31 07:56:34 danielp Exp $
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

/* This is for stage aborts in performance testing! */

#define ABORT_NONE 0  /* do everything */
#define ABORT_NMAP 1  /* before PDX */
#define ABORT_MCS  2  /* after PDX */

#define ABORT_LEVEL ABORT_NONE
#define ABORT_MAX_COUNT 10000
