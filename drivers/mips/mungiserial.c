/****************************************************************************
 *
 *      $Id: mungiserial.c,v 1.3 2002/07/22 10:17:35 cgray Exp $
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
 * these are all the functions that we need for the serial chip
 */


#include "z85230.h"
#include "serial.h"

void wbflush(void);

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


zsccdp porta =  (zsccdp)(long)(Z85230_BASE + 2);
zsccdp portb =  (zsccdp)(long)(Z85230_BASE + 0);



void
zsccputreg (volatile zsccdp dp, unsigned char reg, unsigned char val)
{
  if (reg != zWR0) {
      dp->ucmd = zWR0_REG | reg; 
      wbflush ();
  }
  dp->ucmd = val;
  wbflush ();
}

unsigned char
zsccgetreg (volatile zsccdp dp, unsigned char reg)
{
  if (reg != zRR0) {
    dp->ucmd = zWR0_REG | reg;
    wbflush ();
  }
  return (dp->ucmd);
}

int
zsccinit ()
{

    /* single read to get in known state */
    (void) porta->ucmd;
    (void) portb->ucmd;

    /* global initialisation */

 
    zsccputreg(porta, zWR9, zWR9_HARDRESET |  zWR9_MIE );

    return 0;
}


void
zsccflush (volatile zsccdp dp)
{
    /* wait for Tx fifo to drain */

    int timeout = 10;
    while (!(zsccgetreg (dp, zRR0) & zRR0_TXEMPTY))
	if (--timeout == 0)
	    break;
}



int
zsccprogram (volatile zsccdp dp, int baudrate,int p_type)
{
    zsccflush (dp);

    baudrate &= 0xf;
    if (baudrate == 0)
      return 1;
    
    /*
     * See the zscc manual for details.
     */
    zsccputreg(dp, zWR3, 0);
    zsccputreg(dp, zWR5, 0);

    zsccputreg(dp, zWR4, zWR4_1STOPBIT | zWR4_CLK);
    zsccputreg(dp, zWR10, zWR10_NRZ);
    
    zsccputreg(dp, zWR14, zWR14_NOP); /* stop BRG */
    zsccputreg(dp, zWR11, zWR11_TRXCOUT| zWR11_TRXCBRG| zWR11_TCLKBRG | zWR11_RCLKBRG);
    zsccputreg(dp, zWR12,  zsccbrtc [baudrate] & 0xff);
    zsccputreg(dp, zWR13,  (zsccbrtc [baudrate] >> 8) & 0xff);
    
    zsccputreg(dp, zWR14, zWR14_BRGENABLE | zWR14_BRGSRC );
    
    zsccputreg(dp, zWR15, 0);

    if (p_type == 1)
      {
	/* zsccputreg(dp, zWR9, zWR9_MIE ); */
	zsccputreg(dp, zWR1, zWR1_TXIEN | zWR1_RXIENALL);
      }
    else if (p_type == 2)
      {
	zsccputreg(dp, zWR1, zWR1_NONE);
      }

    zsccputreg(dp, zWR3, zWR3_RXENABLE | zWR3_RX8BITCHAR);
    zsccputreg(dp, zWR5, zWR5_TXENABLE| zWR5_RTS | zWR5_TX8BITCHAR | zWR5_DTR);
    
    /* turn off the tx interrupt, till there is something to print */

    if (p_type == 1)
      zsccputreg(dp, zWR0, zWR0_RESETTXINT);

    return 0;

  
}
