/****************************************************************************
 *
 *      $Id: klibc.h,v 1.5 2002/05/31 05:49:37 danielp Exp $
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

#ifndef __KLIBC_H
#define __KLIBC_H

#include "mungi/types.h"
#include "mungi/stdarg.h"

/*
 * TODO there are too many library functions here
 * bloating the kernel. Must kill some off...
 */
int snprintf ( char *s, size_t n, const char *format, ... );
int sprintf ( char *s, const char *format, ... );
void *memset ( void *s, int c, size_t n );
#define bzero(d, n)	memset(d, 0, n)
size_t strlen(const char *s);
int vsprintf(char *s, const char *format, va_list ap);
int vsnprintf(char *s, size_t n, const char *format, va_list ap);
char *strncpy(char *s1, const char *s2, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *s1, const char *s2);
void strtoupper (char *p);
char *strcat(char *, const char *);
char *strchr(const char *, int);
int atob (unsigned int *, char *, int);
char * btoa (char *, unsigned int, int);
char *llbtoa (char *, unsigned long, int);
int isdigit (int);
void str_fmt (char *, int, int);
int islower(int);
int isupper (int);
char *strichr (char *p, int c);
int toupper(int c);
int kprintf(const char * fmt,...) __attribute__ ((format (printf, 1, 2)));
passwd_t mrandom(void);
void *memcpy(void *, const void *, size_t);
void bcopy(const void *src, void *dest, int n);

#define FMT_RJUST 0
#define FMT_LJUST 1
#define FMT_RJUST0 2
#define FMT_CENTER 3

#endif /* __KLIBC_H */
