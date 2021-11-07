/****************************************************************************
 *
 *      $Id: vm.c,v 1.24.2.1 2002/08/30 06:00:04 cgray Exp $
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

#include "mungi/l4_generic.h"	/* Instead of all the <l4/ *.h> files */

#include "dit.h"

#include "vm/vm_types.h"
#include "vm/mutex.h"

#include "vm/vm.h"
#include "mungi/types.h"
#include "mungi/klibc.h"
#include "mungi/kernel.h"	// Needed for assert...

#define SERIAL			// define if serial drive in DIT image
//#define ADBG

#ifdef ADBG
#define PT_DBG
#endif

#ifdef ADBG
#include "vm/kprintf.h"
#define pause()		l4_ipc_sleep(L4_IPC_TIMEOUT(0,0,2,6,0,0), &result)
#define debug(x...) kprintf(x)
#else
#define debug(x...)
#endif

#ifdef PT_DBG
#include "vm/vm_dbg.h"
#endif

/* function prototypes */
static void ptable_init(void);
static void ftable_init(void);
static void ppftable_init(void);
void debug_lvl(int);


/* FIXME: Too many globals! */
static unsigned int mem_size;	/* available RAM, in frames */
unsigned int ptable_size;	/* page table size, in bytes  */
unsigned int ctable_size;	/* collision table size, in bytes */
unsigned int ftable_size;	/* frame table size, in bytes  */
static unsigned int ds_size;	/* total data structures size, in frames */
page_table_t *ptable;		/* page table */
page_table_t *ctable;		/* collision table */
frame_table_t *ftable;		/* frame table */
uintptr_t fbase;		/* base frame */
pending_pf_t *ppftable;		/* pending page fault table */

extern l4_kernel_info *k;
extern Dit_Dhdr *dhdr;

extern freelist *rpager_freelist;

uint64_t mem_top, mem_bot;

void 
vm_init(void){
        uintptr_t frame;
        page_table_t *pte;

	debug("K: 0x%016llx\n", k);

	/* work out some sizes */
	/* The top section is for device drivers */
	mem_top = k->kernel_data / 2;
	mem_bot = dhdr->d_vaddrend;
	mem_size = (mem_top - mem_bot) >> L4_LOG2_PAGESIZE;
	ptable_size = (mem_top >> L4_LOG2_PAGESIZE) * sizeof(page_table_t);
        ctable_size = ((mem_top >> L4_LOG2_PAGESIZE)  >> CTABLE_SHIFT) 
                * sizeof(page_table_t);
        ftable_size = (mem_top >> L4_LOG2_PAGESIZE) * sizeof(frame_table_t);
	fbase = (((Dit_Phdr *) ((uintptr_t) dhdr + dhdr->d_phoff))->p_base)
	    >> L4_LOG2_PAGESIZE;

	debug("vm_init: avail RAM = %d frames\n", mem_size);
	debug("vm_init: new top of ram is at 0x%llx\n", mem_top);
	debug("vm_init: ptable size = %d bytes, ftable size = %d bytes\n",
	      ptable_size, ftable_size);
	debug("vm_init: ctable size = %d bytes\n", ctable_size);
	debug("vm_init: ptable entries = %d, ftable entries = %d\n",
	      MAX_PTABLE, MAX_FTABLE);
	debug("vm_init: ctable entries = %d\n", MAX_CTABLE);
	debug("vm_init: base frame is 0x%x\n", fbase);

	/* place data structures 
	 *  The extra entry in each case is for free list heads
	 *  NOTE: Be careful of alignment when rearranging 
	 *  the order of the data structures
	 */
	ptable = (page_table_t *) (mem_top - ptable_size);
	ctable =
	    (page_table_t *) ((uintptr_t) ptable - ctable_size -
			      sizeof(page_table_t));

	/* make sure frame table doesn't share any frames with p/ctables */
	ftable = (frame_table_t *) (((uintptr_t) ctable & L4_PAGEMASK)
				    - ftable_size - sizeof(frame_table_t));
	ppftable = (pending_pf_t *) ((uintptr_t) ftable -
			      2 * MAX_PF * sizeof(pending_pf_t));

	ds_size =
	    ((mem_top - (uintptr_t) ppftable) + (L4_PAGESIZE - 1))
	    >> L4_LOG2_PAGESIZE;

	debug("vm_init: ptable at 0x%llx, ftable at 0x%llx\n", ptable,
	      ftable);
	debug("vm_init: ctable at 0x%llx\n", ctable);
	debug("vm_init: ppftable at 0x%llx\n", ppftable);
	debug("vm_init: total data structure size = %d frames\n", ds_size);

	/* initialise data structures */
	debug("Ptable init\n");
	ptable_init();
	debug("ftable init\n");
	ftable_init();
	debug("ppftable init\n");
	ppftable_init();
        
        /* add the tables into the page table */
	debug("\e[33;1m GETTING MORE FRAMES!\e[0m\n" );
	frame = (mem_top >> L4_LOG2_PAGESIZE) - ds_size;
	debug("frame  : 0x%llx\n", frame );
	debug("mem_top: 0x%llx\n", mem_top >> L4_LOG2_PAGESIZE );
	while(frame < (mem_top >> L4_LOG2_PAGESIZE)){
                pte = add_pte(frame,frame,DIRTY);
                ftable[pte->up.framenum].osrsrvd = OSRSRVD;
                frame++;
	}

	debug("Done\n");
	//pt_dump(ptable, MAX_PTABLE, ctable, MAX_CTABLE);

}

