/****************************************************************************
 *
 *      $Id: exits.c,v 1.3 2002/05/31 07:43:52 danielp Exp $
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

/* ways to exit a program */

#include <mungi.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void 
do_death(int code)
{
    int r;

    r = ThreadDelete(THREAD_MYSELF, code, 0);
    
    /* if we got here, something is wrong! */
    printf("ThreadDelete() on exit returned %d\n", r);

    assert(!"badness!");
}

void exit(int status)
{
    do_death(0);
}

void abort(void)
{
    do_death(1);
}
