/****************************************************************************
 *
 *      $Id: limits.h,v 1.6 2002/06/05 05:36:22 cgray Exp $
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

/* Mungi limits */

#ifndef __MUNGI_K_LIMITS_H
#define __MUNGI_K_LIMITS_H

#define SEM_MAX 16			/* semaphores per object */	
#define VCACHE_SIZE 16			/* number on entries in the VCACHE */
#define THREAD_HASHSIZE	1024	        /* size of thread hash table */
#define MTHREAD_STACK_SIZE (1024*128)	/* L4 task stack size in bytes */
#define KTHREAD_STACK_SIZE (16*1024)    /* mungi kernel thread stack size in 
 						bytes */

#endif /* __MUNGI_K_LIMITS_H */
