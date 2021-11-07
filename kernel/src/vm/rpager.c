/****************************************************************************
 *
 *      $Id: rpager.c,v 1.33.2.1 2002/08/30 06:00:03 cgray Exp $
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

#include "mungi/l4_generic.h"
#include "mungi/klibc.h"
#include "dit.h"
#include "sys/types.h"
#include "assert.h"

#include "mungi/types.h"
#include "mungi/mm.h"
#include "vm/ramdisk.h"
#include "vm/startup.h"
#include "vm/vm_types.h"
#include "vm/vm.h"
#include "vm/ptable.h"

//#define ADBG
#ifdef ADBG
//#define DEBUG
#define debug(x...) kprintf(x)
#define pause()	l4_ipc_sleep(L4_IPC_TIMEOUT(0,0,2,6,0,0), &result)
#else
#define debug(x...)
#define pause() (void)(0)
#endif


static freelist _rpfl;
freelist *rpager_freelist = &_rpfl;

/* The Stacks for a few threads - _stack is this thread! */
uintptr_t _stack[STACK_SIZE];
uintptr_t *_sp = &_stack[STACK_SIZE - 1];	/* stack address used by crtS.o */

/*
 * Various threads id.  Accesable globally.  May not be valid until startup
 * finishes!
 */
l4_threadid_t rpagerid;		/* RAM pager (this thread) id */
l4_threadid_t mpagerid;		/* mungi pager id */
l4_threadid_t apagerid;		/* asynchronous pager id */
l4_threadid_t mprid;		/* smarter filesystem */
l4_threadid_t adiskid;		/* asynchronous disk */
l4_threadid_t kprintfdid;       /* handles the printf */
l4_threadid_t startupid;	/* starts up other threads */

/* 
 * Flag indicating availability of file system.
 * IO_NONE means no disk 
 */
extern int io_status;

/* 
 * Some data structures used by the rpager 
 */
extern pending_pf_t *ppftable;	/* The queue of pending page faults */
extern l4_kernel_info *k;	/* The kernel info page */
Dit_Dhdr *dit_hdr;		/* dit header */

/* External MPR functions - FIXME: should be in mpr headers */
void mprd(void);
void mpr_disk(void);

/* Internal functions */
static void map_page(l4_threadid_t faulter,l4_threadid_t faultfor, 
                page_table_t *pte, int rw);
static void basic_mapping_only(void);
static void otherthrd_init(void);

static void handle_mpr_message(void);
static void handle_special_mappings(void);
static void request_page(uintptr_t fault_page,int ppfoffset);
static void ppf_map(int off,uintptr_t page,uintptr_t frame);

/* 
 * These are shared by most of the functions in this file
 *
 * They represent the current fault being handled.
 */
static l4_ipc_reg_msg_t msg;	/* L4 msg */
static l4_threadid_t    faulter; /* Who sent the fault message */
static int req_done = TRUE;	/* flag: Have we serviced the request yet? */
static l4_msgdope_t result;     /* result of last message */

/* 
 * otherthrd_init
 * 
 * Initialises the debug and startup threads.
 */
static void otherthrd_init(void)
{
	l4_threadid_t excpt, page;
	uintptr_t oip, osp;

	/* Who am I? */
	rpagerid = l4_myself();

	/* Get kernel info & DIT header page */
	msg.reg[0] = SIGMA0_KERNEL_INFO_MAP;
	l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		    L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		    &msg, L4_IPC_NEVER, &result);
	k = (l4_kernel_info *) (msg.reg[0] & L4_PAGEMASK);
	dit_hdr = (Dit_Dhdr *) (k->dit_hdr);

        /* 
         * start debugd thread.
         * This does all the printing.  Hopefully this solves all the sync
         * problems on debug.  Also starting it early means all threads
         * can use it immediately.
         */
        kprintfdid = rpagerid;
        kprintfdid.id.lthread = 2;      /* current thread is 0 */
        excpt.ID = -1LL;
        page.ID = -1LL;

        l4_thread_ex_regs(kprintfdid,
                          (uintptr_t) kprintfd,
                          (uintptr_t) & kprintfd_stack[STACK_SIZE - 1],
                          &excpt, &page, (void *)&oip, (void *)&osp);
        debug("rpager: My ID is 0x%lx\n", rpagerid.ID);
        debug("rpager: 0x%p\n", k);
        debug("rpager: kprintfd started\n");

	/* 
	 * start startup thread.
	 */
	startupid = rpagerid;
	startupid.id.lthread = 1;	/* current thread is 0 */
	excpt.ID = -1LL;
	page.ID = -1LL;

	l4_thread_ex_regs(startupid, (uintptr_t) startup,
			  (uintptr_t) & startup_stack[STACK_SIZE - 1],
			  &excpt, &page, (void *)&oip,(void *)&osp);

	debug("rpager: startup started\n");
}


