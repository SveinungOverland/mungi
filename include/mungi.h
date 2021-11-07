/****************************************************************************
 *
 *      $Id: mungi.h,v 1.6 2002/05/31 05:20:11 danielp Exp $
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

/*
 * This should be the only header file a Mungi application needs
 * to include for basic mungi services.
 */

#ifndef __MUNGI_H
#define __MUNGI_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <sys/types.h>
#include <syscalls.h>
#include <stddef.h>
#include <clist.h>
#include <stdbool.h>
#include <exception.h>
#include <semaphore.h>


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MUNGI_H */
