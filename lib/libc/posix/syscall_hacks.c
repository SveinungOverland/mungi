/****************************************************************************
 *
 *      $Id: syscall_hacks.c,v 1.4 2002/08/01 08:10:10 cgray Exp $
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

/* the place for hacked-up syscalls to get things working (eg. tar) */

#include <mungi.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef void (*sighandler_t)(int);

#define POSIX_STUB_WARNINGS
#ifdef POSIX_STUB_WARNINGS
#define pwarn(x...) printf( "POSIX: " ), printf(x)
#else
#define pwarn(x...) ((void)0)
#endif


/* some stuff required for system errors? */
/* these should stop anyone using them, but allow compilation */
char **sys_errlist = NULL;
int sys_nerr = -1;


// `access'		        - return 0 :)
int access(const char *pathname, int mode)
{
    pwarn("access() called\n");
    return 0;
}


// `chmod'			- return 0
int chmod(const char *path, mode_t mode)
{
    pwarn("chmod() called\n");
    return 0;
}

// `chown'			- return 0
int chown(const char *path, uid_t owner, gid_t group)
{
    pwarn("chown() called\n");
    return 0;
}

// `execl'			- assert should be OK?
int execl( const char *path, const char *arg, ...)
{
    pwarn("execl() called\n");
    return 0;
}

// `fork'			- ERK! assert should be OK
pid_t fork(void)
{
    assert(!"fork() called");
    return -1;
}

// `geteuid'		        - return 0
uid_t geteuid(void)
{
    pwarn("geteuid() called\n");
    return 0;
}

// `link'			- assert?? (impl. later with p9 stuff?)
int link(const char *oldpath, const char *newpath)
{
    assert(!"link() called");
    return 0;
}

// `symlink'		- assert?
int symlink(const char *oldpath, const char *newpath)
{
    assert(!"symlink() called");
    return 0;
}

// `mknod'			- assert?
int mknod(const char *pathname, mode_t mode, dev_t dev)
{
    assert(!"mknod() called");
    return 0;
}

// `pipe'			- assert?
int pipe(int filedes[2])
{
        assert(!"pipe() called");
        return 0;
}

// `readlink'		        - assert?
int readlink(const char *path, char *buf, size_t bufsiz)
{
        assert(!"readlink() called");
        return 0;
}

// `signal'		        - return 0?
sighandler_t signal(int signum, sighandler_t handler)
{
        pwarn( "signal() called. ignoring.\n" );
        return 0;
}

// `strncasecmp'		- return 1?
int strncasecmp(const char *s1, const char *s2, size_t n)
{
        pwarn( "strncasecmp() called\n" );
        return 0;
}

// `system'		        - assert for now
int system (const char * string)
{
        assert( !"system called" );
        return 0;
}

// `time'			- return 0?
time_t time(time_t *t);
time_t time(time_t *t)
{
        pwarn("time() called\n");
        return 0;
}

// `umask'			- return 0777?
mode_t umask(mode_t mask)
{
        pwarn("umask() called\n");
        return 0777;
}

// `utime'			- lib has an impl? return 0?
#ifdef UTIME_H_
int utime(const char *filename, struct utimbuf *buf);
int utime(const char *filename, struct utimbuf *buf)
{
        pwarn("utime() called\n");
        return 0;
}
#endif

// `fscanf'                     - could impl, butt not really needed ATM
int fscanf( FILE *stream, const char *format, ...)
{
        assert(!"fscanf() called");
        return 0;
}

// `qsort'                      - only needed during creation. impl some time.
void qsort(void *base, size_t nmemb, size_t size,
	   int (*compar)(const void *, const void *))
{
        assert( !"qsort() called" );
}
