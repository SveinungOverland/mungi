/****************************************************************************
 *
 *      $Id: syscallids.h,v 1.2 2002/05/31 05:49:37 danielp Exp $
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

/* thread id's for Mungi's servers */

#ifndef __M_SYSCALLIDS_H_
#define __M_SYSCALLIDS_H_

#include "mungi/l4_generic.h"

extern l4_threadid_t thread_tid;
extern l4_threadid_t object_tid;
extern l4_threadid_t semaphore_tid;
extern l4_threadid_t apd_tid;
extern l4_threadid_t mm_tid;

#define SEMAPHORE_TID 0
#define OBJECT_TID 0
#define THREAD_TID 0
#define MM_TID 0
#define APD_TID 0

#endif /* __M_SYSCALLIDS_H_ */
