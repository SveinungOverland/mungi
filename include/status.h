/****************************************************************************
 *
 *      $Id: status.h,v 1.9 2002/08/23 08:24:15 cgray Exp $
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

/*
 *    Mungi status values
 */

#ifndef __MUNGI_STATUS_H_
#define __MUNGI_STATUS_H_

#define ST_SUCC         0x00    /* successful */
#define ST_NOMEM        0x01    /* out of memory */
#define ST_SIZ          0x02    /* invalid size */
#define ST_POS          0x04    /* invalid position */
#define ST_CAP          0x05    /* invalid capability */
#define ST_CLIST        0x06    /* invalid C-list */
#define ST_PWD          0x07    /* invalid password */
#define ST_INFO		0x08    /* invalid information */
#define ST_NULL		0x09	/* invalid NULL value */
#define ST_LOCK         0x11    /* APD locked */
#define ST_NOGROW       0x12    /* cannot grow */
#define ST_OVFL         0x13    /* table overflow */
#define ST_THR          0x14    /* invalid thread ID */
#define ST_PROT         0x16    /* protection violation */
#define ST_RNG          0x17    /* invalid range */
#define ST_EXCPT        0x18    /* invalid exception */
#define ST_USE          0x19    /* semaphore in use */
#define ST_SEMA         0x1a    /* invalid semaphore */
#define ST_NOIMP	0x1b	/* syscall not implemnted */
#define ST_ERR		0x1c	/* something really bad happened */
#define ST_SEMLMT	0x1d	/* no room for more semaphores */
#define ST_SDEL		0x1e	/* the semaphore has been deleted while we 
					were waiting on it */
#define ST_BANK		0x1f	/* invalid bank account data */
#define ST_PDX		0x20	/* invalid pdx data */

#endif /* __MUNGI_STATUS_H_ */
