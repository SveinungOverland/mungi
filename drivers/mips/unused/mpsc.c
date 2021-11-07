/****************************************************************************
 *
 *      $Id: mpsc.c,v 1.2 2002/05/31 05:10:21 danielp Exp $
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

/* #include <mips.h> 
#include <pmon.h> */
#include "termio.h"
#include "mpsc.h"

extern void wbflush(void);

typedef struct mpscdev *mpscdp;

#define MPSCCLK	6144000	 /* clock rate 6.144MHz */

static const int mpscbrtc[] = {
    -1,
    BRTC (MPSCCLK, 50),
    BRTC (MPSCCLK, 75),
    BRTC (MPSCCLK, 110),
    BRTC (MPSCCLK, 134),
    BRTC (MPSCCLK, 150),
    BRTC (MPSCCLK, 200),
    BRTC (MPSCCLK, 300),
    BRTC (MPSCCLK, 600),
    BRTC (MPSCCLK, 1200),
    BRTC (MPSCCLK, 1800),
    BRTC (MPSCCLK, 2400),
    BRTC (MPSCCLK, 4800),
    BRTC (MPSCCLK, 9600),
    BRTC (MPSCCLK, 19200),
    BRTC (MPSCCLK, 38400)};


/*
 * internal routines to access mpsc registers
 * (we dont need to worry about interrupts do we?)
 */
static void
mpscputreg (volatile mpscdp dp, unsigned int reg, unsigned int val)
{
  if (reg != mCR0) {
      dp->ucmd = mCR0_REG | (reg << 24); 
      wbflush ();
  }
  dp->ucmd = val;
  wbflush ();
}

static unsigned int
mpscgetreg (volatile mpscdp dp, unsigned int reg)
{
  if (reg != mSR0) {
    dp->ucmd = mCR0_REG | (reg << 24);
    wbflush ();
  }
  return (dp->ucmd);
}

static void
mpscputbaud (volatile mpscdp dp, int regval, int baud)
{
  mpscputreg (dp, mCR12, regval);
  dp->ucmd = baud << 24; wbflush();
  dp->ucmd = baud << 16; wbflush ();
  (void)dp->ucmd;
}


static int
mpscinit (volatile mpscdp dp)
{
    volatile mpscdp dpa, dpb;


    /* dp points to channel 1, because P4000 connectors are swapped! */
    dpb = dp; dpa = (mpscdp) ((char *) dpb - 4);


    /* single read to get in known state */
    (void) dpa->ucmd;
    (void) dpb->ucmd;

    /* chip remains in standby mode until first write to register 0 */
    dpa->ucmd = mCR0_NOP; wbflush();

    /* global initialisation */
    dpa->ucmd = mCR0_RESETCHN; wbflush();
    dpb->ucmd = mCR0_RESETCHN; wbflush();

    /* select interrupt operation and make receivers have priority */
    mpscputreg (dpa, mCR2A, mCR2A_PRIRX|mCR2A_SVEC|mCR2A_B1|mCR2A_INTAINTB);
    
    /* interrupt vector */
    mpscputreg (dpb, mCR2B, 0);

    return 0;
}


static int
mpscprogram (volatile mpscdp dp, int baudrate)
{
    unsigned int cr3, cr4, cr5, brtc;
    int timeout;

    /* wait for Tx fifo to drain */
    timeout = 10000;
    while (!(mpscgetreg (dp, mSR1) & mSR1_ALLSENT))
	if (--timeout == 0)
	    break;

    baudrate &= CBAUD;
    if (baudrate == 0)
      return 1;

    /*
     * See the mpsc manual for details.
     */
    cr4 = mCR4_CLKX | mCR4_STOP1;
    cr3 = mCR3_AUTO | mCR3_RXEN | mCR3_8BIT;
    cr5 = mCR5_DTR | mCR5_RTS | mCR5_TXEN | mCR5_8BIT;
    brtc = mpscbrtc [baudrate];

    mpscputreg (dp, mCR1, 0);		/* disable interrupts */
    mpscputreg (dp, mCR4, cr4);
    mpscputreg (dp, mCR5, cr5);
    mpscputreg (dp, mCR3, cr3);
    mpscputreg (dp, mCR14, mCR14_NOP); /* stop BRG */
    mpscputbaud (dp, mCR12_TXBRGSET, brtc);
    mpscputbaud (dp, mCR12_RXBRGSET, brtc);
    mpscputreg (dp, mCR15, mCR15_RXRXBRG|mCR15_TXTXBRG|mCR15_TRXCO|mCR15_TRXCTXCLK);
    mpscputreg (dp, mCR10, mCR10_NRZ);
    mpscputreg (dp, mCR14, mCR14_NOP|mCR14_BRGSYS|mCR14_BRGRX|mCR14_BRGTX);
    mpscputreg (dp, mCR12, mCR12_TXTRXC);
    mpscputreg (dp, mCR11, 0);
    return 0;
}


int pmpsc (int op, char *dat, int chan, int data)
{
    volatile mpscdp dp = (mpscdp) dat;

    switch (op) {
    case OP_INIT:
	return mpscinit (dp);
    case OP_BAUD:
	return mpscprogram (dp, data);
    case OP_TXRDY:
	return (dp->ucmd & mSR0_TXEMPTY);
    case OP_TX:
	dp->udata = data << 24; wbflush ();
	break;
    case OP_RXRDY:
	return (dp->ucmd & mSR0_RXRDY);
    case OP_RX:
	return (dp->udata >> 24) & 0xff;
    }
    return 0;
}
