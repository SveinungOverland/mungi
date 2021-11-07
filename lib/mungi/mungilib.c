/****************************************************************************
 *
 *      $Id: mungilib.c,v 1.6 2002/05/31 07:51:18 danielp Exp $
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

/* Mungi library internal stuff */

#include <mungi.h>
#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "mungi/syscallids.h"


cap_t NULL_CAP = { 0, 0};

void _mungi_lib_init(void);

/*
 * Perform any necessary library initialisation.
 */
void
_mungi_lib_init(void)
{
	l4_threadid_t mungi_tid;

	/* Find Mungi's L4 thread id */
	l4_id_nearest(SIGMA0_TID, &mungi_tid);

	/* thread id's of Mungi's various servers */
	semaphore_tid = object_tid = thread_tid = apd_tid = mm_tid = mungi_tid;

	semaphore_tid.id.lthread += SEMAPHORE_TID;
	object_tid.id.lthread += OBJECT_TID;
	thread_tid.id.lthread += THREAD_TID;
	apd_tid.id.lthread += APD_TID;
	mm_tid.id.lthread += MM_TID;
}
