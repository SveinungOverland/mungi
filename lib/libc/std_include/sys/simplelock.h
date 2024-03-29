/****************************************************************************
 *
 *      $Id: simplelock.h,v 1.2 2002/05/31 07:44:12 danielp Exp $
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

/*	$OpenBSD: simplelock.h,v 1.9 2001/02/26 15:43:08 art Exp $	*/

#ifndef _SIMPLELOCK_H_
#define _SIMPLELOCK_H_
/*
 * A simple spin lock.
 *
 * This structure only sets one bit of data, but is sized based on the
 * minimum word size that can be operated on by the hardware test-and-set
 * instruction. It is only needed for multiprocessors, as uniprocessors
 * will always run to completion or a sleep. It is an error to hold one
 * of these locks while a process is sleeping.
 */
struct simplelock {
	int	lock_data;
};

#ifdef _KERNEL

#ifndef NCPUS
#define NCPUS 1
#endif

#define SLOCK_LOCKED 1
#define SLOCK_UNLOCKED 0

#define SLOCK_INITIALIZER { SLOCK_UNLOCKED }

/*
 * We can't debug locks when we use them in real life.
 */
#if (NCPUS != 1) && defined(LOCKDEBUG)
#undef LOCKDEBUG
#endif

#if NCPUS == 1

#ifndef LOCKDEBUG

#define	simple_lock(lkp)
#define	simple_lock_try(lkp)	(1)	/* always succeeds */
#define	simple_unlock(lkp)
#define simple_lock_assert(lkp)

static __inline void simple_lock_init __P((struct simplelock *));

static __inline void
simple_lock_init(lkp)
	struct simplelock *lkp;
{

	lkp->lock_data = SLOCK_UNLOCKED;
}

#else

void _simple_unlock __P((__volatile struct simplelock *, const char *, int));
int _simple_lock_try __P((__volatile struct simplelock *, const char *, int));
void _simple_lock __P((__volatile struct simplelock *, const char *, int));
void _simple_lock_assert __P((__volatile struct simplelock *, int, const char *, int));

void simple_lock_init __P((struct simplelock *));
#define simple_unlock(lkp) _simple_unlock(lkp, __FILE__, __LINE__)
#define simple_lock_try(lkp) _simple_lock_try(lkp, __FILE__, __LINE__)
#define simple_lock(lkp) _simple_lock(lkp, __FILE__, __LINE__)
#define simple_lock_assert(lkp, state) _simple_lock_assert(lkp, state, __FILE__, __LINE__)

#endif /* !defined(LOCKDEBUG) */

#else  /* NCPUS >  1 */

/*
 * The simple-lock routines are the primitives out of which the lock
 * package is built. The machine-dependent code must implement an
 * atomic test_and_set operation that indivisibly sets the simple lock
 * to non-zero and returns its old value. It also assumes that the
 * setting of the lock to zero below is indivisible. Simple locks may
 * only be used for exclusive locks.
 */

static __inline void
simple_lock(lkp)
	__volatile struct simplelock *lkp;
{

	while (test_and_set(&lkp->lock_data))
		continue;
}

static __inline int
simple_lock_try(lkp)
	__volatile struct simplelock *lkp;
{

	return (!test_and_set(&lkp->lock_data))
}

static __inline void
simple_unlock(lkp)
	__volatile struct simplelock *lkp;
{

	lkp->lock_data = 0;
}
#endif /* NCPUS > 1 */

#endif /* _KERNEL */

#endif /* !_SIMPLELOCK_H_ */
