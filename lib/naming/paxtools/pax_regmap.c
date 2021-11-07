/****************************************************************************
 *
 *      $Id: pax_regmap.c,v 1.3 2002/07/22 10:18:04 cgray Exp $
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

#include <assert.h>
#include <l4/types.h>
#include <mungi.h>
#include <mlib.h>
#include "pax_regmap.h"
#include "pax_register.h"

int pax_regmap_register_components( cap_t *caps, int size )
{
	int r, i;
	cmpt_reg_desc_t *cmpts;
	apddesc_t apd;
	clist_t *cl;

	/* check params */
	assert( caps != NULL );

	/* get the map of components */
	cmpts = _rm_entries;
	
	/* register each entry - if nameserver then add cap to APD */
	for( i = 0; cmpts[i].obj_addr != NULL; i++ ) {
		if( i >= size ) {
			assert( 0 ); //FIXME
		}
		r = pax_register_component( &(cmpts[i]), &(caps[i]) );
		assert( r == 0 );
		if( cmpts[i].flags & PREG_NO_REGISTER ) {
			ApdGet( &apd );
			cl = (clist_t*)apd.clist[0].address;
			add_to_clist( caps[i], cl );
		}
	}

	/* return the number of things registered */
	return i;
}

void pax_regmap_showmap()
{
#if 0
	int i, j;
	cmpt_reg_interface_t *cmpts;

	cmpts = reg_map();

	for( i = 0; cmpts[i].obj_addr != NULL; i++ ) {
		printf( "COMPONENT ADDRESS = 0x%p\n", cmpts[i].obj_addr );
		for( j = 0; cmpts[i].entries[j].entry_type != CRE_ENDLIST; j++ ) {
			printf( "\ttype = %d\n", cmpts[i].entries[j].entry_type );
			printf( "\tdata0 = 0x%lx\n", cmpts[i].entries[j].data0 );
			printf( "\tdata1 = 0x%lx\n", cmpts[i].entries[j].data1 );
			printf( "\tdata2 = 0x%lx\n", cmpts[i].entries[j].data2 );
			printf( "\tdata3 = 0x%lx\n", cmpts[i].entries[j].data3 );
		}
	}
#endif
}
