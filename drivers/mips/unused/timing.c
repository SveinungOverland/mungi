/****************************************************************************
 *
 *      $Id: timing.c,v 1.2 2002/05/31 05:10:21 danielp Exp $
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

/****************************************************************************
 *      
 *      $Id: timing.c,v 1.2 2002/05/31 05:10:21 danielp Exp $
 *	Copyright (C) 1999 Distributed Systems (DiSy) Group, UNSW, Australia.
 *
 *      This file is part of the Mungi operating system distribution.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version 2
 *      of the License, or (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *      
 ****************************************************************************/
#ifdef MIPSENV

#include "mungi/kernel.h"
#include "mungi/l4_generic.h"
#include "timing.h"


/*uint32_t * t_tarray = (uint32_t *) O_PROFILING;*/
static char * const _gt64010 = (char *)GT_BASE_ADDR;

static void initsonic(void);

/* Seedy... */
#ifndef PROTB
#define PROTB
#endif
#ifdef PROTB
void
inittimer(void)
{
  /*
   * This will set up the sonic chip so that it can be used
   * for timing. NOTE this should only be run on the indy
   */
#if defined(DIZZY) 
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = 0x1fbd9830;
  r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		       &msg, L4_IPC_NEVER, &caller_dope);
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = 0x1fbb0000; 
  r = l4_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		       &msg, L4_IPC_NEVER, &caller_dope);
#endif

  VERBOSE("Initialising the timer chip\n");
  initsonic();
}

uint32_t
byte_swap(uint32_t x)
{
  return (((x & 0x000000ff) << 24)  |
	  ((x & 0x0000ff00) <<  8)  |
	  ((x & 0x00ff0000) >>  8)  |
	  ((x & 0xff000000) >> 24));
}

static void 
initsonic(void)
{
  
  l4_msgdope_t result;
  int r;
  l4_ipc_reg_msg_t msg;
  
  /* map the gt chip registers */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = GT_BASE_ADDR;
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
                       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
                       &msg, L4_IPC_NEVER, &result);

  /* disable the timer */
  GTWX(GT_TIMER_CNTRL) = 0;

  /* set up timer, i presume the initial value */
  GTWX(GT_TIMER0) = byte_swap(-1);
  /*
  GTWX(GT_INT_CAUSE) = byte_swap(~GT_INT_TIME0EXP);
  GTWX(GT_INT_CPU_MASK) = 0;
  */
  /* enable the timer */
  GTWX(GT_TIMER_CNTRL) = byte_swap(GT_TIM0_EN | GT_TIM0_TMR);
}


uint32_t 
read_time(void)
{
  return GTWX(GT_TIMER0);
}

#else

  

#ifndef DIZZY


/* 
 * This version of init is for the Alogrithmic board 
 */

void
initsonic(void)
{
  
  


  l4_msgdope_t result;
  int r;
  l4_threadid_t tid;
  l4_ipc_reg_msg_t msg;
  
  /* map the sonic registers */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = SONIC_BASE;
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
                       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
                       &msg, L4_IPC_NEVER, &result);
}

int32_t
read_time(void)
{

  struct sonic_reg *s;
 
  s = (struct sonic_reg *) SONIC_BASE;
  
 
  return ((s->wt0 | (s->wt1 << 16))/5);
 
}


#else  /* DIZZY */


void 
initsonic(void)
{

 l4_msgdope_t result;
  int r;
  l4_threadid_t tid;
  l4_ipc_reg_msg_t msg;
  struct pt_clock *pt;
  unsigned int time;
  unsigned char *c;



  /* map the reset control memory */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = HPC3_INT_ADDR;
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
                       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
                       &msg, L4_IPC_NEVER, &result);
  
  assert(r == 0);

  pt = (struct pt_clock *) PT_CLOCK_ADDR;
  
  assert(r == 0);
  
  /* start the timer 2 at max value*/
  pt->pt_control = PTCW_SC(2) | PTCW_16B |   PTCW_MODE(MODE_RG) ;
  sync_wait();
  pt->pt_counter2 = 0xff;
  sync_wait();
  pt->pt_counter2 = 0xff;
  sync_wait();
  pt->pt_control = PTCW_SC(1) | PTCW_16B |  PTCW_MODE(MODE_RG) ;
  sync_wait();
  pt->pt_counter1 = 0xff;
  sync_wait();
  pt->pt_counter1 = 0xff;
  sync_wait();
}

#endif

#endif

#endif
