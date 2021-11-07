/****************************************************************************
 *
 *      $Id: mlib.c,v 1.5 2002/07/31 07:04:41 cgray Exp $
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

#include <l4/types.h>
#include <mungi.h>
#include <mlib.h>
#include <assert.h>

extern int atob (unsigned int *vp, char *p, int base);
extern void *malloc_Z(int size);
void *memset_Z(void *addr, int c, int s);
void free_Z(void *addr );
void start_timer(int start_value);


void UserPrint( char *fmt, ... );

/* new fn. by CEG */
void add_to_clist_pos(cap_t cap, void *clist, int pos)
{
	int cc;
	clist_t *cl;

	cl = (clist_t*)clist;
	cc = cl->n_caps;
	cl->caps[pos] = cap;
	if (cl->n_caps <= pos)
		cl->n_caps = pos + 1;
}


void add_to_clist( cap_t cap, void *clist )
{
	int cc;
	clist_t *cl;

	cl = (clist_t*)clist;
	cc = cl->n_caps;
	cl->caps[cc] = cap;
	cc += 1;
	cl->n_caps = cc;
}

int create_clist_and_add( int size, cap_t *clist )
{
	int r, slot = -1, i;
	apddesc_t apd;

	/* create the clist object and initialise */
	r = create_clist( size, clist );
	assert( r == 0 );

	/* insert clist into APD */
	r = ApdGet( &apd );
	assert( r == 0 );
	for( i = apd.n_locked; i < APD_MAX_ENTRY; i++ ) {
		if( apd.clist[i].address == NULL ) {
			slot = i;
			break;
		}
	}
	assert( slot != -1 );
	r = ApdInsert( slot, (clist_t*)(clist->address) ); 
	assert( r == 0 );

	/* done */
	return 0;
}

int create_clist( int size, cap_t *clist )
{
	int r;
	clist_t *cl;
	r = create_simple_object( size, clist );
	assert(r == 0 );
	cl = (clist_t*)(clist->address);
	init_clist( cl );
	return 0;
}

/* This create a new object, and a PD which contains one clist with the
 * cap to this new object. */
int create_object_and_pd( int size, cap_t *obj, cap_t *pd )
{
	int r, slot;
	apddesc_t *npd;
	clist_t *cl = NULL;
	cap_t clist, *l;

	assert( obj != NULL );
	assert( pd != NULL );

	/* create the object */
	r = create_simple_object( size, obj );
	if( r != 0 )
		return r;

	/* create the pd */
	r = create_simple_object( PAGESIZE, pd );
	if( r != 0 ) {
		ObjDelete( obj->address );
		return r;
	}

	/* create the clist */
	r = create_simple_object( PAGESIZE, &clist );
	if( r != 0 ) {
		ObjDelete( obj->address );
		ObjDelete( pd->address );
		return r;
	}

	/* fill in the clist */
	cl = (clist_t*)clist.address;
	assert( cl != 0 );
	init_clist( cl );
	assert( cl != 0 );
	slot = cl->n_caps;
	assert( cl != 0 );
	l = cl->caps;
	assert( cl != 0 );
	l[slot] = *obj;
	slot += 1;
	cl->n_caps = slot;
	assert( cl != 0 );

	/* fill in the pd */
	npd = (apddesc_t*)pd->address;
	assert( npd != NULL );
	npd->n_locked = 0;
	npd->n_apd = 2;
	assert( npd != NULL );
	npd->clist[0].address = NULL;
	npd->clist[1] = clist;

	/* done */
	return 0;
}

int delete_object_and_pd( void *obj, void *pd )
{
	int r = 0;
//	r = ObjDelete( obj );
//	r |= ObjDelete( ((apddesc_t*)pd)->clist[1].address );
//	r |= ObjDelete( pd );
	return r;
}

