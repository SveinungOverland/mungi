/****************************************************************************
 *
 *      $Id: ramdisk.c,v 1.6.2.1 2002/08/30 06:00:05 cgray Exp $
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

#include "vm/ramdisk.h"
#include "vm/startup.h"
#include "vm/vm.h"
#include "vm/vm_types.h"
#include "mungi/klibc.h"

/* turn off output */
#define kprintf(x...) (void)0 

#define RAMPAGER_MAGIC 4242
#define MAX_SAVED_MESSAGES 10

void write_request(void);
void read_request(void);
void ramdisk_init(void);
void get_msg(void);
void reply_msg(void);

extern l4_kernel_info *k;
extern uintptr_t mem_top,mem_bottom;

char *disk_base;   /* base of 'disk' region */
char *region_base; /* base of the memory we own */
uint64_t *blocks;       /* the block translation table */
uint64_t total_blocks;  /* total number of blocks */

l4_ipc_reg_msg_t msg;
l4_msgdope_t result;
l4_threadid_t sender;

void
ramdisk_main(void){ 
        
        ramdisk_init();

        get_msg();

        while(1){
                switch (msg.reg[0]){
                case READ_REQUEST:
                        read_request();
                        break;
                case WRITE_REQUEST:
                        write_request();
                        break;
                case URGENT:
                        kprintf("RDsk: Got an urgent request - yawn!\n");
                        get_msg();
                        break;
                default:
                        kprintf("Unknown msg - ignored\n");
                        get_msg();
                }
        } 
}

void
get_msg(void){
        l4_ipc_wait(&sender,L4_IPC_SHORT_MSG,&msg,L4_IPC_NEVER,&result);
}

void 
reply_msg(void){
        l4_ipc_reply_and_wait(sender,L4_IPC_SHORT_MSG,&msg,
                        &sender,L4_IPC_SHORT_MSG,&msg,
                        L4_IPC_NEVER,&result);
}

void 
write_request(void){
        uint64_t req;
        uint64_t i;
        uint64_t max_req;
        uint64_t blocknumber;
        uint64_t frame,page;
        struct dirty_list *list;

        list = (struct dirty_list *)msg.reg[2];
        max_req = msg.reg[3];

        for (req = 0 ; req < max_req ; req ++){
                frame = list[req].frame;
                page = list[req].page; 
                blocknumber = -1;
                
                for (i = 0 ; i < total_blocks ; i ++){
                        if (blocks[i] == page){
                                blocknumber = i;
                        } else if (blocks[i] == -1 && blocknumber == -1){
                                blocknumber = i;
                        }
                }

                if (blocknumber == -1){
                        kprintf("RDsk: Out of diskspace\n");
                        continue;
                }
                
                memcpy((void *)((blocknumber << L4_LOG2_PAGESIZE) + disk_base),
                                (void *)(frame << L4_LOG2_PAGESIZE),
                                L4_PAGESIZE);
        }
}

void 
read_request(void){
        uint64_t blocknumber;
        uint64_t destframe;
        uint64_t i;
        
        blocknumber = msg.reg[2];
        destframe = msg.reg[3];

        kprintf("RDsk: Read - looking for %lx\n",blocknumber); 

        for (i = 0 ; i < total_blocks ; i ++){
                if (blocks[i] == blocknumber)
                        break;
        }
        if (i != total_blocks){
                memcpy((void *)(destframe << L4_LOG2_PAGESIZE),
                                (i << L4_LOG2_PAGESIZE) + disk_base,
                                L4_PAGESIZE);
        }

        msg.reg[0] = READ_REPLY;

        reply_msg();
}


void 
ramdisk_init(void){
        uint64_t rsv; /* How many blocks to reserve for data */
        uint64_t i;
        
        kprintf("RDsk: Init\n");
        kprintf("RDsk: Myid is %llx\n",(long long)l4_myself().ID);

        /* Where do live */
        region_base = (char *)(mem_top + 2 * L4_PAGESIZE); 
        
        /* how big... (its all about size) */
        total_blocks = ((char *)k->kernel_data - region_base) / L4_PAGESIZE; 
        total_blocks --;  /* One for the 'superblock' */

        rsv = total_blocks / (L4_PAGESIZE / sizeof(uint64_t));

        total_blocks -= rsv;
        kprintf("RDsk: %ld blocks: base %p\n",total_blocks,blocks);
         
        /* the block table */ 
        blocks = (uint64_t *)(void *)(region_base + TABLE_OFFSET);
        disk_base = region_base + (rsv * L4_PAGESIZE);
        
        if (*(uint64_t *)(void *)(region_base + MAGIC_OFFSET) == RAMPAGER_MAGIC){
                /* Restarting - ram should be there */
        } else {
                kprintf("RDsk: Formating\n");
                for (i = 0 ; i < total_blocks ; i ++){
                        blocks[i] = -1;
                }
                kprintf("RDsk: ...formatted\n");
                *(uint64_t *)(void *)(region_base + MAGIC_OFFSET) = RAMPAGER_MAGIC;
        }     

        l4_ipc_send(startupid,L4_IPC_SHORT_MSG,&msg,L4_IPC_NEVER,&result); 

}
