/****************************************************************************
 *
 *      $Id: pf.h,v 1.2 2002/05/31 04:57:09 danielp Exp $
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

#ifndef __PF_H_
#define __PF_H_

void putchar( char c );
void printf(const char *, ...) __attribute__ ((format (printf, 1, 2)));
void str_fmt(char *p, int size, int fmt);
char *strcpy(char *dstp, const char *srcp);
char *strchr (const char *p, int c);
char *strncpy(char *dst, const char *src, size_t n);
int isdigit(int c);
int atob (unsigned int *vp, char *p, int base);
char * btoa (char *dst, unsigned int value, int base);
char *llbtoa (char *dst, unsigned long value, int base);
char *strcat (char *dst, const char *src);
char *strichr (char *p, int c);
size_t strlen (const char *p);
int strcmp(const char *s1, const char *s2);
int sprintf (char *buf, const char *fmt, ...);

#endif /* __PF_H_ */





