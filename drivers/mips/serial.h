/****************************************************************************
 *
 *      $Id: serial.h,v 1.2 2002/05/31 05:10:18 danielp Exp $
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


#ifndef __SERIAL_H__
#define __SERIAL_H__


#include "z85230.h"

#define HOST_BAUD 15
#define CS1_BASE      0x1c800000
#define Z85230_BASE (CS1_BASE | 0x30)
#define MPSC_BASE       Z85230_BASE
#define STACK_SIZE 512


typedef struct zsccdev *zsccdp;


extern zsccdp porta;
extern zsccdp portb;

#define ZSCCCLK	14745600	 /* clock rate 14.7456 MHz */

void
zsccputreg (volatile zsccdp dp, unsigned char reg, unsigned char val);

unsigned char
zsccgetreg (volatile zsccdp dp, unsigned char reg);

int
zsccinit (void);

void
zsccflush (volatile zsccdp dp);

int
zsccprogram (volatile zsccdp dp, int baudrate, int p_type);

#endif /*  _SERIAL_H_  */
