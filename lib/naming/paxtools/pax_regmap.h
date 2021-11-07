/****************************************************************************
 *
 *      $Id: pax_regmap.h,v 1.3 2002/05/31 07:56:41 danielp Exp $
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

#ifndef __PAX_REGMAP_H_
#define __PAX_REGMAP_H_

#include "pax_register.h"

/* These macros allow the user program to have a simple map at the top of their program
 * declaring all the components and entry points to be generated. */
#define REGISTRATION_MAP()			cmpt_reg_desc_t _rm_entries[] = {

#define NAMESERVER(n)			 { (void*)n, PREG_NO_REGISTER, (char*)"Nameserver", {
#define CONTEXT(n,a)			 { CRE_CONTEXT,  { (long)n, (long)a, (long)0, (long)0 } },

#define COMPONENT(n,t)			 { (void*)n, 0, (char*)t, {
#define ENTRY_PT(c,i,m,e)		 { CRE_ENTRY_PT, { (long)c, (long)i, (long)m, (long)e } },
#define END_ITEM()			 { CRE_ENDLIST,  { (long)0, (long)0, (long)0, (long)0 } } }},	

#define END_REGISTRATION_MAP()		 { NULL, 0, (char*)"SENTINEL ENTRY", {} } };

/* forward declaration for map function */
extern cmpt_reg_desc_t _rm_entries[];
									
/* functions for user to call */
int pax_regmap_register_components( cap_t *caps, int size );
void pax_regmap_showmap(void);


#endif /* __PAX_REGMAP_H_ */
