/****************************************************************************
 *
 *      $Id: timing.h,v 1.2 2002/05/31 05:10:19 danielp Exp $
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

/****************************************************************************\
.                                                                            .
.                                                                            .
.		Mungi Project                                                .
.               (c) Copyright 1994,95,96 all rights reserved                 .
.               University of New South Wales                                .
.		Partly supported by ARC grant #A49330285                     .
.                                                                            .
.               Project Manager: Steve Russell                               .
.               smr@cse.unsw.edu.au                                          .
.                                                                            .
.               Author: Jerry Vochteloo                                      .
.               jerry@cse.unsw.edu.au                                        .
.                                                                            .
.                              						     .
\****************************************************************************/
/* timing.h */

#ifndef __TIMIMG_H__
#define __TIMING_H__


/* 
 * This contains the header stuff for the profiling stuff.
 */
void inittimer(void);
uint32_t read_time(void);
uint32_t byte_swap(uint32_t);



#ifdef PROTB

#define GT_BASE_ADDR         0x14000000

/* Timer / Counter */

#define GT_TIMER0            0x850
#define GT_TIMER1            0x854
#define GT_TIMER2            0x858
#define GT_TIMER3            0x85c
#define GT_TIMER_CNTRL       0x864

#define GT_TIM0_EN           1 
#define GT_TIM0_CNT          0
#define GT_TIM0_TMR          (1 << 1)
#define GT_TIM1_EN           (1 << 2)
#define GT_TIM1_CNT          0
#define GT_TIM1_TMR          (1 << 3)
#define GT_TIM2_EN           (1 << 4)
#define GT_TIM2_CNT          0
#define GT_TIM2_TMR          (1 << 5)
#define GT_TIM3_EN           (1 << 6)
#define GT_TIM3_CNT          0
#define GT_TIM3_TMR          (1 << 7)

#define GT_INT_CAUSE         0xc18
#define GT_INT_CPU_MASK      0xc1c
#define GT_INT_TIME0EXP      (1 << 8)


#define GTWX(x) (*(volatile uint32_t *)(_gt64010 + (x)))

#endif


#ifdef ALG
#include <mungi/stdmlib.h>

#define SONIC_BASE            0x1f600000
#define RESET_BASE            0x1f100000



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


#endif

#ifdef DIZZY /*   defined DIZZY */



/*
 * Definitions for 8254 programmable interval timer
 *
 * NOTE: counter2 is clocked at MASTER_FREQ (defined below), the
 * output of counter2 is the clock for both counter0 and counter1.
 * For the IP17/IP20/IP22, counter0 output is tied to interrupt 2 for the
 * scheduling clock, and interrupt 3 for the 'fast' clock.
 * For all other CPUs, counter0 output is tied to Interrupt 2 to act 
 * as the scheduling clock and the output of counter1 is tied to 
 * Interrupt 4 to act as the profiling clock.
 */
/*
 * control word definitions
 */

#define PTCW_SC(x)      ((x)<<6)        /* select counter x */
#define PTCW_RBCMD      (3<<6)          /* read-back command */
#define PTCW_CLCMD      (0<<4)          /* counter latch command */
#define PTCW_LSB        (1<<4)          /* r/w least signif. byte only */
#define PTCW_MSB        (2<<4)          /* r/w most signif. byte only */
#define PTCW_16B        (3<<4)          /* r/w 16 bits, lsb then msb */
#define PTCW_MODE(x)    ((x)<<1)        /* set mode to x */
#define PTCW_BCD        0x1             /* operate in BCD mode */

/*
 * Mode definitions
 */
#define MODE_ITC        0               /* interrupt on terminal count */
#define MODE_HROS       1               /* hw retriggerable one-shot */
#define MODE_RG         2               /* rate generator */
#define MODE_SQW        3               /* square wave generator */
#define MODE_STS        4               /* software triggered strobe */
#define MODE_HTS        5               /* hardware triggered strobe */



#define IP22BOFF(X)             ((X)|0x3)

/* the z85230 serial chip */
#define HPC3_SERIAL1_CMD                0x1fbd9830      /* 6,0x0c */
#define HPC3_SERIAL1_DATA               0x1fbd9834      /* 6,0x0d */

/* not sure, but header file has serial macro that reads from here after
   each write to serial chip */

#define HPC3_INTSTAT_ADDR       0x1fbb0000 

/* Addresses for the 8254 timer */

#define HPC3_INT3_ADDR          0x1fbd9880      /* 6,0x20 XXX - IP24 */
#define HPC3_INT2_ADDR          0x1fbd9000
#define HPC3_INT_ADDR           HPC3_INT3_ADDR

#define PT_CLOCK_OFFSET         0x30
#define PT_CLOCK_ADDR           (HPC3_INT_ADDR+PT_CLOCK_OFFSET)
#define MASTER_FREQ     1000000         /* master frequency */

/* clear timer 2 bits (ws) */
#define TIMER_ACK_ADDR          (HPC3_INT_ADDR+TIMER_ACK_OFFSET)
#define TIMER_ACK_OFFSET        IP22BOFF(0x20)
#define ACK_TIMER0      0x1     /* write strobe to clear timer 0 */
#define ACK_TIMER1      0x2     /* write strobe to clear timer 1 */




struct pt_clock {
        char fill0[3];
        unsigned char   pt_counter0;            /* counter 0 port */
        char fill1[3];
        unsigned char   pt_counter1;            /* counter 1 port */
        char fill2[3];
        unsigned char   pt_counter2;            /* counter 2 port */
        char fill3[3];
        unsigned char   pt_control;             /* control word */
};


/* clear timer 2 bits (ws) */
#define TIMER_ACK_ADDR          (HPC3_INT_ADDR+TIMER_ACK_OFFSET)
#define TIMER_ACK_OFFSET        IP22BOFF(0x20)
#define ACK_TIMER0      0x1     /* write strobe to clear timer 0 */
#define ACK_TIMER1      0x2     /* write strobe to clear timer 1 */

#endif   /* defined DIZZY */





#ifdef INSTRUMENT


extern uint32_t * t_tarray;

#define initstamps() initsonic()
#define tstamp(a) t_tarray[a]  = (uint32_t) read_time()
#ifdef PROTB
#define prstamps(num) for(i=0;i<num;i+=1) if (t_tarray[i]) printf("t%d=\t%u\r\n",i,(uint32_t) byte_swap(t_tarray[i]));
#else
#define prstamps(num) for(i=0;i<num;i+=1) if (t_tarray[i]) printf("t%d=\t%u\r\n",i,(uint32_t) t_tarray[i]);
#endif
#define clstamps(num) for(i=0;i<num;i+=1) t_tarray[i] = 0;


#else    /* instrument */

#define initstamps() ((void)0)
#define tstamp(a) ((void)0) 
#define prstamps(a) ((void)0)
#define start_time() ((void)0)
#define stop_time() ((void)0)
#define clstamps(a) ((void)0)

#endif    /* instrument */

#endif    /*  __TIMING_H__   */
