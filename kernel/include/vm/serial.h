/****************************************************************************
 *
 *      $Id: serial.h,v 1.3 2002/07/22 10:17:42 cgray Exp $
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

#include <mungi/l4_generic.h>

#ifndef _SERIAL_H
#define _SERIAL_H
#ifdef ALPHAENV
/* Thread id of serial driver */
static const l4_threadid_t SERIAL_TID = {0x1002000000060001LL};
#endif
#endif
