/****************************************************************************
 *
 *      $Id: lock.h,v 1.10 2002/08/23 08:24:19 cgray Exp $
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

/* Copyright (c) 1995 Michael Hohmuth
   See l3/mach_emul/COPYRIGHT for copyright and licensing details */

/* spin locks */

#ifndef MUNGI_LOCK_H
#define MUNGI_LOCK_H

#include "mungi/l4_generic.h"
#include "mungi/types.h"


#define LOCK_KEEP_TID 0  /* keep TIDs in lock structure? */
#define INLINE_LOCKS  1  /* flag to inline the lock code */

struct lock {
#if LOCK_KEEP_TID != 0   /* no point in this field, otherwise */
  l4_threadid_t l_locker;
#endif
  int l_lock;
};

typedef struct lock lock_t;

#define spinlock_t lock_t
#define spin_lock _lock_lock
#define spin_unlock _lock_unlock
#define spin_lock_init _lock_init

/* check if currently locked without acquiring the lock */
#define _lock_locked(l) ((l)->l_lock)

#if INLINE_LOCKS == 0

/* initialize a lock */
void _lock_init(lock_t *l);

/* get a lock; blocking */
void _lock_lock(lock_t *l);

/* try to get a lock; not blocking */
bool _lock_try_lock(lock_t *l);

/* unlock a lock */
void _lock_unlock(lock_t *l);

#define LOCK_INLINE

#else

#define LOCK_INLINE extern inline

/* initialize a lock */
LOCK_INLINE void _lock_init(lock_t *l);

/* get a lock; blocking */
LOCK_INLINE void _lock_lock(lock_t *l);

/* try to get a lock; not blocking */
LOCK_INLINE bool _lock_try_lock(lock_t *l);

/* unlock a lock */
LOCK_INLINE void _lock_unlock(lock_t *l);


/* this is kinda dodgey, but it avoids clag - ceg */
#include "../../lib/klibc/lock.c"

/* stop the lock.c file from re-compiling the functions */
#define LOCK_HIDE_FNS

#endif

#endif /* MUNGI_LOCK_H */