int create_simple_object( int size, cap_t *obj )
{
	int r, i, slot;
	apddesc_t apd;
	clist_t *cl = NULL;
	cap_t *l;

	/* get the local APD so that this thread can access them */
	r = ApdGet( &apd );
	if( r != 0 ) {
		return r;
	}

	/* find an unlocked clist and add the cap. If none is available then we fail */
	for( i = apd.n_locked; i < APD_MAX_ENTRY; i++ ) {
		if( apd.clist[i].address != NULL ) {
			cl = (clist_t*)apd.clist[i].address;
			break;
		}
	}
	if( cl == NULL ) {
		return -1;
	}

	/* create the object - and add its cap to the clist */
	obj->passwd = create_new_passwd();
	obj->address = ObjCreate( size, obj->passwd, NULL );
	if( obj->address == NULL ) {
		return -1;
	}

	slot = cl->n_caps;
	l = cl->caps;
	l[slot] = *obj;
	slot += 1;
	cl->n_caps = slot;

	//	printf( "PAXLIB: new object = 0x%p\n", obj->address );

	/* OK */
	return 0;
}


/**/

int 
apd_insert_cap( cap_t obj )
{
	int r, i, slot;
	apddesc_t apd;
	clist_t *cl = NULL;
	cap_t *l;

	/* get the local APD so that this thread can access them */
	r = ApdGet( &apd );
	if( r != 0 ) {
		return r;
	}

	/* find an unlocked clist and add the cap. 
	   If none is available then we fail */
	for( i = apd.n_locked; i < APD_MAX_ENTRY; i++ ) {
		if( apd.clist[i].address != NULL ) {
			cl = (clist_t*)apd.clist[i].address;
			break;
		}
	}
	if( cl == NULL ) {
		return -1;
	}

	slot = cl->n_caps;
	l = cl->caps;
	l[slot] = obj;
	slot += 1;
	cl->n_caps = slot;


	/* OK */
	return 0;
}




int init_clist( clist_t *clist )
{
	clist->type = 'c';					// FIXME: should be #defd
	clist->rel_ver = 1;					// FIXME: should be #defd
	clist->format = CL_UNSRT_0;
	clist->n_caps = 0;
	clist->reserved = 0;
	return 0;
}

unsigned long create_new_passwd()
{
	static unsigned long x = 4;
	return x++;
}

void _a_memcpy( char *dst, char *src, int s )
{
    int i;
    for( i = 0; i < s; i++ ) {
	dst[i] = src[i];
    }
}

int atob64( void **vp, char *p, int base )
{
    unsigned int v, r;
    r = atob( &v, p, base );
    *vp = (void*)((long)v);
    return r;
}

static cap_t mobj = { 0, 0};
static void *end;
#define MALLOC_SIZE 12000

void *malloc_Z( int size )
{
    int r;
    void *rv;

    /* If no malloc obj has been allocated, create it */
    if( mobj.address == 0 ) {
	mobj.passwd = 5;
	r = create_simple_object( MALLOC_SIZE, &mobj );
	assert( r == 0 );
	end = (void*)((unsigned long)mobj.address + MALLOC_SIZE);
    }

    /* Align size */
    size += (sizeof(void*) - size%sizeof(void*));

    /* Allocate */
    rv = mobj.address;
    mobj.address = (void*)((unsigned long)mobj.address + size);
    if( mobj.address >= end ) {
	assert( !"OUT OF MEMORY" );
    }
    return rv;
}

void *memset_Z( void *addr, int c, int s )
{
    int i;
    for( i = 0; i < s; i++ ) {
	((char*)addr)[i] = c;
    }
    return addr;
}

void free_Z( void *addr )
{
    return;
}

#define GT_BASE 0x14000000
#define gtaddr32(x) ((volatile unsigned int *)(GT_BASE + (x)))
#define byte_swap32(x) ((unsigned int)((((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))

void start_timer( int start_value )
{
#ifdef MIPSENV
    /* mask GT iterrupts and disable all timers - should be done already but
       just in case */
    *gtaddr32(0xc1c)= 0;
    *gtaddr32(0x864)= 0;  
    
    /* set timer interval */
    *gtaddr32(0x850)= byte_swap32(start_value);
    
    /* unmask timer0 interrupt and start timer0 */
    *gtaddr32(0xc1c)= byte_swap32(0x100);
    *gtaddr32(0x864)= byte_swap32(0x3);    
#endif
}

int get_time( )
{
#ifdef MIPSENV
    return (byte_swap32(*gtaddr32(0x850)));
#else
    return 0;
#endif
}

