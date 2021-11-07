/****************************************************************************
 *
 *      $Id: cmpt_setup.c,v 1.5 2002/06/05 05:36:31 cgray Exp $
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

/* this will setup the component etc. so it can be called */


/*************************************************************************
 *
 * Includes
 *
 *************************************************************************/

#include <mungi.h>
#include "mungi/l4_generic.h"
#include <assert.h>
#include <mlib.h>
#include <stdio.h>
#include <syscalls/userprint.h>
#include <string.h>

#include "lib/paxlib.h"
#include "lib/nameserver.h"
#include "paxtools/pax_regmap.h"
#include "paxtools/pax_register.h"
#include "tree_cmpt/tree.h"
#include "tree_cmpt/CNameTree_cc.h"
#include "tree_cmpt/INameTree_io.h"
#include "namehash.h"
#include "namemap.h"

// FIXME: Only use this if we are started from init *before* printing
// is setup
// #define printf(x...) UserPrint(x)

/*************************************************************************
 *
 * Prototypes
 *
 *************************************************************************/
void client_main( int );
cap_t test_proc( cap_t p );

int naming_register_cmpt( cap_t* pdx_objects, int pdx_obj_count );
void naming_set_cmpt( cap_t* caplist, int num_caps );
size_t _naming_serialise(void* dest);
int _naming_deserialise(void * env_data,size_t size);
extern int p9_init( namehash_t *nh, cap_t pbuf );
extern int  p9_mount( char* mount_pt, void *tree, int access_mode );


/*************************************************************************
 *
 * Definitions
 *
 *************************************************************************/
#define PDX_OBJECTS     64
#define MAX_CALLS       1000
#define STARTTIME       0xFFFFFF

/* FIXME: find out from antony what this is */
/* #define PDXBASE         0xa60000 */
#define PDXBASE         naming_register_cmpt

#define PDXDELTA        0x0
#define IO 88


/*************************************************************************
 *
 * Globals
 *
 *************************************************************************/
pdx_t gepr;
pdx_t sepr;


/*************************************************************************
 *
 * Currently registered name provider
 * FIXME: Make this support multiples
 *
 *************************************************************************/
static cap_t    namelist_cap;
static clist_t *namelist_list = NULL;

/***************************************************************************
 *
 * Component Registration Map 
 *
 **************************************************************************/

REGISTRATION_MAP()
     
     NAMESERVER( PDXBASE ) 
       ENTRY_PT( 0, 0, 0, nameserver_get )
       ENTRY_PT( 0, 0, 1, nameserver_set )
     END_ITEM()
     
     COMPONENT( PDXBASE, "TREE_CMPT" )
       ENTRY_PT( CID_CNAMETREE, IID_INAMETREE, 
		 MID_INT_CREATE, cnt_int_stub_create )
       ENTRY_PT( CID_CNAMETREE, IID_INAMETREE, 
		 MID_INT_DELETE, cnt_int_stub_delete )
       ENTRY_PT( CID_CNAMETREE, IID_INAMETREE,
		 MID_INT_LOOKUP, cnt_int_stub_lookup )

       ENTRY_PT( CID_CNAMETREE, IID_INAMETREE, 
		 MID_INT_MODIFY, cnt_int_stub_modify )
       ENTRY_PT( CID_CNAMETREE, IID_INAMETREE, 
		 MID_INT_RLOOKUP, cnt_int_stub_rlookup )
       ENTRY_PT( CID_CNAMETREE, IID_INAMETREE, 
		 MID_INT_FENUM, cnt_int_stub_fenum )


       ENTRY_PT( CID_CNAMETREE, IID_CLASS_INTERFACE, 
		 MID_CI_CONSTRUCTOR, cnt_ci_stub_constructor  )
       ENTRY_PT( CID_CNAMETREE, IID_CLASS_INTERFACE, 
		 MID_CI_DESTRUCTOR, cnt_ci_stub_destructor )
     END_ITEM()

END_REGISTRATION_MAP()


/***************************************************************************
 *
 * naming_register_cmpt() - register the component & return clist of caps
 *
 ***************************************************************************/
