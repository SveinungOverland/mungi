/****************************************************************************
 *
 *      $Id: io.h,v 1.2 2002/05/31 05:00:48 danielp Exp $
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
 *        Created:  06/12/2000 11:03:58 by Simon Winwood (sjw)
 *  Last Modified:  06/12/2000 15:23:58 by Simon Winwood (sjw)
 *   Version info:  $Revision: 1.2 $ 
 *    Description:
 *
 *       Comments:
 *
 * $Log: io.h,v $
 * Revision 1.2  2002/05/31 05:00:48  danielp
 * License header. GPL.
 *
 * Revision 1.1  2002/02/05 03:47:34  brettn
 * Mammoth update:
 * Added pager mappings for MIPS clock into pager
 * Removed dodgy PRINT macro in kernel/src/clock.c
 * Split user and kernel clocks.
 *                                                 (from andrewb)
 * Added multiple user task support (Makefile and subdir.mk)
 * Moved init.c
 * Console driver update/rewrite for alpha.
 * Fix type of pdx_t function
 * Removed \n => \n\r
 * Fixed upcall UserPrint for mips and alpha
 * Add handling for 'mishell' and 'init' instead of mungiapp
 * Added more capabilities for init.
 * Death of aprintf, moved to kprintf.
 * Add character input to startup.
 * Fstat removed from mungi and added to POSIX.
 * Added new apps and mishell.
 * Added alpha serial driver, posix compatibility library and mlib.
 *                                                 (from alexs and cgray)
 * Fixed allocation bug in rpager (will not alloc kernel memory now)
 * Misc rpager cleanups - more needed.
 *                                                 (benno, cgray & me)
 *
 * Revision 1.2  2001/04/05 08:25:41  sjw_
 * Fixed makefile to guess l4root as L4Alpha-2.0.0 (hack?)
 *
 * Revision 1.1  2001/03/27 12:10:35  sjw
 * Added some existing code
 *
 */

#include "io_miata.h"

