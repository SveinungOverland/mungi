/****************************************************************************
 *
 *      $Id: intr_serial.c,v 1.2 2002/05/31 05:10:21 danielp Exp $
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

#include <string.h>
#include <libc.h>
#include <assert.h>
#include <l4/ipc.h>
#include <l4/syscalls.h>
#include <l4/sigma0.h>
#include <kernel/machine.h>
#include "z85230.h"
#include "termio.h"
 
#define STACK_SIZE 512
unsigned long _stack[STACK_SIZE];
unsigned long *_sp = &_stack[STACK_SIZE -1];

#define BUFFSIZE 1024

int io_buffer_send;
char io_buffer[BUFFSIZE];
typedef struct zsccdev *zsccdp;

zsccdp porta =  (zsccdp)(long)(Z85230_BASE + 2);
zsccdp portb =  (zsccdp)(long)(Z85230_BASE + 0);

#define ZSCCCLK	14745600	 /* clock rate 14.7456 MHz */

static const int zsccbrtc[] = {
    -1,
    BRTC (ZSCCCLK, 50),
    BRTC (ZSCCCLK, 75),
    BRTC (ZSCCCLK, 110),
    BRTC (ZSCCCLK, 134),
    BRTC (ZSCCCLK, 150),
    BRTC (ZSCCCLK, 200),
    BRTC (ZSCCCLK, 300),
    BRTC (ZSCCCLK, 600),
    BRTC (ZSCCCLK, 1200),
    BRTC (ZSCCCLK, 1800),
    BRTC (ZSCCCLK, 2400),
    BRTC (ZSCCCLK, 4800),
    BRTC (ZSCCCLK, 9600),
    BRTC (ZSCCCLK, 19200),
    BRTC (ZSCCCLK, 38400)};


static void
zsccputreg (volatile zsccdp dp, unsigned char reg, unsigned char val)
{
  if (reg != zWR0) {
      dp->ucmd = zWR0_REG | reg; 
      wbflush ();
  }
  dp->ucmd = val;
  wbflush ();
}

static unsigned char
zsccgetreg (volatile zsccdp dp, unsigned char reg)
{
  if (reg != zRR0) {
    dp->ucmd = zWR0_REG | reg;
    wbflush ();
  }
  return (dp->ucmd);
}

static int
zsccinit (volatile zsccdp dp)
{
    volatile zsccdp dpa, dpb;

    dpa = dp; dpb = dpa - 1;

    /* single read to get in known state */
    (void) dpa->ucmd;
    (void) dpb->ucmd;

    /* global initialisation */
    
    zsccputreg(dpa, zWR9, zWR9_HARDRESET);

    return 0;
}


static void
zsccflush (volatile zsccdp dp)
{
    /* wait for Tx fifo to drain */
    int timeout = 10;
    while (!(zsccgetreg (dp, zRR0) & zRR0_TXEMPTY))
	if (--timeout == 0)
	    break;
}


static int
zsccprogram (volatile zsccdp dp, int baudrate)
{
    zsccflush (dp);

    baudrate &= CBAUD;
    if (baudrate == 0)
      return 1;
    
    /*
     * See the zscc manual for details.
     */

    zsccputreg(dp, zWR4, zWR4_1STOPBIT | zWR4_CLK);
    zsccputreg(dp, zWR10, zWR10_NRZ);
    
    zsccputreg(dp, zWR14, zWR14_NOP); /* stop BRG */
    zsccputreg(dp, zWR11, zWR11_TRXCOUT| zWR11_TRXCBRG| zWR11_TCLKBRG | zWR11_RCLKBRG);
    zsccputreg(dp, zWR12,  zsccbrtc [baudrate] & 0xff);
    zsccputreg(dp, zWR13,  (zsccbrtc [baudrate] >> 8) & 0xff);
    
    zsccputreg(dp, zWR14, zWR14_BRGENABLE | zWR14_BRGSRC );
    

    
    zsccputreg(dp, zWR15, zWR15_WR7PEN);
    zsccputreg(dp, zWR7, zWR7P_TXEMPTYIEN);
    zsccputreg(dp, zWR1, zWR1_TXIEN | zWR1_RXIENALL);
    zsccputreg(dp, zWR9, zWR9_MIE );
    
    zsccputreg(dp, zWR3, zWR3_RXENABLE | zWR3_RX8BITCHAR);
    zsccputreg(dp, zWR5, zWR5_TXENABLE| zWR5_RTS | zWR5_TX8BITCHAR | zWR5_DTR);

    return 0;
}