static void 
ptable_init(){
	unsigned int i;
	Dit_uint addr = 0;
        Dit_Phdr *phdr;
        uintptr_t frame;
        page_table_t *pte;

        /* free list not required for ptable because it is a hash table */
        debug("startup: Init ptable\n");
	for (i = 0; i < MAX_PTABLE; i++) {
		ptable[i].up.valid = INVALID;
		ptable[i].up.chain = CTABLE_LIST_END;
	}
        /* free list for ctable is a linked list of free entries */
        debug("startup: init collision table\n");
	for (i = MAX_CTABLE; i > 0; --i) {
		ctable[i].fp.valid = INVALID;
		ctable[i].fp.next = i-1;
	}
	ctable[0].fp.valid = INVALID;
	ctable[0].fp.next = CTABLE_LIST_END;

	/* put DIT pages into ptable */
	/* Dit Header (mungi wants it) */
	dhdr = (Dit_Dhdr *)k->dit_hdr;

	addr = (Dit_uint)(uintptr_t)dhdr;
	pte = add_pte((addr >> L4_LOG2_PAGESIZE), (addr >> L4_LOG2_PAGESIZE),
		CLEAN);
        ftable[pte->up.framenum].osrsrvd = OSRSRVD;
        
	/* Ignore everything in the image before Mungi.
	   - On Alpha it is (hopefully) the kernel & SIGMA0
	   - On MIPS it is the serial driver */
	for (phdr = (Dit_Phdr *) ((uintptr_t) dhdr + dhdr->d_phoff), i = 0;
	     i < dhdr->d_phnum; phdr++, i++) {
		if (strcmp(phdr->p_name, "mungi.kernel") == 0)
			break;
		debug("startup: ignoring: (%d) '%s' @ %p\n", i,
		      phdr->p_name, phdr->p_name);
		debug("startup: phnum is %d\n", dhdr->d_phnum);
	}
        if (strcmp(phdr->p_name, "mungi.kernel") != 0){
                debug("startup: Didn't find mungi.kernel image!\n");
                debug("startup: Goodbye Cruel world\n");
                assert(0);
        }
        /* shove everything into the page table */
        debug("Inserting DIT into page table\n");
        frame = phdr->p_base >> L4_LOG2_PAGESIZE;
        while (frame < mem_bot >> L4_LOG2_PAGESIZE){
                addr = frame << L4_LOG2_PAGESIZE;
                *(volatile int *)((uint64_t)addr) = 
                        *(volatile int *)((uint64_t)addr);
                pte = add_pte(frame,frame,DIRTY);
                ftable[pte->up.framenum].osrsrvd = OSRSRVD;
                frame ++;
        }

        frame = ((uintptr_t)k) >> L4_LOG2_PAGESIZE;
	pte = add_pte(frame,frame,DIRTY);
	ftable[pte->up.framenum].osrsrvd = OSRSRVD;
        
}

