/****************************************************************************
 *
 *      $Id: signal.h,v 1.3 2002/05/31 07:43:57 danielp Exp $
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

/*	$OpenBSD: signal.h,v 1.4 1998/11/20 11:18:26 d Exp $	*/
/*	$NetBSD: signal.h,v 1.8 1996/02/29 00:04:57 jtc Exp $	*/

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)signal.h	8.3 (Berkeley) 3/30/94
 */

#ifndef _USER_SIGNAL_H
#define _USER_SIGNAL_H

#include <sys/signal.h>

#if !defined(_ANSI_SOURCE)
#include <sys/types.h>
#endif

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
extern __const char *__const sys_signame[_NSIG];
extern __const char *__const sys_siglist[_NSIG];
#endif

__BEGIN_DECLS
int	raise __P((int));
#ifndef	_ANSI_SOURCE
int	kill __P((pid_t, int));
int	sigaction __P((int, const struct sigaction *, struct sigaction *));
int	sigaddset __P((sigset_t *, int));
int	sigdelset __P((sigset_t *, int));
int	sigemptyset __P((sigset_t *));
int	sigfillset __P((sigset_t *));
int	sigismember __P((const sigset_t *, int));
int	sigpending __P((sigset_t *));
int	sigprocmask __P((int, const sigset_t *, sigset_t *));
int	sigsuspend __P((const sigset_t *));

#if defined(__GNUC__) && defined(__STDC__)
extern __inline int sigaddset(sigset_t *set, int signo) {
	extern int errno;

	if (signo <= 0 || signo >= _NSIG) {
		errno = 22;			/* EINVAL */
		return -1;
	}
	*set |= (1 << ((signo)-1));		/* sigmask(signo) */
	return (0);
}

extern __inline int sigdelset(sigset_t *set, int signo) {
	extern int errno;

	if (signo <= 0 || signo >= _NSIG) {
		errno = 22;			/* EINVAL */
		return -1;
	}
	*set &= ~(1 << ((signo)-1));		/* sigmask(signo) */
	return (0);
}

extern __inline int sigismember(const sigset_t *set, int signo) {
	extern int errno;

	if (signo <= 0 || signo >= _NSIG) {
		errno = 22;			/* EINVAL */
		return -1;
	}
	return ((*set & (1 << ((signo)-1))) != 0);
}
#endif

/* List definitions after function declarations, or Reiser cpp gets upset. */
#define	sigemptyset(set)	(*(set) = 0, 0)
#define	sigfillset(set)		(*(set) = ~(sigset_t)0, 0)

#ifndef _POSIX_SOURCE
int	killpg __P((pid_t, int));
int	sigblock __P((int));
int	siginterrupt __P((int, int));
int	sigpause __P((int));
int	sigreturn __P((struct sigcontext *));
int	sigsetmask __P((int));
int	sigstack __P((const struct sigstack *, struct sigstack *));
int	sigaltstack __P((const struct sigaltstack *, struct sigaltstack *));
int	sigvec __P((int, struct sigvec *, struct sigvec *));
void	psignal __P((unsigned int, const char *));
int	sigwait __P((const sigset_t *, int *));
#endif	/* !_POSIX_SOURCE */
#endif	/* !_ANSI_SOURCE */
__END_DECLS

#endif	/* !_USER_SIGNAL_H */