/*
 * This is the RAM pager for the Mungi server.
 * This starts up the startup thread because the RAM pager needs
 * to be lthread0 in order to correctly intercept Mungi device (and other
 * special) mapping requests sent to Sigma0.
 *
 * Note that (saved) variables are saved in the pending page fault tables to
 * be restored later
 */
int main(void)
{
	l4_threadid_t faultfor;	/* Who _really_ faulted     (saved) */
	access_t access;	/* read/write/execute flags (saved) */
	page_table_t *pte;	/* the page table entry */
	uint32_t fault_page;	/* What logical page the fault was on */
	uintptr_t addr;		/* Virtual address of fault */
        int ppfoffset;          /* offset for saved state */
	int r;

	/* Start the other threads */
	otherthrd_init();

	/* Wait for the startup thread to send me a message - 
	 * this means all the data structures should be inited 
	 * and we can handle page faults safely */
	debug("rpager: waiting for the startup thread\n");
	r = l4_ipc_receive(startupid, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
			   &result);
	debug("rpager: startup indicated it ready - handling faults!\n");

	if (io_status == IO_NONE) {
		basic_mapping_only();	/* never returns */
	}

	/*
	 * The Main Pager Loop 
	 */
	for (;;) {
                debug("rpager: processing next message\n");
		/* Do we need to get a new request? */
		if (req_done) {
                        debug("rpager: waiting for a message\n");
			r = l4_ipc_wait(&faulter, L4_IPC_SHORT_MSG, &msg,
					L4_IPC_NEVER, &result);
			assert(r == 0);
			req_done = FALSE;
		}

		if (faulter.ID == mprid.ID) {
			handle_mpr_message();
			req_done = TRUE;
			continue;
		}

                /* Check here if it is a page fault from mpager or
		 * a msg. from mpager on behalf of another thread's
		 * fault - ceg
		 */
		if (msg.reg[0] == MPAGER_MAGIC &&
                                msg.reg[1] == MPAGER_MAGIC) {
			faulter.ID = msg.reg[2]; /* should be apager */
			msg.reg[0] = msg.reg[5]; /* set ip & bva back */
			msg.reg[1] = msg.reg[6];
		}

                
		/* It must be a request from userland */
		addr = msg.reg[0] & (uintptr_t) L4_PAGEMASK;
		fault_page = msg.reg[0] >> L4_LOG2_PAGESIZE;
		access = msg.reg[0] & L4_FPAGE_RW_MASK;
		faultfor.ID = msg.reg[3];

		debug("#### Page fault from 0x%016lx\n", faulter.ID);
		debug("\treg[0] = 0x%016lx (addr)\n", msg.reg[0]);
		debug("\treg[1] = 0x%016lx (ip)\n", msg.reg[1]);
		debug("\treg[2] = 0x%016lx \n", msg.reg[2]);
		debug("\treg[3] = 0x%016lx \n", msg.reg[3]);

		if (fault_page == 0) {
                        /* FIXME:? Should try and kill the process?
                         * However any OS on top should not pass it through.
                         * Else the OS has died... */
			debug("NULL pointer dereference - ignoring\n");
			debug("Fault ip is 0x%016lx\n", msg.reg[1]);
                        req_done = TRUE;
			continue;
		}

		/* special pages */
		if (fault_page == -1) {
			debug("rpager: Going to special mappings handler\n");
			handle_special_mappings();
			req_done = TRUE;
			continue;
		}

		pte = find_pte(fault_page);

		debug("RP: Fault page: 0x%x : %s fault\n", fault_page,
		      access ? "Write" : "Read");
		debug("RP: pte is %p\n", pte);

                if (pte == 0) {
                        ppfoffset = save_state(faulter,faultfor,
                                        fault_page,access);
                        req_done = TRUE;
                        if (ppfoffset != ALREADY_REQUESTED)
                                request_page(fault_page,ppfoffset);
			continue;
		} else {
			map_page(faulter,faultfor, pte, access);
			req_done = TRUE;
			continue;
		}

	}
}