static void ftable_init()
{
	unsigned int i, j, largest = -1, ff = MAX_FTABLE;
	Dit_Phdr *phdr;
        
	for (phdr = (Dit_Phdr *) ((uintptr_t) dhdr + dhdr->d_phoff), j = 0;
	     j < dhdr->d_phnum; phdr++, j++) {
		ff = PA2IND(phdr->p_base + phdr->p_size);
		largest = (largest > ff) ? largest : ff;
		/* fill in dit frame details */
		for (i = PA2IND(phdr->p_base);
		     i < PA2IND(phdr->p_base + phdr->p_size); i++) {
			ftable[i].pagenum = IND2PF(i);
			ftable[i].reference = NOT_REFERENCED;
			ftable[i].osrsrvd = 1;
			ftable[i].dirty = DIRTY; /* dit not on disk at startup */
			ftable[i].lock = 0;
		}
	}

	/* initialise frame table free list down to DIT */
        for (i = mem_bot >> L4_LOG2_PAGESIZE ; i < MAX_FTABLE; i ++){
		ftable[i].freelist = i + 1;
		ftable[i].pagenum = 0;
		ftable[i].reference = 0;
		//ftable[i].osrsrvd = 0;
		ftable[i].dirty = 0;
		ftable[i].cow = 0;
		ftable[i].lock = 0;
	}
	ftable[MAX_FTABLE - 1].freelist = FTABLE_LIST_END;

	/* mark data structure frames as used */
        /* NOTE: offset of 1 in loop */
	for (i = 1; i <= ds_size; i++) {
		/* the data structures start from the highest frame and work
		 * downwards 
		 */
		ftable[MAX_FTABLE - i].osrsrvd = OSRSRVD;
	}

	debug("init rpager_freelist\n");
	debug("rpager_freelist = %p\n", rpager_freelist);
        rpager_freelist->front = mem_bot >> L4_LOG2_PAGESIZE;
	rpager_freelist->back = MAX_FTABLE - i;
	ftable[rpager_freelist->back].freelist = FTABLE_LIST_END;
}

/* Initialises pending page fault table */
static void ppftable_init(void) {
	int i;

	for (i = 0; i < MAX_PF * 2; i++) {
                ppftable[i].faulter = L4_NIL_ID;
                ppftable[i].faultfor = L4_NIL_ID; 
                ppftable[i].fault_page = 0;
                ppftable[i].access = 0;
                ppftable[i].valid = 0;
                ppftable[i].collision = NULL;
                
	}
}


unsigned int alloc_frame_off_list(freelist *fl) {
        int find;

        do {
                if(fl->front == FTABLE_LIST_END) {
                        return NO_FREE_FRAMES;
                }
                find = fl->front;
                bittestandset(&ftable[find], FT_LOCK_MASK);
                fl->front = ftable[find].freelist;

                ftable[find].lock = 0;
                
        } while (ftable[find].osrsrvd != 0);

        return find;
}

void add_frame_to_list(freelist *fl, unsigned int pfno) {
        unsigned int oldback;
        if(fl->front == FTABLE_LIST_END) {
                /* shouldn't happen! */
                ftable[pfno].freelist = FTABLE_LIST_END;
                fl->front = pfno;
                fl->back = pfno;
                return;
        }
        bittestandset(&ftable[fl->back], FT_LOCK_MASK);
        oldback = fl->back;
        ftable[oldback].freelist = pfno;
        ftable[pfno].freelist = FTABLE_LIST_END;
        fl->back = pfno;
        ftable[oldback].lock = 0;
}

void add_to_free_list(unsigned int pfn) {
        add_frame_to_list(rpager_freelist, pfn);
}

unsigned int 
alloc_frame(unsigned int vpno, unsigned int osrsrvd, unsigned int dirty){
        unsigned int find;

        /* ask for one off the rpager's free list */
        find = alloc_frame_off_list(rpager_freelist);

        if (find == NO_FREE_FRAMES){
		/* Out of memory - no swap :( */
                assert(find != NO_FREE_FRAMES);
        }

        /* fill in detials of new frame */
        //bittestandset(&ftable[find], FT_LOCK_MASK);
        ftable[find].pagenum = vpno;
        ftable[find].reference = REFERENCED;
        ftable[find].osrsrvd = osrsrvd;
        ftable[find].dirty = dirty;
    
        /* Unlock Frame */
        ftable[find].lock = 0;
	    
        return (IND2PF(find));
}


