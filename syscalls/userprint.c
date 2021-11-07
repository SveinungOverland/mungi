/****************************************************************************
 *
 *      $Id: userprint.c,v 1.4 2002/08/26 07:03:12 cgray Exp $
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

#include "userprint.h"

int vsprintf(char *d, const char *s, va_list ap);

int UserPrint(const char *s, ...) {
  int x;
  va_list ap;
  char str[64];

  va_start(ap, s);
  x = vsprintf(str, s, ap);
  SeedyPrint(str);
  va_end(ap);

  return x;

}
