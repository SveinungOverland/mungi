/****************************************************************************
 *
 *      $Id: serial.c,v 1.2 2002/05/31 05:10:21 danielp Exp $
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

#include "termio.h"

#define MPSC_BAUD       HOST_BAUD

#ifdef P4000
#include "mpsc.h"
#include <kernel/p4000i.h>
#define MPSC_BASE       0x1f300000      /* NEC uPD72001 */
#define MPSC_REG_SEP    4ul
#endif

#ifdef U4600
#include "z85230.h"
#include <kernel/u4600.h>
#define MPSC_BASE       Z85230_BASE
#define MPSC_REG_SEP    2ul
#endif

#include <r4kc0.h>
#include <stdarg.h>
#include <libc.h>
#include <assert.h>
#include <l4/types.h>
#include <l4/ipc.h>
#include <l4/syscalls.h>
#include <l4/sigma0.h>

#define STACK_SIZE 512
unsigned long _stack[STACK_SIZE];
unsigned long *_sp = &_stack[STACK_SIZE -1];

extern int pmpsc (int op, char *dat, int chan, int data);
int devinit();

typedef void * Addr;
typedef int iFunc();


typedef struct ConfigEntry {
  Addr devinfo;
  int chan;
  iFunc *handler;
  int rxqsize;
  int brate;
} ConfigEntry;

ConfigEntry     ConfigTable[] =
{
  /* p4000 has swapped mpsc ports */
  {(Addr)(MPSC_BASE+MPSC_REG_SEP), 0, pmpsc, 256, MPSC_BAUD},
  {(Addr)(MPSC_BASE+0ul), 1, pmpsc, 256, MPSC_BAUD},
  {0}};

int devinit ()
{
  int             i, brate;
  ConfigEntry    *q;

  for (i = 0; ConfigTable[i].devinfo; i++) {
    q = &ConfigTable[i];
    if (q->chan == 0)
    {
      (*q->handler) (OP_INIT, q->devinfo);
    }
    brate = q->brate;
    (*q->handler) (OP_BAUD, q->devinfo, q->chan, brate);
  }
    return (0);
}


void outch(int c)
{
  ConfigEntry    *q;
  
  q = &ConfigTable[0];

  /* wait to transmit */
  while(!(*q->handler) (OP_TXRDY, q->devinfo, 0));
  pmpsc( OP_TX, q->devinfo, 0, c);
}

int inch(void)
{
    ConfigEntry    *q;
  q = &ConfigTable[0];

  /* wait to transmit */
  while(!(*q->handler) (OP_RXRDY, q->devinfo, 0));
  return pmpsc( OP_RX, q->devinfo, 0,0);
}

#define BUFFSIZE 1024l

char sbuff[BUFFSIZE];
long sin;
l4_threadid_t rcvtid;
extern dword_t rcv_stack[];
extern void rcv_thread(void);

void main()
{
  l4_msgdope_t result;
  int r;
  l4_threadid_t tid, id;
  int *p;
  int i;
  l4_ipc_reg_msg_t msg;
  char *m;

#ifdef P4000  
  /* map the reset control memory */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = 0x1f100000;
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		       &msg, L4_IPC_NEVER, &result);
  assert(r == 0);

  p = (int *) (long) SIO_RESET;
  *p = -1;
#endif
  
  /* map serial port memory */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = MPSC_BASE;
  
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		       &msg, L4_IPC_NEVER, &result);
  assert(r == 0);

  devinit();
  tid = l4_myself();
  rcvtid.ID = 0;
  tid.id.lthread = 1;
  id.ID= -1;
  
  l4_thread_ex_regs(tid,
		    (dword_t) rcv_thread,
		    (dword_t) &rcv_stack[127],
		    &id,
		    &id,
		    &msg.reg[0],
		    &msg.reg[0]);
  
  
  
  
  sprintf(sbuff,"\r\nP4000 SERIAL DRIVER: enabled, thread id 0x%llx\r\n", tid);
  
  sin = 0;
  while (sbuff[sin] != 0)
  {
    outch(sbuff[sin]);      
    sin ++;
  }
  
  
  while (1)
  {
    r = l4_mips_ipc_wait(&tid, L4_IPC_SHORT_MSG, &msg,
			 L4_IPC_NEVER, &result);
    if (r != 0)
    {
      sprintf(sbuff,"\r\nSERIAL DRIVER: received msg error: %x\r\n",
	      r);
      for (sin = 0; sbuff[sin] !=0; sin++)
      {
	outch( sbuff[sin]);
      }
    }
    else
    {
      if (msg.reg[0] == 0)
      {
	rcvtid.ID = msg.reg[1];
      }
      else
      {
	/* sprintf(sbuff, "0x%llx: ", tid.ID);
	for (sin = 0; sbuff[sin] !=0; sin++)
	{
	  outch( sbuff[sin]);
	} */
	m = (char *) &msg.reg[0];
      
	for (i = 0; i < 64; i++)
	{
	  if (m[i] == 0)
	  {
	    break;
	  }
	  else
	  {
	    outch(m[i]);
	    if (m[i] == '\n') outch('\r');
	  }
	}
      }
    }
  }
}

dword_t rcv_stack[128];

void rcv_thread(void)
{
  int r;
  char c;
  l4_ipc_reg_msg_t msg;
  l4_msgdope_t result;
  ConfigEntry    *q;
  q = &ConfigTable[0];

  while(1)
  {
    
    /* wait to transmit */
    while(!(*q->handler) (OP_RXRDY, q->devinfo, 0))
      l4_thread_switch(L4_NIL_ID);
    c =  pmpsc( OP_RX, q->devinfo, 0,0);
    
    if (rcvtid.ID != 0)
    {
      if (c == '\r')
      {
	msg.reg[0] = '\n';
      }
      else
      {
	msg.reg[0] = c;
      }
      r = l4_mips_ipc_send(rcvtid, L4_IPC_SHORT_MSG, &msg,
			   L4_IPC_NEVER, &result);
      if (r != 0)
      {
	rcvtid.ID == 0;
      }
    }
    
  }
}
