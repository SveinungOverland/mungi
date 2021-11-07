/****************************************************************************
 *
 *      $Id: atob.c,v 1.3 2002/05/31 07:44:20 danielp Exp $
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

#include "compat.h"
#include <string.h>
 
char *llbtoa (char *dst, unsigned long value, int base);

static char * _getbase(char *,int *);
static int _atob(unsigned long *, char *, int);


static char * _getbase (char *p, int *basep)
{
    if (p[0] == '0') {
	switch (p[1]) {
	case 'x':
	    *basep = 16;
	    break;
	case 't': case 'n':
	    *basep = 10;
	    break;
	case 'o':
	    *basep = 8;
	    break;
	default:
	    *basep = 10;
	    return (p);
	}
	return (p + 2);
    }
    *basep = 10;
    return (p);
}


/*************************************************************
 *  _atob(vp,p,base)
 */
static int _atob (unsigned long *vp, char *p, int base)
{
    unsigned long  value, v1, v2;
    char           *q, tmp[20];
    int             digit;

    if (base == 16 && (q = strchr (p, '.')) != 0) {
	if (q - p > sizeof(tmp) - 1)
	    return (0);
	strncpy (tmp, p, (size_t) (q - p));
	tmp[q - p] = '\0';
	if (!_atob (&v1, tmp, 16))
	    return (0);
	q++;
	if (strchr (q, '.'))
	    return (0);
	if (!_atob (&v2, q, 16))
	    return (0);
	*vp = (v1 << 16) + v2;
	return (1);
    }
    value = *vp = 0;
    for (; *p; p++) {
	value *= base;
	if (*p >= '0' && *p <= '9')
	    digit = *p - '0';
	else if (*p >= 'a' && *p <= 'f')
	    digit = *p - 'a' + 10;
	else if (*p >= 'A' && *p <= 'F')
	    digit = *p - 'A' + 10;
	else
	    return (0);
	if (digit >= base)
	    return (0);
	value += digit;
    }
    *vp = value;
    return (1);
}

/*************************************************************
 *  atob(vp,p,base) 
 *      converts p to binary result in vp, rtn 1 on success
 */
int atob (unsigned int *vp, char *p, int base);
int 
atob(unsigned int *vp, char *p, int base)
{
    unsigned long  v;

    if (base == 0)
      p = _getbase (p, &base);
    if (_atob (&v, p, base)) {
	*vp = (unsigned int) v; /* should be 32-bit value */
	return (1);
    }
    return (0);
}


/*************************************************************
 *  char *btoa(dst,value,base) 
 *      converts value to ascii, result in dst
 */
char * btoa (char *dst, unsigned int value, int base);
char * 
btoa (char *dst, unsigned int value, int base)
{

#ifdef ALPHAENV

	long x = 0;
	x = (long)value;
	x &= 0x00000000FFFFFFFF;
	return llbtoa( dst, x, base );

#else

    char            buf[34], digit = '?';
    int             i, j, rem, neg,extnd=0;

    if (value == 0) {
	dst[0] = '0';
	dst[1] = 0;
	return (dst);
    }
    neg = 0;
    if (base == -10) {
	base = 10;
	if (value & (1L << 31)) {
	    value = (~value) + 1;
	    neg = 1;
	}
    }
    if (base==-16)
      {
	base= 16; extnd =1;
      }
    for (i = 0; value != 0; i++) {
	rem = value % base;
	value /= base;
	if (rem >= 0 && rem <= 9)
	    digit = rem + '0';
	else if (rem >= 10 && rem <= 36)
	    digit = (rem - 10) + 'a';
	buf[i] = digit;
    }
    buf[i] = 0;
    if (neg)
	strcat (buf, "-");
    if (extnd)
      strcat (buf,"x0");

/* reverse the string */
    for (i = 0, j = strlen (buf) - 1; j >= 0; i++, j--)
	dst[i] = buf[j];
    dst[i] = 0;
    return (dst);
#endif
}

#if __mips >= 3 || ALPHAENV
/*************************************************************
 *  char *btoa(dst,value,base) 
 *      converts value to ascii, result in dst
 */
char *llbtoa (char *dst, unsigned long value, int base)
{
    char            buf[66], digit = '?';
    int             i, j, rem, neg, extnd=0;

    if (value == 0) {
	dst[0] = '0';
	dst[1] = 0;
	return (dst);
    }
    neg = 0;
    if (base == -10) {
	base = 10;
#ifdef _LONGLONG
	if (value & (1LL << 63)) {
#else
	 if (value & (1L << 63)) {
#endif
	    value = (~value) + 1;
	    neg = 1;
	}
    }
    if (base==-16) {
      base = 16; extnd = 1;
    }
    for (i = 0; value != 0; i++) {
	rem = (int) (value % base);
	value /= base;
	if (rem >= 0 && rem <= 9)
	    digit = rem + '0';
	else if (rem >= 10 && rem <= 36)
	    digit = (rem - 10) + 'a';
	buf[i] = digit;
    }
    buf[i] = 0;
    if (neg)
	strcat (buf, "-");

    if (extnd)
      strcat (buf,"x0");

/* reverse the string */
    for (i = 0, j = strlen (buf) - 1; j >= 0; i++, j--)
	dst[i] = buf[j];
    dst[i] = 0;
    return (dst);
}
#endif
