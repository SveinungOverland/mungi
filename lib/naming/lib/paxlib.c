/****************************************************************************
 *
 *      $Id: paxlib.c,v 1.4 2002/07/22 10:18:03 cgray Exp $
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

/****************************************************************************
 *
 * Includes
 *
 ****************************************************************************/
#include "paxlib.h"
#include <mlib.h>
#include <assert.h>
#include <syscalls/userprint.h>
#include <stdio.h>

/****************************************************************************
 *
 * Globals
 *
 ****************************************************************************/
static cap_t namebuff;
extern pdx_t gepr;
extern pdx_t sepr;

/****************************************************************************
 *
 * Implementation
 *
 ****************************************************************************/
int pax_ci_request( entry_pt_t ept, cap_t param, res_desc_t *ret, void *pd )
{
    int r;

    r = PdxCall( ept, param, (cap_t*)ret, pd );
    if( r != 0 ) {
	    UserPrint( "r!=0!\n" );
	    ret->type = MCS_SYSTEM_EXCEPTION;
	    ret->data0 = ((unsigned long)r << 48);
	    r = MCSE_EXCEPTION;
    } else if( ret->type != MCS_NO_EXCEPTION ) {
	    UserPrint( "Exception!\n" );
	    r = MCSE_EXCEPTION;
    }
    return r;
}

int pax_cl_get_entry_pt( cid_t cid, iid_t iid, mid_t mid, entry_pt_t *epr )
{
    apddesc_t myapd;
    clist_t *clist;
    int r;
    res_desc_t res;

    /* if the namebuff is empty then create object */
    if( namebuff.address == 0 ) {
	namebuff.passwd = 0x2;
	namebuff.address = ObjCreate( PAGESIZE, namebuff.passwd, NULL );
	assert( namebuff.address != NULL );
	ApdGet( &myapd );
	clist = myapd.clist[0].address;
	clist->caps[clist->n_caps++] = namebuff;
    }

    /* form the name as a string */
    sprintf( (char*)namebuff.address, "%016lx_%016lx_%04hx", cid, iid, mid );

    /* now call the nameserver */
    r = pax_ci_request( gepr, namebuff, &res, PD_EMPTY );
    assert( r == 0 );
    assert( res.type == 0 );
    assert( res.data1 != 0 );

    /* put the result into epr and return */
    *epr = (entry_pt_t)res.data1;
    return 0;
}

int pax_cl_set_entry_pt( cid_t cid, iid_t iid, mid_t mid, entry_pt_t epr )
{
    apddesc_t myapd;
    clist_t *clist;
    char *buff;
    int r;
    res_desc_t res;

    /* if the namebuff is empty then create object */
    if( namebuff.address == 0 ) {
	namebuff.passwd = 0x2;
	namebuff.address = ObjCreate( PAGESIZE, namebuff.passwd, NULL );
	assert( namebuff.address != NULL );
	ApdGet( &myapd );
	clist = myapd.clist[0].address;
	clist->caps[clist->n_caps++] = namebuff;
    }

    /* pack params */
    buff = namebuff.address;
    *((entry_pt_t*)(void *)buff) = epr;
    buff += sizeof(entry_pt_t);
    sprintf( (char*)buff, "%016lx_%016lx_%04hx", cid, iid, mid );

    /* now call the nameserver */
    r = pax_ci_request( sepr, namebuff, &res, PD_EMPTY );
    assert( r == 0 );
    assert( res.type == 0 );
    return 0;
}

unsigned long pax_cl_get_component_class( cicap_t cicap )
{
    int r;
    objinfo_t objinfo;

    /* Component-classes must place their CID in the userinfo
     * field of the objinfo struct of the instance object. */
    UserPrint( "cicap.ref is 0x%llx\n", cicap.ref );
    r = ObjInfo( cicap.ref, 0, &objinfo );
    if( r != 0 ) {
	UserPrint( "Could not get ObjInfo correctly!\n" );
	return 0;
    }
    return (unsigned long)objinfo.userinfo;
}