static void 
map_page(l4_threadid_t mpfaulter,l4_threadid_t mpfaultfor,
                page_table_t * pte, int rw){
        volatile int x;
        
        assert(pte != NULL);
	set_ft_bit_nolock(pte->up.framenum, FT_REFERENCE);
        
	if (rw) {
		if (ftable[pte->up.framenum].cow == COW_PAGE) {
			/* FIXME: Should Handle this gracefully */

			debug("RP: COW_PAGE things go to hell now!\n");
			assert(!"COW_PAGE?? Copy on Write not implemented\n");
		}
		ftable[pte->up.framenum].dirty = DIRTY;
		set_ft_bit_nolock(pte->up.framenum, FT_DIRTY);
	}
     
        /* touch it */
        assert(pte->up.framenum != 0);
        x = *(int *)(uintptr_t)(pte->up.framenum << L4_LOG2_PAGESIZE);

        msg.reg[0] = ((uint64_t)((uintptr_t) pte->up.pagenum) << L4_LOG2_PAGESIZE);
	msg.reg[1] =
	    (uintptr_t) l4_fpage(((uintptr_t) pte->up.framenum) <<
				 L4_LOG2_PAGESIZE, L4_LOG2_PAGESIZE,
				 (ftable[pte->up.framenum].dirty =
				  DIRTY) ? L4_FPAGE_RW : L4_FPAGE_RO,
				 L4_FPAGE_MAP).fpage;
	msg.reg[2] = mpfaultfor.ID;
	msg.reg[3] = 0;

	debug("RP: sending: msg.reg[0]: %lx\n", msg.reg[0]);
	debug("RP: sending: msg.reg[1]: %lx\n", msg.reg[1]);
	debug("RP:MAP_PAGE: mapping Page 0x%lx: Frame 0x%x\n", pte->up.pagenum,
	      (unsigned int)pte->up.framenum);
	debug("RP: sending to %lx\n", mpfaulter.ID);
	l4_ipc_send(mpfaulter, L4_IPC_SHORT_FPAGE, &msg, L4_IPC_NEVER,
		    &result);
}

/*
 * handle_mpr_message
 *
 * Handles a message from the MPR.  Currently handles two types of messages
 *  - A request for more frames
 *  - Notification that a read request has been completed successfully
 */
static void handle_mpr_message(void)
{
        int ppfoffset;
        uintptr_t VPN;
        uintptr_t frame;

        debug("rpager: message from mpr\n");

	switch (msg.reg[0]) {
	case READ_REPLY:
                ppfoffset = msg.reg[1];
                VPN = msg.reg[2];
                frame = msg.reg[3]; 

                ppf_map(ppfoffset,VPN,frame);

		break;
	default:
		debug("Invalid message from mpr\n");

	}

}

static void
ppf_map(int off,uintptr_t page,uintptr_t frame){
	page_table_t *pte;
        pending_pf_t *pf;

        debug("rpager: off %d, page %lx, frame %lx\n",off,page,frame);
        
        /* Sanity check */
        if (off > (2*MAX_PF) || off < 0){
                return;
        }

        pf = &ppftable[off];

        
        /* Map those pages */
        /* FIXME: Doesn't clean up ppf table corretly */
        while (pf != NULL){
                if  (pf->fault_page == page){
                        pte = add_pte(page,frame,ppftable[off].access);
                        map_page(pf->faulter,pf->faultfor,pte,DIRTY);
                }
                pf = pf->collision;
        }
}

/*
 * request_page
 *
 * Sends a read request for a particular page to the mpr
 *
 * Also allocates a 1 frame for the mpr
 */ 
static void request_page(uintptr_t fault_page,int ppfoffset){
        int r;
        
        debug("rpager: read %lx, ref %d\n",fault_page,ppfoffset);
        
        msg.reg[0] = READ_REQUEST;
        msg.reg[1] = ppfoffset;
        msg.reg[2] = fault_page;
        msg.reg[3] = alloc_frame(fault_page,0,0);

        debug("rpager: request is [0]: %8llx\n",(long long)msg.reg[0]);
        debug("rpager: request is [1]: %8llx\n",(long long)msg.reg[1]);
        debug("rpager: request is [2]: %8llx\n",(long long)msg.reg[2]);
        debug("rpager: request is [3]: %8llx\n",(long long)msg.reg[3]);

        /* FIXME: Currently using a call instead of reply_and_wait
         *      Since there is no chance of the mpr blocking 
         *      there is no problem an dit avoid a potential deadlock
         *      This needs to be fixed for a real disk (with latency)
         */
        r = l4_ipc_call(mprid, L4_IPC_SHORT_MSG,&msg, 
                                L4_IPC_SHORT_MSG, &msg, 
                                L4_IPC_NEVER, &result);
        faulter.ID = mprid.ID;
        debug("rpager: got response\n");
	req_done = FALSE;
}


/* 
 * handle_special_mappings
 *
 * Handle special mappings - kernel info, dit header, device mappings and
 * the like.  
 */
