/****************************************************************************
 *
 *      $Id: exception.h,v 1.3 2002/05/31 05:20:11 danielp Exp $
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

#ifndef __MUNGI_EXCEPTION_H
#define __MUNGI_EXCEPTION_H


#define EXP_MAX 16 /* Maximum number of exceptions */

typedef unsigned int excpt_t;

typedef void (*excpthndlr_t)(excpt_t, void *);

/**************\
 * Exceptions *
\**************/
#define E_KILL  ((excpt_t)1)    /* thread killed */
#define E_PROT  ((excpt_t)2)    /* protection violation */
#define E_ARITH ((excpt_t)3)    /* arithmetic exception */
#define E_UPCL  ((excpt_t)4)    /* upcall failed */
#define E_ILL   ((excpt_t)5)    /* illegal instruction */

#endif /* __MUNGI_EXCEPTION_H */
