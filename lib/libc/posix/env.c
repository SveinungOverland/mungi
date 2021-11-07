/****************************************************************************
 *
 *      $Id: env.c,v 1.4 2002/05/31 07:43:52 danielp Exp $
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

#include <mungi.h>
#include <assert.h>
#include <string.h>

/*DEBUG ONLY*/
#include <stdio.h>

#include <mlib.h>
#include "oftab.h"
#include "env.h"
#include <dirent.h>
#include <syscalls/userprint.h>

static uintptr_t align(uintptr_t val);
static struct env_rec *deserialise(struct env_rec * rec);

static uintptr_t
align(uintptr_t val){

	uintptr_t ret ;

	ret = val % sizeof(uintptr_t) ;
	if(ret !=0)
		val += sizeof(uintptr_t) -ret ;
	return val;
}


/**
 *
 * an implimentation for fork and exec at the same time.
 *
 */
mthreadid_t 
fexec(void *entry_point ){

	void * env_data ;
	size_t local_size;
	struct env_rec *local_header ;
	struct env_header *global_header ;
	cap_t  clist_obj,env_cap ;
	cap_t * dit_cap ;
	clist_t * child_clist ;
	int r ;
	cap_t child_pd_cap;
	apddesc_t *child_pd;
	mthreadid_t child_id ; 

	/*FIXME:  passwd might be a good idea.*/

	/* step 1: create object ( env )  */
	r = create_simple_object(ENV_SIZE, &env_cap);
	assert(r==0);

	/*adding cap for my self to avoid being killed by mungi*/
	global_header=env_cap.address ;
	strcpy(&global_header->magic[0],ENV_MAGIC);
	global_header->size=0;


	/* step 2: pack data into object  */
	 
	/*** adding file table ***/
	env_data = env_cap.address + sizeof(struct env_header) +
		sizeof(struct env_rec);
	
	/* serialise data and align data size */
	local_size = _oft_serialise(env_data);
	local_size = align(local_size);

	/* fill in local header */
	local_header =  env_cap.address + sizeof(struct env_header) ;
	local_header->type=ENV_FILE_TABLE ;
	local_header->size=local_size;
	global_header->size +=  sizeof(struct env_rec) + local_size ;


	/*** adding namespace stuff ***/

	/* serialise data and align data size */
	env_data += local_size + sizeof( struct env_rec );
	local_size = _naming_serialise(env_data);
	local_size = align(local_size);

	/* fill in local header */
	local_header = env_data - sizeof( struct env_rec );
	local_header->type = ENV_NAMESPACE ;
	local_header->size = local_size;
	global_header->size += sizeof(struct env_rec) + local_size;
	
	/*** mark the end of the env data ***/
	local_header = ((struct env_rec*) ((void *)local_header) + local_size);
	local_header->type=ENV_END ;
	local_header->size=0;
	global_header->size += sizeof(struct env_rec);
	
	/* step 3: create APD for child   */

	/* create clist and add caps */
	r = create_clist_and_add( ENV_OBJ_SIZE, &clist_obj );
	assert( r == 0 );

	/* we need access to the dit for code*/
	dit_cap = ApdLookup( entry_point, M_READ );
	assert( dit_cap != NULL );

	child_clist = (clist_t*) clist_obj.address;
	child_clist->caps[child_clist->n_caps++] = clist_obj;
	child_clist->caps[child_clist->n_caps++] = env_cap;
	child_clist->caps[child_clist->n_caps++] = *dit_cap;


	/* create apd */
	r = create_simple_object( ENV_OBJ_SIZE, &child_pd_cap );
	assert( r == 0 );

	child_pd = (apddesc_t*)(child_pd_cap.address);
	child_pd->n_locked = 0;
	child_pd->n_apd = 2;
	child_pd->clist[0].address = NULL;
	child_pd->clist[1] = clist_obj;

	/* step 4: thread create !        */ 	
	printf("fexec: send data from 0x%p with size %ld\n",
	       global_header,global_header->size);

	/* create a thread in the APD */
	child_id = ThreadCreate( entry_point, env_cap.address, 
				 NULL, child_pd );
	assert( child_id != THREAD_NULL );



	return child_id ;
}


int 
env_decode(void * env_data){

	struct env_header * global_header =  env_data;
	struct env_rec *    local_header ;
	int magic_val ;
        /*unpack the global header */
	magic_val = memcmp(global_header->magic, ENV_MAGIC,strlen(ENV_MAGIC));
	if(magic_val != 0){
		return 1 ;/*error: blame someone */
	}
		
	/* moving on to local data */
	local_header = env_data +sizeof(struct env_header);
		
		
	while(local_header->type != ENV_END){
		local_header = deserialise(local_header);
		if(local_header == NULL){
			return 1; /*error: decoding*/
		}
	}

	return 0;

}


static struct env_rec *
deserialise(struct env_rec * record ){

	void * ret = record ;
	void * data ;
	int r;

	/*calculate the return val*/
	ret += align(record->size) + sizeof(struct env_rec);
		
	data = ((void* ) record)+sizeof(*record);
	switch(record->type){

		case ENV_FILE_TABLE:
			r = _oft_deserialise(data,record->size);
			if(r!=0){
				return NULL;
			}
			break;

		case ENV_NAMESPACE:
			r = _naming_deserialise( data, record->size );
			if( r!=0 )
				return NULL;
			break;

		case ENV_END :	
			assert(!"YOU'RE A BAD PROGRAMMER!");
			return NULL ;
		default :
			UserPrint("WRN:unknown enviroment record.\n");		

	}
	return ret ;
}
