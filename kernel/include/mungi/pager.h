/****************************************************************************
 *
 *      $Id: pager.h,v 1.5 2002/05/31 05:49:37 danielp Exp $
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

#ifndef __M_PAGER_H
#define __M_PAGER_H

void pager(void);
void apager(void);

struct pending_request *handle_fault(struct pending_request *);
void protection_exception(mthread_t *,l4_threadid_t,void *,void *);
void do_map(l4_threadid_t,void *,access_t);

#endif /* __M_PAGER_H */
