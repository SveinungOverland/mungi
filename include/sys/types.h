/****************************************************************************
 *
 *      $Id: types.h,v 1.16.2.1 2002/08/29 04:31:54 cgray Exp $
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

#ifndef __MUNGI_TYPES_H
#define __MUNGI_TYPES_H

#include <types.h>

#ifndef NULL
#define NULL 0
#endif

/* Types */
#if defined(MIPSENV)
#if defined(_MIPS_SZPTR) && (_MIPS_SZPTR == 64)
#else
#error Need 64 bit compiler to build Mungi - sorry!
#endif
#if defined(__GNUC__) && __GNUC__ < 3
typedef unsigned char		_Bool;	/* part of new C Standard */
#endif
typedef signed char		int8_t;
typedef unsigned char		uint8_t;
typedef short			int16_t;
typedef unsigned short		uint16_t;
typedef int			int32_t;
typedef unsigned int		uint32_t;
typedef long			int64_t;
typedef unsigned long		uint64_t;
#ifndef _SIZE_T
#define _SIZE_T
typedef uint64_t 		size_t;
#endif
typedef int64_t			ssize_t;
typedef uint64_t 		uintptr_t;

#elif defined(ALPHAENV)
#if defined(__GNUC__) && __GNUC__ < 3
typedef unsigned char		_Bool;	/* part of new C Standard */
#endif
typedef signed char		int8_t;
typedef unsigned char		uint8_t;
typedef short			int16_t;
typedef unsigned short		uint16_t;
typedef int			int32_t;
typedef unsigned int		uint32_t;
typedef long      		int64_t;
typedef unsigned long     	uint64_t;
#ifndef _SIZE_T
#define _SIZE_T
typedef uint64_t                size_t;
#endif
typedef int64_t			ssize_t;
typedef uint64_t 		uintptr_t;

#else /*  #if defined(arch) */
#error Mungi not ported to this architecture yet!!
#endif /* #if defined(arch) */

#define	bool _Bool
typedef unsigned int 		uint;
typedef int8_t			apdpos_t;
typedef uint64_t 		passwd_t;

/* capability */
typedef struct {
	void *address;
	passwd_t passwd;
} cap_t;

/* 
 * Access Rights stuff
 */
typedef int8_t access_t;
typedef int8_t mac_access_t;

#define M_EXECUTE	((access_t) 1<<0)
#define M_WRITE		((access_t) 1<<1)
#define M_READ		((access_t) 1<<2)
#define M_DESTROY	((access_t) 1<<3)
#define M_PDX		((access_t) 1<<4)
#define M_NOT		((access_t) 1<<5)
#define M_SYNC		(M_READ|M_WRITE)
#define M_OWNER		(M_EXECUTE|M_READ|M_WRITE|M_DESTROY|M_SYNC)

/* MAC validation request sent to policy object and returned with the result */
struct validation_request {
	unsigned long subject_label:60;
	unsigned long set_0_or_1:1;
	unsigned long read_or_create_domain:1;
	unsigned long write_or_create_type:1;
	unsigned long execute_or_transfer:1;
	unsigned long object_label:60;
	unsigned long destroy_or_pdx:1;
	unsigned long result:1;
	unsigned long undef:2;
};


/************\
 *  Objects *
\************/

/* time */
typedef uint64_t time_t;
#define SLEEP_INFINITY	((uint64_t)-1)


/* Flags specifying what to modify with an ObjectInfo call */

#define O_SET_NONE      0x00  /* don't change */ 
#define O_SET_MODIFY    0x01  /* set modify time */
#define O_SET_ACCESS    0x02  /* set access time */
#define O_SET_ACCNT     0x04  /* set accounting time */
#define O_SET_TYPE      0x08  /* set the security type label */
#define O_SET_MASK      0x0f
 
#define O_TCH_MODIFY    0x10  /* touch modify time */
#define O_TCH_ACCESS    0x20  /* touch access time */
#define O_TCH_ACCNT     0x40  /* touch accounting time */
#define O_TCH_MASK      0xf0


/* general object flags */

typedef int objflags_t;
#define O_PERS		((objflags_t)0x001)	/* is persistent */

/* special object flags */
#define O_PD		((objflags_t)0x001)	/* A PD object */
#define O_ACCT		((objflags_t)0x002)	/* is a bank account */
#define O_ACCT_F	((objflags_t)0x004)	/* bank account is financial */