void set_ft_bit(unsigned int pfno, unsigned int mask)
{
	/* broken
	   bittestandset(&ftable[PF2IND(pfno)], FT_LOCK_MASK);
	   (uint64_t)(ftable[PF2IND(pfno)]) |= mask;
	   ftable[PF2IND(pfno)].lock = 0;
	 */

}
void set_ft_bit_nolock(unsigned int pfno, unsigned int mask)
{
	/* broken
	   (uint64_t)(ftable[PF2IND(pfno)]) |= mask;
	 */
}

void clr_ft_bit(unsigned int pfno, unsigned int mask)
{
    /* broken 
	bittestandset(&ftable[PF2IND(pfno)], FT_LOCK_MASK);
	(uint64_t)(ftable[PF2IND(pfno)]) &= (~mask);
	ftable[PF2IND(pfno)].uf.lock = 0;
    */
}


/*
 *  force_frame
 *
 * Finds a frame in the frame table. 
 * Uses Second chance algorithm (~ LRU) to find a victim
 *
 * Does not search ff frame list.  Use alloc_frame for that
 */
unsigned int 
force_frame(unsigned int vpno, unsigned int osrsrvd, unsigned int dirty)
{
        static unsigned int findex_odd = 1;
        static unsigned int findex_even = 0;
        unsigned int findex;
        unsigned int last;

        kprintf("Force_Frame Called\n");

        if (vpno % 2)
                findex = findex_odd;
        else
                findex = findex_even;


        while (1) {
                // Increment
                last = findex;
                findex = (findex + 2) % MAX_FTABLE;

                /* Check its not used locked down */
                if (ftable[findex].osrsrvd == TRUE){
                        continue;
                }

                /* Lock the ENTRY */
                bittestandset(&ftable[findex], FT_LOCK_MASK);
                if (ftable[findex].reference == NOT_REFERENCED) {
                        /* Found a page to swap out */
                        bittestandset(&ftable[findex], FT_LOCK_MASK);
                        ftable[findex].reference = REFERENCED;
                        if (ftable[findex].dirty == DIRTY) {
                        } else {
                                // Just need to page in the new one
                        }

                        kprintf("Found a frame to swap out: %d\n",findex);

                        ftable[findex].lock = 0;
                } else {
                        /* Referenced bit is set - 
			   need to clear it and unmap it */
                        ftable[findex].reference = FALSE;
                        l4_fpage_unmap(l4_fpage(IND2PA(findex), 
                                              L4_LOG2_PAGESIZE, 1, 0), 
                                     L4_FP_OTHER_SPACES | L4_FP_FLUSH_PAGE);
                      
                        ftable[findex].lock = 0;
                }

                ftable[findex].lock = 0;

       }       
        return IND2PF(findex);
}

/*
 * save_state: 
 * Saves data into the PPF table for later restoration 
 * First searches for a valid entry then stores appropriate info in it
 * Returns offset for use with indexs
 */
int save_state(l4_threadid_t faulter, l4_threadid_t faultfor,
                uintptr_t fault_page, access_t access)
{
	int i;
        int frame_seen = 0;  /* Have we seen this frame (flag) */
        pending_pf_t *last = NULL; /* Last link in collision chain */

        i = fault_page % MAX_PF;

        if (ppftable[i].valid){
                last = &ppftable[i];
                i = MAX_PF;
                
                while (ppftable[i].valid){
                        if (ppftable[i].fault_page == fault_page)
                                frame_seen = 1;
                        i ++;
                                
                }
                last->collision = &ppftable[i];
        } 

        /* We have a free ppf entry */
        ppftable[i].valid = TRUE;
        ppftable[i].faulter = faulter;
        ppftable[i].faultfor = faultfor;
        ppftable[i].fault_page = fault_page;
        ppftable[i].access = access;
        ppftable[i].collision = NULL;

        if (last != NULL){
                last->collision = &ppftable[i];
        }
        
        if (frame_seen){
                return ALREADY_REQUESTED;
        } else {
                return i;
        }
}
