/****************************************************************************
 *
 *      $Id: random.c,v 1.2 2002/05/31 06:14:10 danielp Exp $
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

#include "mungi/types.h"
#include "mungi/klibc.h"

/*
 *  FIXME (very^10000) dodgy random number generator, just used for the moment
 */
passwd_t
mrandom()
{
  static passwd_t seed = 0x1234567890abcdef;

  return (seed *= 121);
}
