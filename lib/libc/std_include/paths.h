/****************************************************************************
 *
 *      $Id: paths.h,v 1.3 2002/05/31 07:43:57 danielp Exp $
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

/*	$OpenBSD: paths.h,v 1.18 2000/06/10 20:01:06 deraadt Exp $	*/
/*	$NetBSD: paths.h,v 1.7 1994/10/26 00:56:12 cgd Exp $	*/

/*
 * Copyright (c) 1989, 1993
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
 *	@(#)paths.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _PATHS_H_
#define	_PATHS_H_

/* Default search path. */
#define	_PATH_DEFPATH	"/usr/bin:/bin:/usr/sbin:/sbin:/usr/X11R6/bin:/usr/local/bin"
/* All standard utilities path. */
#define	_PATH_STDPATH	"/usr/bin:/bin:/usr/sbin:/sbin:/usr/X11R6/bin:/usr/local/bin"

#define	_PATH_BSHELL	"/bin/sh"
#define	_PATH_CONSOLE	"/dev/console"
#define	_PATH_CSHELL	"/bin/csh"
#define	_PATH_DEFTAPE	"/dev/rst0"
#define	_PATH_DEVDB	"/var/run/dev.db"
#define	_PATH_DEVNULL	"/dev/null"
#define	_PATH_DRUM	"/dev/drum"
#define	_PATH_FSIRAND	"/sbin/fsirand"
#define _PATH_KLOG      "/dev/klog"
#define	_PATH_KMEM	"/dev/kmem"
#define	_PATH_KSYMS	"/dev/ksyms"
#define	_PATH_KVMDB	"/var/db/kvm_bsd.db"
#define	_PATH_LOCALE	"/usr/share/locale"
#define _PATH_LOGCONF   "/etc/syslog.conf"
#define _PATH_LOGPID    "/var/run/syslog.pid"
#define	_PATH_MAILDIR	"/var/mail"
#define	_PATH_MAN	"/usr/share/man"
#define	_PATH_MEM	"/dev/mem"
#define	_PATH_NOLOGIN	"/etc/nologin"
#define	_PATH_RSH	"/usr/bin/rsh"
#define	_PATH_SENDMAIL	"/usr/sbin/sendmail"
#define	_PATH_SHELLS	"/etc/shells"
#define	_PATH_SKEYKEYS	"/etc/skeykeys"
#define	_PATH_TTY	"/dev/tty"
#define	_PATH_UNIX	"/bsd"
#define	_PATH_VI	"/usr/bin/vi"
#define _PATH_AFSD	"/usr/libexec/afsd"


/* Provide trailing slash, since mostly used for building pathnames. */
#define _PATH_BOOTDIR	"/usr/mdec/"
#define	_PATH_DEV	"/dev/"
#define	_PATH_TMP	"/tmp/"
#define	_PATH_UUCPLOCK	"/var/spool/lock/"
#define	_PATH_VARDB	"/var/db/"
#define	_PATH_VARRUN	"/var/run/"
#define	_PATH_VARTMP	"/var/tmp/"

#endif /* !_PATHS_H_ */
