/****************************************************************************
 *
 *      $Id: sonic.h,v 1.2 2002/05/31 05:10:21 danielp Exp $
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

#ifndef SONIC_H
#define SONIC_H

#define SONIC_BASE 0x1f600000

#define S_R(r) unsigned :16; unsigned r:16
 
struct sonic_reg
{
  S_R(cr);	        /* 00: Command */
  S_R(dcr);		/* 01: Data Configuration */
  S_R(rcr);		/* 02: Receive Control */
  S_R(tcr);		/* 03: Transmit Control */
  S_R(imr);		/* 04: Interrupt Mask */
  S_R(isr);		/* 05: Interrupt Status */
  S_R(utda);		/* 06: Upper Transmit Descriptor Address */
  S_R(ctda);		/* 07: Current Transmit Descriptor Address */
  S_R(_tps);		/* 08* Transmit Packet Size */
  S_R(_tfc);		/* 09* Transmit Fragment Count */
  S_R(_tsa0);		/* 0a* Transmit Start Address 0 */
  S_R(_tsa1);		/* 0b* Transmit Start Address 1 */
  S_R(_tfs);		/* 0c* Transmit Fragment Size */
  S_R(urda);		/* 0d: Upper Receive Descriptor Address */
  S_R(crda);		/* 0e: Current Receive Descriptor Address */
  S_R(_crba0);	        /* 0f* Current Receive Buffer Address 0 */
  S_R(_crba1);		/* 10* Current Receive Buffer Address 1 */
  S_R(_rbwc0);		/* 11* Remaining Buffer Word Count 0 */
  S_R(_rbwc1);		/* 12* Remaining Buffer Word Count 1 */
  S_R(eobc);		/* 13: End Of Buffer Word Count */
  S_R(urra);		/* 14: Upper Receive Resource Address */
  S_R(rsa);		/* 15: Resource Start Address */
  S_R(rea);		/* 16: Resource End Address */
  S_R(rrp);		/* 17: Resource Read Pointer */
  S_R(rwp);		/* 18: Resource Write Pointer */
  S_R(_trba0);		/* 19* Temporary Receive Buffer Address 0 */
  S_R(_trba1);		/* 1a* Temporary Receive Buffer Address 1 */
  S_R(_tbwc0);		/* 1b* Temporary Buffer Word Count 0 */
  S_R(_tbwc1);		/* 1c* Temporary Buffer Word Count 1 */
  S_R(_addr0);		/* 1d* Address Generator 0 */
  S_R(_addr1);		/* 1e* Address Generator 1 */
  S_R(_llfa);		/* 1f* Last Link Field Address */
  S_R(_ttda);		/* 20* Temp Transmit Descriptor Address */
  S_R(cep);		/* 21: CAM Entry Pointer */
  S_R(cap2);		/* 22: CAM Address Port 2 */
  S_R(cap1);		/* 23: CAM Address Port 1 */
  S_R(cap0);		/* 24: CAM Address Port 0 */
  S_R(ce);		/* 25: CAM Enable */
  S_R(cdp);		/* 26: CAM Descriptor Pointer */
  S_R(cdc);		/* 27: CAM Descriptor Count */
  S_R(sr);		/* 28: Silicon Revision */
  S_R(wt0);		/* 29: Watchdog Timer 0 */
  S_R(wt1);		/* 2a: Watchdog Timer 1 */
  S_R(rsc);		/* 2b: Receive Sequence Counter */
  S_R(crct);		/* 2c: CRC Error Tally */
  S_R(faet);		/* 2d: FAE Tally */
  S_R(mpt);		/* 2e: Missed Packet Tally */
  S_R(_mdt);		/* 2f* Maximum Deferral Timer */
  S_R(_r0);		/* 30* Reserved */
  S_R(_r1);		/* 31* Reserved */
  S_R(_r2);		/* 32* Reserved */
  S_R(_r3);		/* 33* Reserved */
  S_R(_r4);		/* 34* Reserved */
  S_R(_r5);		/* 35* Reserved */
  S_R(_r6);		/* 36* Reserved */
  S_R(_r7);		/* 37* Reserved */
  S_R(_r8);		/* 38* Reserved */
  S_R(_r9);		/* 39* Reserved */
  S_R(_r10);		/* 3a* Reserved */
  S_R(_r11);		/* 3b* Reserved */
  S_R(_r12);  		/* 3c* Reserved */
  S_R(_r13);		/* 3d* Reserved */
  S_R(_r14);		/* 3e* Reserved */
  S_R(dcr2);		/* 3f* Data Configuration 2 */
};


#define S_CR_ST         0x0020
#define S_CR_STP        0x0010

#define S_IMR_TCEN      0x0080
#define S_ISR_TC        0x0080

#endif
