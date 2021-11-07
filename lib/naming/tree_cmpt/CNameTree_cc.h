/****************************************************************************
 *
 *      $Id: CNameTree_cc.h,v 1.2 2002/05/31 07:56:43 danielp Exp $
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

/***************************************************************************************************
 *
 * CNameTree_cc.h
 *
 * Definitions for the CNameTree component-class.
 * Autogenerated by PaxGen-0.1 on Tue Oct 23 11:42:46 2001
 *
 **************************************************************************************************/
#ifndef __CNAMETREE_CC_H__
#define __CNAMETREE_CC_H__


/* Includes */
#include "tree.h"


/* component data */
typedef struct {
	/* FILL-IN: Insert component-core data members here. */
	void *btree;      /* ptr to btree */
	cap_t malloc_cap; /* cap for malloc obj */
} cnt_t;



/* component functions */
	/* FILL-IN: Insert component-core function members here. */


/* INameTree interface implementation functions */
btcerror_t cnt_int_create( cnt_t *cthis, cap_t obj, long name_offset, long flags, long value );
btcerror_t cnt_int_delete( cnt_t *cthis, cap_t obj, long name_offset );
btcerror_t cnt_int_lookup( cnt_t *cthis, cap_t obj, long name_offset, int ret_flags, int ret_offset );
btcerror_t cnt_int_modify( cnt_t *cthis, cap_t obj, long name_offset, long new_name_offset, long flags, long value );
btcerror_t cnt_int_rlookup( cnt_t *cthis, cap_t obj, long name_offset, long start, long overflow, long recv_off, long recv_len, long num, long value );
btcerror_t cnt_int_fenum( cnt_t *cthis, cap_t obj, long name_offset, long start, long overflow, long recv_off, long recv_len, long num, long value );


/* IClassInterface interface stubs */
res_desc_t cnt_ci_stub_constructor( cap_t param );
res_desc_t cnt_ci_stub_destructor( cap_t param );
res_desc_t cnt_ci_stub_create_cicap( cap_t param );
res_desc_t cnt_ci_stub_get_typeinfo( cap_t param );


/* INameTree interface stubs */
res_desc_t cnt_int_stub_create( cap_t param );
res_desc_t cnt_int_stub_delete( cap_t param );
res_desc_t cnt_int_stub_lookup( cap_t param );
res_desc_t cnt_int_stub_modify( cap_t param );
res_desc_t cnt_int_stub_rlookup( cap_t param );
res_desc_t cnt_int_stub_fenum( cap_t param );

#endif /* __CNAMETREE_CC_H__ */
