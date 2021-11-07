/****************************************************************************
 *
 *      $Id: panic.c,v 1.4 2002/05/31 06:14:10 danielp Exp $
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

#include "mungi/kernel.h"
#include "drivers/console.h"

void
panic(const char *fmt, ...)
{
	char buffer[1024];
	size_t bufsiz;
	va_list args;

	va_start(args, fmt);
	vsprintf(buffer, fmt, args); /* EG FIXME need vnsprintf!! */
	va_end(args);

	buffer[1023] = 0;
	bufsiz = strlen(buffer);

	console_out(buffer, bufsiz);

	assert(0);
}
