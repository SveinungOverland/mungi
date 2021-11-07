/****************************************************************************
 *
 *      $Id: semaphore.h,v 1.2 2002/05/31 05:20:11 danielp Exp $
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

#ifndef __MUNGI_SEMAPHORE_H
#define __MUNGI_SEMAPHORE_H

#define WAKE_ALL	(1 << 0)	/* wake all waiters of semaphore */
#define WAKE_LIFO	(1 << 1)	/* use a LIFO queue */
#define WAKE_AFFINITY	(1 << 2)	/* choose thread to wake up considering
					  CPU affinity (used with WAKE_LIFO) */

#endif /* __MUNGI_SEMAPHORE_H */