int 
naming_register_cmpt( cap_t* pdx_objects, int pdx_obj_count )
{
	int r; 
	l4_threadid_t me;

	/* Hello Everyone */
	me = l4_myself();
	printf( "Registering tree component\n" );
	printf( "My id is 0x%p\n", (void*)(me.ID) );

/****************************************************************************
 *
 * Register components 
 *
 ****************************************************************************/

	gepr = (pdx_t)((unsigned long)nameserver_get + PDXDELTA);
	sepr = (pdx_t)((unsigned long)nameserver_set + PDXDELTA);
	
	printf( "\n" );
	printf( " **** REGISTER COMPONENTS ****\n" );
	r = pax_regmap_register_components( pdx_objects, pdx_obj_count );
	assert( r > 0 );
	printf( " *****************************\n\n" );
	
	/* worked */
	return r;
}


/* insert a set of caps into our APD and save them for fexec() */
void 
naming_set_cmpt( cap_t* caplist, int num_caps )
{
	int r, i;
	
	/* create a new clist */
	r = create_clist_and_add( PAGESIZE, &namelist_cap );
	assert( r == 0 );

	/* copy in caps */
	printf("name: Copying %d caps\n", num_caps );
	namelist_list = ((clist_t*)namelist_cap.address);
	/*
	namelist_list->n_caps = num_caps;
	memcpy( &namelist_list->caps[0], caplist, num_caps );
	*/

	for( i = 0; i < num_caps; i++ ) {
		printf( "name: adding cap 0x%p\n", caplist[i].address );
		add_to_clist( caplist[i], namelist_list );
	}

	/* GEPR and SEPR?? */
}


/* a simple type for easy serialising.
 * Only serialise one namespace set for now - others is a bit tricky
 * and I just want to get this thing working! -ceg
 */
#define MAX_NAME_CAPS 10
struct naming_env
{
/*
	cap_t gepr;
	cap_t sepr;
*/
	/* FIXME: these prolly need caps to get to them!! */
	pdx_t gepr;
	pdx_t sepr;

	int num_caps;
	cap_t cap_list[MAX_NAME_CAPS];

	cicap_t root_cap;
};

/* Create a tree from a CICAP */
static INameTree_t *
create_namespace( cicap_t cap )
{
	namehash_t *nh;
	INameTree_t *tree;
	environment_t ev;
	int size = 2 * PAGESIZE;
	cap_t pb;
	int r;

	/* creat the param buffer */
	r = create_simple_object( size, &pb );
	assert( r == 0 );

	/* init our new namespace */
	nh = nh_init( 100, 100 );  /* FIXME: mmmm... magic */
	assert( nh != NULL );

	r = p9_init( nh, pb );
	assert( r == 0 );

	/* create a tree from the CICAP */
	tree = int_cicap_constructor( cap, pb, size, &ev );
	UserPrint( "new nametree is 0x%llx\n", tree );

	/* mount the tree as root */
	r = p9_mount( "/", tree, NMF_READWRITE );

	if( r != NM_OK )
		UserPrint( "Error on mount\n" );

        return NULL;
}

/* Serialise and un-serialise for the environment */

extern cicap_t p9_get_default_cicap(void);

size_t
_naming_serialise(void* dest)
{
	struct naming_env *data;

	UserPrint( "Serialising naming...\n" );

	/* Serialising naming:
	   - gepr/sepr
	   - clist
	   - mount points (ack!)
	      - incl. CICAPS
	      - only doing one for now (root)
	*/

	data = (struct naming_env *)dest;

	data->gepr = gepr;
	data->sepr = sepr;

	data->num_caps = (MAX_NAME_CAPS < namelist_list->n_caps) ? 
		MAX_NAME_CAPS : namelist_list->n_caps ;
	

	UserPrint( "num_caps is 0x%llx\n", data->num_caps );

	/* copy the clist */
	memcpy( data->cap_list, namelist_list->caps, 
		data->num_caps * sizeof( cap_t ) );

	/* copy the CICAP */
	data->root_cap = p9_get_default_cicap();

	return sizeof(*data);
}

int 
_naming_deserialise(void * env_data,size_t size)
{
	struct naming_env *data;
	INameTree_t *tree;
	int r;

	UserPrint( "Deserialising naming...\n" );

	data = (struct naming_env *) env_data;
	assert( size == sizeof( *data ) );

	/* create the clist */
	UserPrint( "Creating clist\n" );
	r = create_clist_and_add( PAGESIZE, &namelist_cap );
	assert( r == 0 );

	gepr = data->gepr;
	sepr = data->sepr;

	/* set the component */
	UserPrint( "Setting component\n" );
	naming_set_cmpt( data->cap_list, data->num_caps );

	/* create the name tree & mount */
	UserPrint( "creating namespace\n" );
	tree = create_namespace( data->root_cap );

	UserPrint( "returning\n" );
	return 0;
}

