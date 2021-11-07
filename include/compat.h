/****************************************************************************
 *
 *      $Id: compat.h,v 1.4 2002/08/01 08:10:08 cgray Exp $
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

#ifndef _SYS_TYPES_EXTENSION_H_
#define _SYS_TYPES_EXTENSION_H_
// #include <standards.h>

typedef unsigned int   uint_t;
typedef unsigned long  ulong_t;
typedef unsigned short ushort_t;
typedef unsigned char  uchar_t;

typedef int64_t        quad_t;

typedef long           off_t;
typedef char *         caddr1_t;
typedef int            dev_t;
typedef uintptr_t      ino_t;
typedef ushort_t       nlink_t;
#ifndef _WCHAR_T
#define _WCHAR_T
typedef unsigned char  wchar_t;
#endif
typedef unsigned int   wint_t;
typedef unsigned int   wctype_t;
typedef uint64_t       u_quad_t;
typedef char *         caddr_t;        

typedef ulong_t        ulong;
typedef ushort_t       ushort;

typedef ushort_t       u_short;
typedef uint_t         u_int;
typedef uchar_t        u_char;
typedef ulong_t        u_long;

#ifndef _MODE_T
#define _MODE_T
typedef uint_t         mode_t;
#endif

#ifndef _PID_T
#define _PID_T
typedef int            pid_t;
#endif

#ifndef _UID_T
#define _UID_T
typedef uint_t         uid_t;
#endif

#ifndef _GID_T
#define _GID_T
typedef uint_t		gid_t;		/* group ID */
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define NULL 0

/* POSIX types */
typedef uint64_t                uintmax_t;
typedef long int                clock_t;
typedef uint64_t                id_t;
typedef uint64_t                fd_set;

#endif
