/*	$NetBSD: findfp.c,v 1.12 1998/11/15 17:19:53 christos Exp $	*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)findfp.c	8.2 (Berkeley) 1/4/94";
#else
__RCSID("$NetBSD: findfp.c,v 1.12 1998/11/15 17:19:53 christos Exp $");
#endif
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <compat.h>
#include "namespace.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"
#include "glue.h"
#include "reentrant.h"

int	__sdidinit;

#define	NDYNAMIC 10		/* add ten more whenever necessary */

#define	std(flags, file) \
	{0,0,0,flags,file,{0},0,__sF+file,__sclose,__sread,__sseek,__swrite}
/*	 p r w flags file _bf z  cookie      close    read    seek    write */

				/* the usual - (stdin + stdout + stderr) */

FILE __sF[3] = {
	std(__SRD, STDIN_FILENO),		/* stdin */
	std(__SWR|__SLBF, STDOUT_FILENO),	/* stdout (+linebuf)*/
	std(__SWR|__SNBF, STDERR_FILENO)	/* stderr */
};

static struct glue *moreglue __P((int));
void f_prealloc __P((void));

static struct glue *
moreglue(n)
	int n;
{
	struct glue *g;
	FILE *p;
	static FILE empty;

	/**********************************
	 * Introducing:  Another Stephens *
	 *   Dodgy Source Addition!!!     *
	 **********************************/

#define ALIGNBYTES 8
#define ALIGN(x)   (((x) % ALIGNBYTES) == 0 ? (x) : ((x)/ALIGNBYTES + 1)*ALIGNBYTES)

	g = (struct glue *)malloc(sizeof(*g) + ALIGNBYTES + n * sizeof(FILE));
	if (g == NULL)
		return (NULL);
	p = (FILE *)ALIGN((u_long)(g + 1));
	g->next = NULL;
	g->niobs = n;
	g->iobs = p;
	while (--n >= 0)
		*p++ = empty;
	return (g);
}

/*
 * Find a free FILE for fopen et al.
 */
FILE *
__sfp()
{
	FILE *fp;
	int n;
	struct glue *g;

	if (!__sdidinit)
		__sinit();

	rwlock_wrlock(&__sfp_lock);
	for (g = &__sglue;; g = g->next) {
		for (fp = g->iobs, n = g->niobs; --n >= 0; fp++)
			if (fp->_flags == 0)
				goto found;
		if (g->next == NULL && (g->next = moreglue(NDYNAMIC)) == NULL)
			break;
	}
	rwlock_unlock(&__sfp_lock);
	return (NULL);
found:
	fp->_flags = 1;		/* reserve this slot; caller sets real flags */
	fp->_p = NULL;		/* no current pointer */
	fp->_w = 0;		/* nothing to read or write */
	fp->_r = 0;
	fp->_bf._base = NULL;	/* no buffer */
	fp->_bf._size = 0;
	fp->_lbfsize = 0;	/* not line buffered */
	fp->_file = -1;		/* no file */
/*	fp->_cookie = <any>; */	/* caller sets cookie, _read/_write etc */
	fp->_ub._base = NULL;	/* no ungetc buffer */
	fp->_ub._size = 0;
	fp->_lb._base = NULL;	/* no line buffer */
	fp->_lb._size = 0;
	rwlock_unlock(&__sfp_lock);
	return (fp);
}

/*
 * XXX.  Force immediate allocation of internal memory.  Not used by stdio,
 * but documented historically for certain applications.  Bad applications.
 */

/* sysconf appears to be used infrequently, if at all.  It is a syscall, 
 * however, and should eventually be fixed up.
 *
 *   -Shane
 */

#define sysconf(x) 100

void
f_prealloc()
{
	struct glue *g;
	int n;

	n = (int)sysconf(_SC_OPEN_MAX) - FOPEN_MAX + 20; /* 20 for slop. */
	for (g = &__sglue; (n -= g->niobs) > 0 && g->next; g = g->next)
		/* void */;
	if (n > 0)
		g->next = moreglue(n);
}

/*
 * exit() calls _cleanup() through *__cleanup, set whenever we
 * open or buffer a file.  This chicanery is done so that programs
 * that do not use stdio need not link it all in.
 *
 * The name `_cleanup' is, alas, fairly well known outside stdio.
 */
void
_cleanup()
{
	/* (void) _fwalk(fclose); */
	(void) _fwalk(__sflush);		/* `cheating' */
}

/*
 * __sinit() is called whenever stdio's internal variables must be set up.
 */
void
__sinit()
{
	/* make sure we clean up on exit */
	__cleanup = _cleanup;		/* conservative */
	__sdidinit = 1;
}