#define O_MAX_CAPS      0x80
#define O_MAX_PDX       0x10
#define O_MAX_ENTPT     0xC0


#define PD_MERGE        (apddesc_t *)-1
#define PD_EMPTY        (apddesc_t *)0 

typedef cap_t (*pdx_t)(cap_t); /* PDX function with arbitrary arg */
typedef struct {
	cap_t      clist[O_MAX_PDX];
	passwd_t  passwd[O_MAX_PDX];
	int       n_entry[O_MAX_PDX];
	int       x_entry[O_MAX_PDX];
	pdx_t    entry[O_MAX_ENTPT];
} pdxdata_t;

/* public object info */
typedef struct {
	/* public */
	size_t		extent;   /* block size to use on backing store */
	time_t		creation;
	time_t		modification;
	time_t		access;
	time_t		accounting;
	void		*userinfo;
	void		*acctinfo;
	size_t		length;
                
	/* private */
	objflags_t	flags;
	objflags_t	special;
	uint		n_caps;
	uint		n_pdx;
	cap_t		account;
	cap_t		pager;
	cap_t		cntrl_object;
	passwd_t   	passwd[O_MAX_CAPS]; /* separate password & rights for */
	access_t	rights[O_MAX_CAPS]; /* smaller space due to alignment */
        pdxdata_t	pdx;
} objinfo_t;



/*******\
 * APD *
\*******/

#define APD_MAX_ENTRY      0x10
typedef struct {
	cap_t		clist[APD_MAX_ENTRY];	/* address of Clists */
	apdpos_t	n_locked;		/* slot number that is locked */
	apdpos_t	n_apd;			/* number of slots in use */
} apddesc_t;


typedef uint8_t clistformat_t;
#define CL_UNSRT_0      ((clistformat_t)0x1)    /* unsorted format */
#define CL_SRT_0        ((clistformat_t)0x2)    /* sorted format */

typedef struct {
	char          	type;           /* magic number 'c' */
	uint8_t         rel_ver;        /* Presently 1 */
	clistformat_t 	format;
	uint16_t       	n_caps;         /* size of Clist */
	uint32_t        reserved;
	cap_t         	caps[1];	/* cap array of size n_caps */
} clist_t;




/**********\
 * Threads *
\**********/

typedef int (*thread_t)(void *); /* thread function with arbitrary arg */

typedef uint64_t         mthreadid_t;
#define THREAD_MYSELF	((mthreadid_t)0)
#define THREAD_ANY	((mthreadid_t)0)
#define THREAD_NULL	((mthreadid_t)0)


/* flags used to specify what parameters we supplied */
#define THREAD_STACK_ADDR	(1 << 0) /* a stack address was specified */
#define THREAD_STACK_SIZE	(1 << 1) /* a stack size was specified */
#define THREAD_MEM_LIMIT	(1 << 2) /* memory limit was specified */
#define THREAD_CPU_TIME		(1 << 3) /* time limit was specified */
#define THREAD_DETACHED		(1 << 4) /* start the thread detached (ie dont 
					   wait for it to finish */
#define THREAD_BANK_ACCOUNT	(1 << 5)
#define THREAD_NO_JOINPD	(1 << 6) /* instantiate from PD */ 

typedef struct {
	int32_t flags; /* flags indicating thread parameters filled in*/
	uint	prio;
	void 	*stack_addr;
	size_t	stack_size;
	time_t	start_time;
	time_t	cpu_time;
	time_t	cpu_limit;
	size_t	mem_limit;
	void	*bank_account;
	void	*env;	/* environment */
	size_t	env_size;
} threadinfo_t;


/*****************\
 * Page Mappings *
\*****************/

typedef int pagefault_t;
#define PF_RESID        ((pagefault_t)1)        /* Residency fault */
#define PF_WRITE        ((pagefault_t)2)        /* Write fault */
#define PF_FLUSH        ((pagefault_t)3)        /* Flush event */

typedef int pagedisp_t;
#define P_DSP_ZERO      ((pagedisp_t)1)
#define P_DSP_REPLACE   ((pagedisp_t)2)
#define P_DSP_KEEP      ((pagedisp_t)3)
#define P_DSP_UNALIAS   ((pagedisp_t)4)

typedef bool (*pager_t)(const void *, size_t n_pages, pagefault_t);

#endif  /* __MUNGI_TYPES_H */

