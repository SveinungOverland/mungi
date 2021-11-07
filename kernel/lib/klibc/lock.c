/****************************************************************************
 *
 *      $Id: lock.c,v 1.8 2002/08/23 08:24:22 cgray Exp $
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

/*   Hacked 3/9/96 for l4  by Jerry Vochteloo*/

#if !defined(INLINE_LOCKS) || (INLINE_LOCKS == 0)
#include "mungi/l4_generic.h"
#include "mungi/lock.h"
#endif

#ifndef LOCK_HIDE_FNS

/* macro to set us as the locker of a lock (if keeping lockers!) */
#if LOCK_KEEP_TID != 0
#define set_locker( lock, tst )    \
	if (tst)                   \
		lock->l_locker = l4_myself();
#else
#define set_locker( lock, tst ) ((void)0)
#endif  /* LOCK_KEEP_TID */

/* Arch-specific macros to do the atomic locking */
#ifdef MIPSENV
#define get_lock_indirect(l, v)			\
	asm("	.set	noreorder	\n"	\
	    "	ll	$8, (%0)	\n"	\
	    "	bne	$8, $0, 1f	\n"	\
	    "   move	$2, $0		\n"	\
	    "   li	$2, 1		\n"	\
	    "   sc      $2, (%0)	\n"	\
	    "1:	sw	$2, (%1)	\n"	\
	    "	.set	reorder		\n"	\
	    :					\
	    : "r" (&(l->l_lock)), "r" (v)	\
	    : "0", "1", "$8", "$2"		\
	    ) /* $0=zero, $2=v0, $8=t0 */
#endif /* MIPSENV */

#ifdef ALPHAENV
#define get_lock_indirect(l, v)			\
	asm("	clr	$0		\n"	\
	    "	ldl_l	$1, 0(%0)	\n"	\
	    "	bne	$1, 1f		\n"	\
	    "	ldiq	$0, 1		\n"	\
	    "	stl_c	$0, 0(%0)	\n"	\
	    "1:	stl	$0, 0(%1)	\n"	\
	    :					\
	    : "r" (&(l->l_lock)), "r" (v)	\
	    : "0", "1", "$0", "$1"              \
	    )
#endif /* ALPHAENV */

LOCK_INLINE void 
_lock_init(lock_t *l)
{
	l->l_lock = 0;
}

LOCK_INLINE bool 
_lock_try_lock(lock_t *l)
{
	int is_locked;

	get_lock_indirect(l, &is_locked);

	set_locker( l, is_locked );
	return (bool) is_locked;
}


/* wait on a lock */
static void 
wait_on_lock( lock_t *l )
{
	l4_threadid_t t;

	while (! _lock_try_lock(l)) {
		/* instead of busy-waiting trying the lock, poll the lock
		   status; this is more cache friendly */
#if LOCK_KEEP_TID != 0
		t = l->l_locker;
#else
		t = L4_NIL_ID;
#endif
		/* busy wait - yield to owner of lock 
		   OR anyone if not tracking locks.
		   FIXME: OK to switch to anyone? ceg
		*/
		l4_thread_switch(t); 
	}
}


LOCK_INLINE void 
_lock_lock(lock_t *l)
{
	int lock;

	/* see if we get the lock first off */
	get_lock_indirect( l, &lock );
	if( lock == 0 )
		wait_on_lock(l);

	/* mark it as ours */
	set_locker(l, 1);
}

LOCK_INLINE void 
_lock_unlock(lock_t *l)
{

	/* TODO should assert that we are the owner of the lock? */
#if LOCK_KEEP_TID != 0
	l->l_locker.ID = 0;
#endif
	l->l_lock = 0;
}

#endif /* LOCK_HIDE_FNS */
