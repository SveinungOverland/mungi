/****************************************************************************
 *
 *      $Id: nameserver.c,v 1.4 2002/07/31 07:04:42 cgray Exp $
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
#include <mungi.h>
#include <string.h>
#include "paxlib.h"
#include "mlib.h"

cap_t nameserver_get(cap_t param);
cap_t nameserver_set(cap_t param);


/****************************************************************************
 *
 * Defines
 *
 ****************************************************************************/
#define MAX_ENTRIES 20
#define MAX_NAME_SIZE 256

/****************************************************************************
 *
 * Globals
 *
 ****************************************************************************/
char ns_name[MAX_ENTRIES][MAX_NAME_SIZE] = { "Hello", "There", "Antony" };
unsigned long ns_value[MAX_ENTRIES] = { 0, 0, 0 };
static clist_t *clist = NULL;

/* FIXME: this is until we have a proper way to do it. ceg */
static void
check_apd(void)
{
	apddesc_t myapd;
	
	/* if init isn't set, add the param to the APD */
	if( ApdLookup( clist, M_WRITE ) == NULL )
	{
		ApdGet( &myapd );
		clist = myapd.clist[0].address;
	}
}


/****************************************************************************
 *
 * Implementation
 *
 ****************************************************************************/
cap_t nameserver_get( cap_t param )
{
    res_desc_t rv;
    char *name;
    int i;

    /* check caps etc. we need */
    check_apd();

    /* inser the user cap into pos 0 of our user-cap-clist */
    /* FIXME: change this... later */
    add_to_clist_pos( param, clist, 0 );

    /* get the string name */
    name = (char*)param.address;
    
    /* setup the return value */
    rv.type = 0;
    rv.data1 = 0;

    /* linear search for match */
    for( i = 0; i < MAX_ENTRIES; i++ ) {
	    if( ns_value[i] != 0 ) {
		    if( strcmp(ns_name[i],name) == 0 ) {
			    _a_memcpy( (char*)&(rv.data1), 
				       (char*)&(ns_value[i]), 
				       sizeof(unsigned long) );
			    break;
		    }
	    }
    }

    return *((cap_t*)&rv);
}

cap_t nameserver_set( cap_t param )
{
    res_desc_t rv;
    char *name;
    void *value;
    int sl, i, f = 0;

    /* check caps etc. we need */
    check_apd();

    /* inser the user cap into pos 0 of our user-cap-clist */
    /* FIXME: change this... later */
    add_to_clist_pos( param, clist, 0 );

    /* get vars */
    value = *((entry_pt_t*)param.address);
    name = (char*)param.address;
    name += sizeof(entry_pt_t);

    /* set rv */
    rv.type = 0;
    rv.data1 = 0;
    rv.data0 = 0;
    
    /* save for retrieval */
    for( i = 0; i < MAX_ENTRIES; i++ ) {
	if( ns_value[i] == 0 ) {
	    sl = strlen( name );
	    _a_memcpy( (char*)ns_name[i], (char*)name, sl );
	    ns_name[i][sl] = 0;
	    _a_memcpy( (char*)&(ns_value[i]), (char*)&value, 
		       sizeof(unsigned long) );
	    f = 1;
	    break;
	}
    }

    /* throw exception if bad */
    if( f == 0 )
	rv.type = 1;    
    return *((cap_t*)(&rv));
}

