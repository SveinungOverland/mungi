/****************************************************************************
 *
 *      $Id: exception.h,v 1.2 2002/05/31 05:49:36 danielp Exp $
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

#ifndef __M_EXCEPTION_H__
#define __M_EXCEPTION_H__

#include <exception.h>
#include "mungi/threads.h"
#include "mungi/upcall.h"

void exception(void);
excpthndlr_t excp_reg(excpthndlr_t [], excpt_t, excpthndlr_t);
void exception_msg(upcall_t *, excpthndlr_t, l4_threadid_t, excpt_t, void *);

#endif /* __M_EXCEPTION_H__ */
