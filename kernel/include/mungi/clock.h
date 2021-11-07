/****************************************************************************
 *
 *      $Id: clock.h,v 1.4 2002/07/22 10:17:38 cgray Exp $
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

#ifndef _CLOCK_H_
#define _CLOCK_H_

#if defined(CLOCK)
void _clock_init(void);
void _clock_start(void);
void _clock_dump(void);
void _clock_set_dump_enabled(int);
uint32_t _clock_stop(int v);

#define clock_init  _clock_init

/* interim timing functions */
#if 0
#define clock_start _clock_start
#define clock_stop  _clock_stop
#define clock_dump  _clock_dump
#define clock_set_dump_enabled _clock_set_dump_enabled
#else
#define clock_start() ((void)0)
#define clock_stop(x) ((void)0)
#define clock_dump()  ((void)0)
#define clock_set_dump_enabled(x) ((void)0)
#endif

#else
#define clock_init()  ((void)0)
#define clock_start() ((void)0)
#define clock_dump()  ((void)0)
#define clock_stop(x) ((void)0)
#define clock_set_dump_enabled(x) ((void)0)
#endif

enum clock_ticks 
{
	MSG_ENTRY = 0,   /* a debug entry */
	PRE_CR_IPC,
	PRE_PDX_IPC,     
	PRE_CE_IPC,
	PRE_RET_IPC,
	PRE_THREAD_RETURN,
	POST_CR_IPC,
	POST_PDX_IPC,
	POST_CE_IPC,
	
	POST_PDX_CACHE_LOOKUP,
	PRE_PDX_THREAD_CREATE,
	IN_THREAD_PDX_CREATE,
	PRE_SETUP_STACK,
	POST_SETUP_STACK,
	PRE_THREAD_ADD,
	POST_THREAD_ADD,
	IN_PDX_CACHE_LOOKUP,
	POST_HASHING,
	POST_CACHE_LOCK,
	PRE_APD_CHECK_ID,
	POST_APD_CHECK_ID,
	PRE_GET_MTHREAD,
	POST_GET_MTHREAD,

	APD_PRE_GET_TIME,
	APD_POST_GET_TIME,
	APD_GOT_ENT,
	APD_GOT_LOCK,
	APD_PRE_UNLOCK,
	APD_EXIT,

	FINAL_MEASURE
};


#ifdef MIPSENV
#include <l4/sigma0.h>
#include <kernel/gt64010a.h>
#include <kernel/u4600.h>

#define GT_TIMER_INTERVAL ((uint32_t)-1) /* maximum start value */

/* Macros for accessing GT chip */
#define byte_swap32(x) \
	((unsigned int)( \
		(((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
		(((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
		(((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
		(((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))

#define gtaddr32(x)	((volatile unsigned int   *)(GT_BASE_ADDR + (x)))
#define gtaddr16(x)	((volatile unsigned short *)(GT_BASE_ADDR + (x)))
#define gtaddr8(x)	((volatile unsigned char  *)(GT_BASE_ADDR + (x)))
#define gtval32(x)	byte_swap32(*gtaddr32(x))
#define gtval32set(x,y)	(*gtaddr32(x) = byte_swap32(y))
#endif /* MIPSENV */


#define CLOCK_OBJ_ADDR		((uint32_t *) 0x16000000LL)
#define CLOCK_OBJ_PASSWD	((passwd_t  ) 0x99999999LL)

#ifdef ALPHAENV
uint32_t read_pcc(void);
#endif /* ALPHAENV */


#endif /* _CLOCK_H_ */
