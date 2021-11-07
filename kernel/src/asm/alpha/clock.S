/****************************************************************************
 *
 *      $Id: clock.S,v 1.2 2002/05/31 06:27:44 danielp Exp $
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

#include <asm.h>
#include <regdef.h>

/****************************************************************************
 *
 * uint32_t read_pcc(void)
 *
 * returns value of processor cycle counter
 *
 ****************************************************************************/

        .globl read_pcc

read_pcc:
        rpcc    v0
        ret     (ra)

