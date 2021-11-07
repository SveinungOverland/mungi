/****************************************************************************
 *
 *      $Id: getenv.c,v 1.3 2002/05/31 08:01:36 danielp Exp $
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

#include <stdlib.h>
#include <string.h>

struct env {
        char *name;
        char *value;
};

struct env env[] = {
        {"HOME","/"},
        {"LOGNAME","mungi"},
        {"PATH",""},
        {"TERM","mungi-console"},
        {NULL, NULL},
};

char *
getenv(const char *name) 
{
        struct env *ep;

        for (ep = env ; ep->name != NULL ; ep ++){
                if (strcmp(ep->name,name) == 0)
                        return ep->value;
        }

        return 0;
}
    

