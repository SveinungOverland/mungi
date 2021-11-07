/****************************************************************************
 *
 *      $Id: sonic.c,v 1.2 2002/05/31 05:10:21 danielp Exp $
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

#include "sonic.h"
#include <kernel/p4000i.h>
#include <libc.h>
#include <assert.h>
#include <l4/ipc.h>
#include <l4/syscalls.h>
#include <l4/sigma0.h>

static const l4_threadid_t SERIAL_TID = {0x1002000000060001};

main()
{
  l4_msgdope_t result;
  int r;
  l4_threadid_t tid;
  l4_ipc_reg_msg_t msg;
  struct sonic_reg *s;
  int *net_reset;
  unsigned int time;
  
  /* map the reset control memory */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = RESET_BASE;
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		       &msg, L4_IPC_NEVER, &result);
  
  assert(r == 0);

  /* map the sonic registers */
  msg.reg[0] = SIGMA0_DEV_MAP;
  msg.reg[1] = SONIC_BASE;
  r = l4_mips_ipc_call(SIGMA0_TID, L4_IPC_SHORT_MSG, &msg,
		       L4_IPC_MAPMSG(0, L4_WHOLE_ADDRESS_SPACE),
		       &msg, L4_IPC_NEVER, &result);

  assert(r == 0);

  /* take sonic out of reset state */
  net_reset = (int *) NET_RESET;
  *net_reset = -1;

  s = (struct sonic_reg *) SONIC_BASE;
  
  sprintf((char *) &msg.reg[0], "SONIC Rev %d enabled\r\n", s->sr);

  r = l4_mips_ipc_send(SERIAL_TID, L4_IPC_SHORT_MSG,  &msg,
		  L4_IPC_NEVER, &result);

  assert(r == 0);

  /* start the timer */
  s->wt0 = 0xffff;
  s->wt1 = 0xffff;
  s->cr = S_CR_ST;

  r = l4_mips_ipc_wait(&tid, L4_IPC_SHORT_MSG,
		       &msg,  L4_IPC_NEVER, &result);
  assert(r == 0);
  
  while (1)
  {
    msg.reg[0] = s->wt0 | (s->wt1 << 16);
    r = l4_mips_ipc_reply_and_wait(tid, L4_IPC_SHORT_MSG, &msg,
				   &tid, L4_IPC_SHORT_MSG, &msg,
				   L4_IPC_NEVER, &result);
    assert(r == 0);
  }
}