static void
handle_special_mappings(void) {
#ifdef MIPSENV
	if (msg.reg[0] == SIGMA0_DEV_MAP) {
		/* Okay its a device request - hopefully its valid 
		 * We are just going to pass it on, and back up again */
		debug("rpager: SIGMA0_DEV_MAP request\n");
		l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
				L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
				&msg, L4_IPC_NEVER, &result);
		msg.reg[1] =
		    (uintptr_t) l4_fpage(msg.reg[0], L4_LOG2_PAGESIZE,
					 L4_FPAGE_RW, L4_FPAGE_MAP).fpage;
		msg.reg[2] = 0; /* FIXME: should be the faultfor */
		msg.reg[3] = 0;
		l4_ipc_send_deceiving(faulter, SIGMA0_TID,
				      L4_IPC_SHORT_FPAGE, &msg,
				      L4_IPC_NEVER, &result);
                req_done = TRUE;
	} else
#endif				/* MIPSENV */
	if (msg.reg[0] == SIGMA0_KERNEL_INFO_MAP) {
		debug("rPager: request for kinfo (0x%p)\n", k);
		msg.reg[0] = (uintptr_t) k;
		msg.reg[1] = (uintptr_t) l4_fpage((uintptr_t) k,
						    L4_LOG2_PAGESIZE,
						    L4_FPAGE_RO,
						    L4_FPAGE_MAP).fpage;
		msg.reg[3] = 0;
                debug("rpager: sending kinfo to %lx\n", faulter.ID);
		l4_ipc_send(faulter, L4_IPC_SHORT_FPAGE, &msg,
				L4_IPC_NEVER, &result);
                debug("Sent\n");
                req_done = TRUE;
	}
}

void basic_mapping_only(void)
{
	uint64_t addr;
	int r;

	debug("rpager: 1:1 mapping only\n");

	while (1) {
		l4_ipc_wait(&faulter, L4_IPC_SHORT_MSG, &msg, L4_IPC_NEVER,
			    &result);
		if (msg.reg[0] == SIGMA0_KERNEL_INFO_MAP) {
			debug("rPager: request for kinfo (k = 0x%p)\n", k);
			msg.reg[0] = (uintptr_t) k;
			msg.reg[1] = (uintptr_t) l4_fpage((uintptr_t) k,
							  L4_LOG2_PAGESIZE,
							  L4_FPAGE_RO,
							  L4_FPAGE_MAP).fpage;
			msg.reg[3] = 0;
			debug("\treplying to: 0x%016lx\n", faulter.ID);
			debug("\tmsg.reg[0] is 0x%p\n", k);
			r = l4_ipc_send(faulter, L4_IPC_SHORT_FPAGE, &msg,
					L4_IPC_NEVER, &result);

			assert(r == 0);
			debug("\tkinfo mapped\n");
			continue;

#ifdef MIPSENV
		} else if (msg.reg[0] == SIGMA0_DEV_MAP) {
			/* Okay its a device request */
			debug("rPager: SIGMA0_DEV_MAP request\n");
			addr = msg.reg[0];
			r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
					L4_IPC_MAPMSG(0,
						      L4_WHOLE_ADDRESS_SPACE),
					&msg, L4_IPC_NEVER, &result);
			assert(r == 0);
			msg.reg[0] = addr;
			msg.reg[1] =
			    (uintptr_t) l4_fpage(msg.reg[0],
						 L4_LOG2_PAGESIZE,
						 L4_FPAGE_RW,
						 L4_FPAGE_MAP).fpage;
			msg.reg[3] = 0;

			debug("\treplying to: 0x%016lx\n", faulter.ID);
			r = l4_ipc_send_deceiving(faulter, SIGMA0_TID,
						  L4_IPC_SHORT_FPAGE, &msg,
						  L4_IPC_NEVER, &result);
			assert(r == 0);
			debug("\tdev mapped\n");
			continue;

#endif
		}

		r = msg.reg[0] & L4_FPAGE_RW;
		msg.reg[0] &= L4_PAGEMASK;
		addr = msg.reg[0];

		if (addr == 0) {
			debug("NULL pointer dereference - silently ignoring\n");
			continue;
		}
		//debug("\trequest from user land, fault bit is %x\n", r);

		if (r)		/* write fault */
			*(volatile int *) msg.reg[0] =
			    *(volatile int *) msg.reg[0];
		else		/* read fault */
			*(volatile int *) msg.reg[0];
		//debug("\ttouched page\n");

		msg.reg[0] = addr;
                msg.reg[1] = 
                        (uintptr_t)L4_IPC_MAPMSG(msg.reg[0]|r,L4_LOG2_PAGESIZE);
		msg.reg[3] = 0;
		msg.reg[4] = PAGER_MAGIC;

		r = l4_ipc_send(faulter, L4_IPC_SHORT_FPAGE, &msg,
				L4_IPC_NEVER, &result);
		assert(r == 0);

		//debug("\tsent page to 0x%016llx\n", faulter.ID);
	}
	debug("rpager: FATAL: Fell of the end of basic mapping only!\n");
	assert(!"Should not be here!");
}

