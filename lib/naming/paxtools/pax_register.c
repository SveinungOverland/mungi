/****************************************************************************
 *
 *      $Id: pax_register.c,v 1.6 2002/07/31 07:04:42 cgray Exp $
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

#include "pax_register.h"
#include "../lib/paxlib.h"
#include <syscalls/userprint.h>

#define printf(x...) UserPrint(x)

int pax_register_component( cmpt_reg_desc_t *crd, cap_t *pdc )
{
	int i, ep_count = 0, r;
	pdx_t entry_pts[MAX_PDX_ENTRY_PTS];
	clist_t *cl;
	entry_type_t etype;
	cap_t pdx_clist, *lkp, empty;

	/* check params */
	assert( crd != NULL );
	assert( pdc != NULL );

	printf( "REGISTER COMPONENT = %s\n", crd->name );

	/* make the pdx cap */
	pdc->address = crd->obj_addr;
	pdc->passwd = create_new_passwd();

	/* make the clist that will be registered with the component */
	r = create_clist( PAGESIZE, &pdx_clist );
	assert( r == 0 );
	cl = (clist_t*)pdx_clist.address;
       	if( ((crd->flags)&PREG_NO_REGISTER) == 0 ) {
	    cl->format = CL_PROTCTX;
	}

	/* Add an empty space so he can put in the inst obj */
	empty.address = (void*)0;
	empty.passwd = 0;
	add_to_clist( empty, cl );

	/* add another spot for client caps */
	/* FIXME: this is temporary until we have a proper PDX/cap library
	 * and MCS is a little smarter */
	add_to_clist( empty, cl );

	/* Add the clist cap so he cann access it */
	add_to_clist( pdx_clist, cl );

	/* add the object cap to the clist, so he can execute his code */
	printf( "STEP 1\n" );
	printf( "crd->obj_addr = 0x%llx\n", crd->obj_addr );
	lkp = ApdLookup( crd->obj_addr, M_READ | M_WRITE | M_EXECUTE );
	assert( lkp != NULL );
	printf( "STEP 1 - OK\n" );
	add_to_clist( *lkp, cl );

	/* DEBUG */
	printf( "CODE CAP -> 0x%p, 0x%p\n", lkp->address, (void*)lkp->passwd );
	
	/* copy the entry points into the buffer */
	for( i = 0; i < MAX_TOTAL_ENTRIES && (crd->entries)[i].entry_type != CRE_ENDLIST; i++ ) {
	    etype = (crd->entries)[i].entry_type;
	    switch( etype ) {
	    case CRE_ENTRY_PT:
		
		/* DEBUG */
		printf( "EPT %d (%ld,%ld,%ld) - 0x%p\n", ep_count, 
			(crd->entries)[i].data.data0,
			(crd->entries)[i].data.data1,
			(crd->entries)[i].data.data2,
			(void*)(crd->entries)[i].data.data3 );
		entry_pts[ep_count] = (void*)((crd->entries)[i].data.data3);
		ep_count += 1;
		break;
	    case CRE_CONTEXT:
		printf( "CRE_CONTEXT: 0x%p, 0x%lx\n", 
			(void*)((crd->entries)[i].data.data0), 
			(crd->entries)[i].data.data1 );
		lkp = ApdLookup( (void*)((crd->entries)[i].data.data0), 
				 (crd->entries)[i].data.data1 );
		assert( lkp != NULL );
		add_to_clist( *lkp, cl );
		//			printf( "CAP_FOUND: 0x%p, 0x%p\n", lkp->address, (void*)lkp->passwd );
		break;
	    default:
		assert( !"Unknown entry type" );
	    }
	}
	
	/* DEBUG - PRINT THE CLIST */
	//	printf( "\t#caps = %d\n", cl->n_caps );
	//	for( i = 0; i < cl->n_caps; i++ )
	//		printf( "\t\t0x%p, 0x%p\n", cl->caps[i].address, (void*)(cl->caps[i].passwd) );
	//printf( "# entry points = %d\n", ep_count );
	//for( i = 0; i < ep_count; i++ ) {
	//	printf( "\t0x%p\n", entry_pts[i] );
	//}
	
	/* make pdx object */
	r = ObjCrePdx( *pdc, cl, 0, 0, 0, ep_count, entry_pts );
	assert( r == 0 );
	
	/* if no_register flag set then return */
	if( (crd->flags) & PREG_NO_REGISTER ) 
	    return 0;
	
	/* otherwise register each name with the cl - so start the paxlib, see if already started */
	//	FIXME: for some reason this wasn't working - always thought it had to restart
	//	paxlib_status = paxlib_init();
	//	assert( paxlib_status == 0 || paxlib_status == E_PAXLIB_ALREADY_STARTED );
	
	/* register names */
	for( i = 0; i < MAX_TOTAL_ENTRIES; i++ ) {   
	    etype = (crd->entries[i]).entry_type;
	    if( etype == CRE_ENDLIST )
		break;
	    if( etype == CRE_ENTRY_PT ) {
		r = pax_cl_set_entry_pt(	
					(crd->entries)[i].data.data0, 
					(crd->entries)[i].data.data1,
					(crd->entries)[i].data.data2,
					(entry_pt_t)(crd->entries)[i].data.data3 );
		assert( r == 0 );
	    }
	}
	
	/* ok */
	return 0;
}




