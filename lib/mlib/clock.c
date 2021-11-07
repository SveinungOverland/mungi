/****************************************************************************
 *
 *      $Id: clock.c,v 1.3 2002/07/22 10:17:56 cgray Exp $
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
 * simple clock driver for mungi profiling
 * provides clock_start() and clock_stop() functions for user code
 *
 * MIPS:
 * uses the 50MHz timer of the GT chip
 * GT device mapping taken from mpager (for user threads) or rpager in kernel
 * requires extra code in mpager to handle the device mapping
 *
 * ALPHA:
 * uses the builtin CPU clock via the rpcc instruction
 * the kernel side init sets up an object with a known address and password
 * that is used to store the start value of the timer (for cross-domain timing)
 * depends on read_pcc in lib/mungi/asm/alpha/clock.S
 */

#include "mungi/kernel.h"
#include "mungi/objects.h"
#include "mungi/mm.h"
#include "mungi/clock.h"
#include <l4_generic.h>


#if defined(CLOCK)

//#define pause()	{ l4_msgdope_t _res; l4_ipc_sleep(L4_IPC_TIMEOUT(0,0,2,6,0,0), &_res); }
#define pause() ((void)0)


#ifdef KERNEL_CLOCK
// #define debug(x...) kprintf( "clK: " x )
#define debug(x...) ((void)0)
#else
extern void UserPrint(const char *, ...);
//#define debug(x...) UserPrint( "clU: " x )
#define debug(x...) ((void)0)
#endif



#define ntab(x) ((x)<10?&"\t\t\t\t\t\t\t\t\t\t"[10-(x)]:"ERR: ")

static int dump_enabled = 0;
uint32_t *startval = CLOCK_OBJ_ADDR;

int find_clock_entry( int p );
void swap( int *x1, int *x2 );

#ifdef KERNEL_CLOCK

static void clock_init_kernel(void);
static void 
clock_init_kernel(void)
{
	/* in kernel, create object at known address to store the start time */
	void *base;
	object_t *obj;

	/* allocate objects backing */
	base = kmalloc_obj_addr(CLOCK_OBJ_ADDR, sizeof(*startval) * 0x1000);
	assert(base == CLOCK_OBJ_ADDR);

	/* add object to object table */
	obj = object_add(CLOCK_OBJ_ADDR, sizeof(*startval) * 0x1000, 
			 CLOCK_OBJ_PASSWD);
	assert(obj != NULL);
}

#else

struct perf_rec
{
	int depth;
	char *desc;
	enum clock_ticks p1, p2;
};

static struct perf_rec perf_dat[] = 
{
	{ 0, "** IPC Timings",       0, 0 },
	{ 1, "caller  -> syscall",   PRE_CR_IPC,  POST_CR_IPC },
	{ 1, "syscall -> callee",    PRE_PDX_IPC, POST_PDX_IPC },
	{ 1, "callee  -> syscall",   PRE_CE_IPC,  POST_CE_IPC },
	{ 1, "syscall -> caller",    PRE_RET_IPC, FINAL_MEASURE },
	{ 0, "** Kernel Timings",    0, 0 },
	{ 1, "total kernel (there)", POST_CR_IPC, PRE_PDX_IPC },
	{ 2, "PDX cache lookup",     POST_CR_IPC, POST_PDX_CACHE_LOOKUP },
	{ 3, "hash",                 IN_PDX_CACHE_LOOKUP, POST_HASHING },
	{ 3, "lock",                 POST_HASHING, POST_CACHE_LOCK },
	{ 3, "if()",                 POST_CACHE_LOCK, PRE_APD_CHECK_ID },
	{ 3, "check_id",             PRE_APD_CHECK_ID, POST_APD_CHECK_ID },
	{ 2, "if stmt",              POST_PDX_CACHE_LOOKUP, PRE_PDX_THREAD_CREATE },
	{ 2, "thrd_pdx_create",      PRE_PDX_THREAD_CREATE, PRE_PDX_IPC },
	{ 3, "call",                 PRE_PDX_THREAD_CREATE, IN_THREAD_PDX_CREATE },
	{ 3, "lock",                 IN_THREAD_PDX_CREATE, PRE_GET_MTHREAD },
	{ 3, "thread_get_mt",        PRE_GET_MTHREAD, POST_GET_MTHREAD },
	{ 3, "setup_stack",          PRE_SETUP_STACK, POST_SETUP_STACK },
	{ 3, "thread_add",           PRE_THREAD_ADD, POST_THREAD_ADD },
	{ 3, "fiddle syscall",       POST_THREAD_ADD, PRE_PDX_IPC },
	{ 1, "total kernel (back)",  POST_CE_IPC, PRE_RET_IPC },
	{ 0, "** Bottom line",       0, 0 },
	{ 1, "total time",           PRE_CR_IPC,  FINAL_MEASURE },
};