dword_t print_stack[128];
l4_threadid_t intr, rcvtid;
int count;

void print_thread (void)
{
  l4_ipc_reg_msg_t msg;
  l4_msgdope_t result;
  int r;
  unsigned char intcode, c;
  /* associate interrupt */
  intr.ID = 0;
  intr.id.version_low = 2 + 1;
  r = l4_mips_ipc_receive(intr,L4_IPC_SHORT_MSG, &msg,
			  L4_IPC_TIMEOUT(0,0,0,1,0,0), &result);
  while (1)
  {
    /* wait for buffer to clear */
    r = l4_mips_ipc_receive(intr,L4_IPC_SHORT_MSG, &msg,
			    L4_IPC_NEVER, &result);
    intcode = zsccgetreg (portb, zRR2B);
    switch (intcode & 016)
    {
    case 010:
      /* TX intr */
      if (io_buffer_send == count)
      {
	/* disable Tx interrupts */
	zsccputreg(porta, zWR0, zWR0_RESETTXINT);
	count = 0;
	io_buffer_send = 0;
      }
      else
      {
	for (r = 0; r < 4; r++)
	{
	  if (io_buffer[count] == '\n')
	  {
	    porta->udata = io_buffer[count];
	    io_buffer[count] = '\r';
	  }
	  else
	  {
	    porta->udata = io_buffer[count++];
	  }
	  if (io_buffer_send ==  count)
	  {
	    break;
	  }
	}
	wbflush();
      }
      break;
    case 014:
      c =  porta->udata;
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
      
      break;
    default:
      break;
    }
  }
}
  
  
void main(void)
{
  l4_ipc_reg_msg_t msg;
  l4_msgdope_t result;
  l4_threadid_t tid, page, pid;
  dword_t dum;
  int r;
  /* map the serial port memory  */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = Z85230_BASE;
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		       &msg, L4_IPC_NEVER, &result);
  
  assert(r == 0);

  io_buffer_send = 0;
  /* first initialise serial port */
  zsccinit(porta);
  zsccprogram(porta,HOST_BAUD);
  pid = tid = l4_myself();
  pid.id.lthread = 1;
  page.ID = -1;
  /* now start interrupt thread thread */
  l4_thread_ex_regs(pid,
                    (dword_t) print_thread,
                    (dword_t) &print_stack[127],
                    &page,
                    &page,
                    &dum,
                    &dum);

  /* print out my id */
  
  sprintf(io_buffer,
	  "Interrupt Serial I0, thread id 0x%llx\r\n",tid);
  io_buffer_send = strlen(io_buffer);
  io_buffer[64] = 0;
  while(1)
  {
    if (io_buffer_send > 0)
    {
      if (io_buffer[0] == '\n')
      {
	count = 0;
	io_buffer[count] = '\r';
	porta->udata =  '\n';
	wbflush();
      }
      else
      {
	count = 1;
	porta->udata = io_buffer[0];
	wbflush();
      }
    }
    r = l4_mips_ipc_wait(&tid, L4_IPC_SHORT_MSG, &msg,
			 L4_IPC_NEVER, &result);
    while (io_buffer_send != 0) l4_thread_switch(L4_NIL_ID);
    if (r != 0)
    {
      sprintf(io_buffer,
	      "\r\n***** SERIAL DRIVER: received msg error, ignoring....\r\n"
	      );
      io_buffer_send = strlen(io_buffer);
    }
    else
    {
      if (msg.reg[0] == 0)
      {
	rcvtid.ID = msg.reg[1];
      }
      else
      {
	strncpy(io_buffer, (char *)  &msg.reg[0], sizeof(msg));
	io_buffer_send = strlen(io_buffer);
      }
    }
  }
}



