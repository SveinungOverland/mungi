/****************************************************************************
 *
 *      $Id: tree.h,v 1.2 2002/05/31 07:56:44 danielp Exp $
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
 * tree.h
 *
 * Global type, constant and exception definitions from tree.idl.
 * Autogenerated by PaxGen-0.1 on Tue Oct 23 11:42:46 2001
 *
 **************************************************************************************************/
#ifndef __TREE_H__
#define __TREE_H__


/* Includes */
#include "sph.h"
#include "stdpax.h"


/* type definitions */
enum ___aaaaa {
	BTC_OK	= 0,
	BTC_ERROR	= 1
};
typedef enum ___aaaaa btcerror_t;

/*
struct ___aaaab {
	long address;
	long passwd;
};
typedef struct ___aaaab cap_t;
*/


/* component identifiers */
#define CID_CNAMETREE 0x00370100

#endif /* __TREE_H__ */