#define NUM_PERFS (sizeof(perf_dat)/sizeof(struct perf_rec))

static int abs( int i )
{
	if( i < 0 )
		return -i;
	else
		return i;
}

void 
_clock_dump(void)
{
	int x, i1, i2;
	
	if( dump_enabled == 0 )
		return;

	/* dump out the raw values */
/*
	for( x = 2; x < startval[1]; x++ )
		UserPrint( "startval[%2d] = %u\n", x, startval[x] );
*/
	UserPrint( "\n---- START CLOCK DUMP ----\n" );
	for( x = 0; x < NUM_PERFS; x++ )
	{
		if( perf_dat[x].p1 == 0 )
			UserPrint( "%s%s\n", ntab( perf_dat[x].depth ), 
				   perf_dat[x].desc );
		else
		{
			i1 = find_clock_entry( perf_dat[x].p1 );
			i2 = find_clock_entry( perf_dat[x].p2 );
			
			if( startval[i2] < startval[i1] )
				swap( &i1, &i2 );

			UserPrint("%s%-30s - %6u (%2d) [%d, %d]\n", 
				  ntab( perf_dat[x].depth ),perf_dat[x].desc, 
				  startval[i2] - startval[i1], abs(i2 - i1) / 2,
				  i2, i1 );
		}
	}

	startval[1] = 2;
	UserPrint( "----- END CLOCK DUMP -----\n" );
}


static int ApdGet(apddesc_t *buffer);
static cap_t *ApdLookup(const void *address, access_t minrights);

static l4_threadid_t mungi_tid;

static void clock_init_user(void);
static void 
clock_init_user(void)
{
	/* setup capability to allow us to access the start time */
	clist_t *clist;
	apddesc_t apd;
	cap_t cap = { CLOCK_OBJ_ADDR, CLOCK_OBJ_PASSWD };

	l4_id_nearest(SIGMA0_TID, &mungi_tid);
	ApdGet(&apd);
	clist = (clist_t *)apd.clist[0].address;
	clist->caps[clist->n_caps++] = cap;
	assert(ApdLookup(CLOCK_OBJ_ADDR, (M_READ|M_WRITE)) != NULL);

	debug( "clU: Stuff configured\n" );
}

static int
ApdGet(apddesc_t *buffer)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_APD_GET;
	sysmsg.syscall.data.apd.get.buffer = buffer;
	
	l4_ipc_call(mungi_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
		    L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.retval;
}

static cap_t *
ApdLookup(const void *address, access_t minrights)
{
	syscall_t sysmsg;
	l4_msgdope_t result;

	sysmsg.syscall.number = SYS_APD_LOOKUP;
	sysmsg.syscall.data.apd.lookup.address = (void *)address;
	sysmsg.syscall.data.apd.lookup.minrights = minrights;

	l4_ipc_call(mungi_tid, L4_IPC_SHORT_MSG, &sysmsg.msg, 
			L4_IPC_SHORT_MSG, &sysmsg.msg, L4_IPC_NEVER, &result);

	return sysmsg.sysret.apd.lookup.cap_addr;
}



#endif /* KERNEL_CLOCK */

static void genclock_init(void);

static void
genclock_init(void)
{
	debug("genclock_init called\n" );
	pause();

	startval[1] = 2;
}


static inline void 
genclock_start( uint32_t value );

inline void 
genclock_start( uint32_t value )
{
	*startval = value;
	startval[1] = 2;  /* reset the counter */

	debug("genclock_start called\n" );
	pause();
}

static inline int genclock_stop( uint32_t value, int p );

