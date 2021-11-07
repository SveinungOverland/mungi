/****************************************************************************
 *
 *      $Id: serial.h,v 1.3 2002/05/31 05:00:49 danielp Exp $
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
 *        Project:  
 *        Created:  28/04/2000 17:18:18 by Simon Winwood (sjw)
 *  Last Modified:  03/05/2000 19:43:39 by Simon Winwood (sjw)
 *   Version info:  $Revision: 1.3 $ 
 *    Description:
 *
 *       Comments:
 *
 */

#ifndef SERIAL_H
#define SERIAL_H

/* These functions are implemented by the device specific handlers */

int dev_init(void);
void dev_putc(int uart, char c);
int dev_getc(int uart);

void console_out(char *msg);

#endif  /* SERIAL_H */
