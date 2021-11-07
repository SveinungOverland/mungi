/****************************************************************************
 *
 *      $Id: paxlib.h,v 1.2 2002/05/31 07:56:39 danielp Exp $
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

#ifndef __PAXLIB_H__
#define __PAXLIB_H__

/****************************************************************************
 *
 * Includes
 *
 ****************************************************************************/
#include <mungi.h>

/****************************************************************************
 *
 * Definitions
 *
 ****************************************************************************/
#define MAX_DATA 8
#define PAX_ECICAP_COUNT 3
/* FIXME: this should be in sys/types.h */
#define CL_PROTCTX             ((clistformat_t)0x3)

/****************************************************************************
 *
 * Exceptions.
 *
 ****************************************************************************/
#define MCS_NO_EXCEPTION         0
#define MCS_USER_EXCEPTION       1
#define MCS_STUB_EXCEPTION       2
#define MCS_SYSTEM_EXCEPTION     3

#define MCS_STE_PROT             201
#define MCS_STE_NOINSTOBJ        202
#define MCS_STE_NOTIMPLEMENTED   203
#define MCS_STE_CREIOFAILED      204
#define MCS_STE_INSTLIMIT        205

/****************************************************************************
 *
 * Error codes returned from MCS-RT functions
 *
 ****************************************************************************/
#define MCSE_SUCCESS     0
#define MCSE_FAIL        1
#define MCSE_EXCEPTION   2

/****************************************************************************
 *
 * Header constants
 *
 ****************************************************************************/
#define MCS_IF_VALID     1

/****************************************************************************
 *
 * Type definitions
 *
 ****************************************************************************/
typedef void* entry_pt_t;
typedef unsigned long cid_t;
typedef unsigned long iid_t;
typedef unsigned short mid_t;

typedef struct {
    unsigned long data1;
    unsigned long data0 : 62;
    unsigned type : 2;
} res_desc_t;

typedef struct {
    char _major;
    unsigned short _minor;
    char _data[MAX_DATA];
} environment_t;

typedef struct {
    unsigned long password;
    void *ref;
} cicap_t;

typedef struct {
    short hv;
    unsigned long opasswd, epasswd[PAX_ECICAP_COUNT];
    unsigned int vo, vl;
    unsigned short ef[PAX_ECICAP_COUNT], flags;
    unsigned long dpasswd[1];
} cihdr_t;

/****************************************************************************
 *
 * Prototypes
 *
 ****************************************************************************/
int pax_ci_request( entry_pt_t ept, cap_t param, res_desc_t *ret, void *pd );
int pax_cl_get_entry_pt( cid_t cid, iid_t iid, mid_t mid, entry_pt_t *epr );
int pax_cl_set_entry_pt( cid_t cid, iid_t iid, mid_t mid, entry_pt_t epr );
unsigned long pax_cl_get_component_class( cicap_t cicap );

#endif