static inline int
genclock_stop( uint32_t value, int p )
{
	int k = startval[1];

	debug( "startval is 0x%p\n", startval );
	debug( "k is %d\n", k );
	pause();

	startval[k] = value - *startval;
	startval[k+1] = p;

	debug( "interim" );
	pause();

	/* increment the position pointer */
	startval[1] += 2;

	debug( "outerim" );
	pause();

	return value;

}


void 
_clock_set_dump_enabled( int e )
{
	dump_enabled = e;
}


int 
find_clock_entry( int p )
{
	int x;

	for( x = 2; x < startval[1]; x += 2 )
		if( startval[x+1] == p )
			return x;

	/* could have been a boonky run */
	return 0;
}

void
swap( int *x1, int *x2 )
{
	int tmp;

	tmp = *x2;
	*x2 = *x1;
	*x1 = tmp;
}

#endif

/* ============================= CLOCK undef'ed ============================= */
#if !defined(CLOCK)



/* ================================== MIPS ================================== */
#elif defined(MIPSENV)

#ifdef KERNEL_CLOCK

void 
_clock_init(void)
{

	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
	int r;

	debug( "Initting in kernel\n" );
	pause();

	/* get gt mapping */
	msg.reg[0] = SIGMA0_DEV_MAP;
	msg.reg[1] = GT_BASE_ADDR;
	r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE), &msg,
		L4_IPC_NEVER, &result);
	assert(r == 0 && result.md.fpage_received);

	/* don't want to get interrupts */
	gtval32set(GT_INT_CAUSE, ~GT_INT_TIM0EXP);
	gtval32set(GT_INT_CPU_MASK, gtval32(GT_INT_CPU_MASK) & ~GT_INT_TIM0EXP);


	clock_init_kernel();
	genclock_init();

	debug( "kernel clock init done!\n" );
}

#else

void 
_clock_init(void)
{

	l4_ipc_reg_msg_t msg;
	l4_msgdope_t result;
	int r;
	l4_threadid_t mpagerid, exc;
	uint64_t dummy;

	debug( "Initting in user-land\n" );
	pause();

	exc = mpagerid = L4_INVALID_ID;
	l4_thread_ex_regs(l4_myself(), -1LL, -1LL, &exc,
				&mpagerid, &dummy, &dummy);

	/* get gt mapping */
	msg.reg[0] = SIGMA0_DEV_MAP;
	msg.reg[1] = GT_BASE_ADDR;
	r = l4_ipc_call(mpagerid, L4_IPC_SHORT_MSG, &msg,
		L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE), &msg,
		L4_IPC_NEVER, &result);
	assert(r == 0 && result.md.fpage_received);

	/* don't want to get interrupts */
	gtval32set(GT_INT_CAUSE, ~GT_INT_TIM0EXP);
	gtval32set(GT_INT_CPU_MASK, gtval32(GT_INT_CPU_MASK) & ~GT_INT_TIM0EXP);

	/* do the mungi thing */
	clock_init_user();

	/* generic init */
	genclock_init();
}
#endif

void 
_clock_start(void)
{
	static int started = 0;

	gtval32set(GT_TIMER0, GT_TIMER_INTERVAL);
	gtval32set(GT_TIMER_CNTRL, GT_TIM0_EN | GT_TIM0_CNT);
	started = 1;

	debug( "started\n" );
	pause();

	genclock_start( GT_TIMER_INTERVAL );
}

/* store current time into array with id p */
uint32_t 
_clock_stop(int p)
{
	uint32_t timer;

	debug( "checking clock\n" );
	pause();

 	timer = gtval32(GT_TIMER0);

	debug( "clock checked: %ud\n", timer );
	pause();
	
	genclock_stop( timer, p );

	debug( "genclock stopped\n" );
	pause();

	return GT_TIMER_INTERVAL - timer;
}


/* ================================= ALPHA ================================== */
#elif defined(ALPHAENV)

#ifdef KERNEL_CLOCK
void
clock_init(void)
{
	clock_init_kernel();
	genclock_init();
}

#else
void 
clock_init(void)
{
	clock_init_user();
	genclock_init();
}
#endif  /* KERNEL_CLOCK */

void 
_clock_start(void)
{
	genclock_start( read_pcc() );
}

/* stop timer, print message, return time since started */
uint32_t 
_clock_stop(int p)
{
	return genclock_stop( read_pcc(), p );
}

#else
#error Unsupported architecture.
#endif

