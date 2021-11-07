/****************************************************************************
 *
 *      $Id: CNameTree_ccs.c,v 1.4 2002/07/31 07:04:44 cgray Exp $
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
 * CNameTree_ccs.c
 *
 * Component stub implementations for CNameTree.
 * Autogenerated by PaxGen-0.1 on Tue Oct 23 11:42:46 2001
 *
 **************************************************************************************************/

/* Includes */
#include "tree.h"
#include "CNameTree_cc.h"
#include <string.h>
#include <assert.h>


/* FIXME: the pax compiler shouldn't spit out such seedy code */
static cihdr_t **instobj = NULL;
static clist_t *_clist = NULL;

#define PARAM_CAP_POS 1

/* INameTree interface */
res_desc_t cnt_int_stub_create( cap_t param )
{
	cicap_t _cicap;
	cihdr_t *_hdr;
	cnt_t *_inst;
	btcerror_t _rv;
	res_desc_t _retval;
	void **_pb;
	cap_t _obj;
	long _name_offset;
	long _flags;
	long _value;

	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* unpack parameters */
	_pb = (void**)param.address;
	_cicap = *((cicap_t*)_pb);
	_hdr = (cihdr_t*)_cicap.ref;
	_inst = (cnt_t*)(void *)((char*)_hdr + sizeof(cihdr_t) + (sizeof(cihdr_t)%sizeof(void*)));
	_pb += 2;
	_obj = *((cap_t *)_pb);
	_pb += ((sizeof(cap_t)/sizeof(void*)) + 1);
	_name_offset = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_flags = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_value = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);

	/* validate access */
	if( IS_NOT_VALID(_hdr) || validate_cicap(_hdr,_cicap) == 0 ) {
		_retval.type = MCS_STUB_EXCEPTION;
		_retval.data1 = MCS_STE_PROT;
		_retval.data1 <<= 48;
		return _retval;
	}

	/* invoke the method */
	_rv = cnt_int_create( _inst, _obj, _name_offset, _flags, _value );

	/* setup result descriptor */
	_retval.type = MCS_NO_EXCEPTION;
	_retval.data1 = (unsigned long)_rv;
	_retval.data0 = 0;
	return _retval;
}

res_desc_t cnt_int_stub_delete( cap_t param )
{
	cicap_t _cicap;
	cihdr_t *_hdr;
	cnt_t *_inst;
	btcerror_t _rv;
	res_desc_t _retval;
	void **_pb;
	cap_t _obj;
	long _name_offset;

	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* unpack parameters */
	_pb = (void**)param.address;
	_cicap = *((cicap_t*)_pb);
	_hdr = (cihdr_t*)_cicap.ref;
	_inst = (cnt_t*)(void *)((char*)_hdr + sizeof(cihdr_t) + (sizeof(cihdr_t)%sizeof(void*)));
	_pb += 2;
	_obj = *((cap_t *)_pb);
	_pb += ((sizeof(cap_t)/sizeof(void*)) + 1);
	_name_offset = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);

	/* validate access */
	if( IS_NOT_VALID(_hdr) || validate_cicap(_hdr,_cicap) == 0 ) {
		_retval.type = MCS_STUB_EXCEPTION;
		_retval.data1 = MCS_STE_PROT;
		_retval.data1 <<= 48;
		return _retval;
	}

	/* invoke the method */
	_rv = cnt_int_delete( _inst, _obj, _name_offset );

	/* setup result descriptor */
	_retval.type = MCS_NO_EXCEPTION;
	_retval.data1 = (unsigned long)_rv;
	_retval.data0 = 0;
	return _retval;
}

res_desc_t cnt_int_stub_lookup( cap_t param )
{
	cicap_t _cicap;
	cihdr_t *_hdr;
	cnt_t *_inst;
	btcerror_t _rv;
	res_desc_t _retval;
	void **_pb;
	cap_t _obj;
	long _name_offset;
	int _ret_flags;
	int _ret_offset;

	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* unpack parameters */
	_pb = (void**)param.address;
	_cicap = *((cicap_t*)_pb);
	_hdr = (cihdr_t*)_cicap.ref;
	_inst = (cnt_t*)(void *)((char*)_hdr + sizeof(cihdr_t) + (sizeof(cihdr_t)%sizeof(void*)));
	_pb += 2;
	_obj = *((cap_t *)_pb);
	_pb += ((sizeof(cap_t)/sizeof(void*)) + 1);
	_name_offset = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_ret_flags = *((int *)_pb);
	_pb += ((sizeof(int)/sizeof(void*)) + 1);
	_ret_offset = *((int *)_pb);
	_pb += ((sizeof(int)/sizeof(void*)) + 1);

	/* validate access */
	if( IS_NOT_VALID(_hdr) || validate_cicap(_hdr,_cicap) == 0 ) {
		_retval.type = MCS_STUB_EXCEPTION;
		_retval.data1 = MCS_STE_PROT;
		_retval.data1 <<= 48;
		return _retval;
	}

	/* invoke the method */
	_rv = cnt_int_lookup( _inst, _obj, _name_offset, _ret_flags, _ret_offset );

	/* setup result descriptor */
	_retval.type = MCS_NO_EXCEPTION;
	_retval.data1 = (unsigned long)_rv;
	_retval.data0 = 0;
	return _retval;
}

res_desc_t cnt_int_stub_modify( cap_t param )
{
	cicap_t _cicap;
	cihdr_t *_hdr;
	cnt_t *_inst;
	btcerror_t _rv;
	res_desc_t _retval;
	void **_pb;
	cap_t _obj;
	long _name_offset;
	long _new_name_offset;
	long _flags;
	long _value;

	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* unpack parameters */
	_pb = (void**)param.address;
	_cicap = *((cicap_t*)_pb);
	_hdr = (cihdr_t*)_cicap.ref;
	_inst = (cnt_t*)(void *)((char*)_hdr + sizeof(cihdr_t) + (sizeof(cihdr_t)%sizeof(void*)));
	_pb += 2;
	_obj = *((cap_t *)_pb);
	_pb += ((sizeof(cap_t)/sizeof(void*)) + 1);
	_name_offset = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_new_name_offset = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_flags = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_value = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);

	/* validate access */
	if( IS_NOT_VALID(_hdr) || validate_cicap(_hdr,_cicap) == 0 ) {
		_retval.type = MCS_STUB_EXCEPTION;
		_retval.data1 = MCS_STE_PROT;
		_retval.data1 <<= 48;
		return _retval;
	}

	/* invoke the method */
	_rv = cnt_int_modify( _inst, _obj, _name_offset, _new_name_offset, _flags, _value );

	/* setup result descriptor */
	_retval.type = MCS_NO_EXCEPTION;
	_retval.data1 = (unsigned long)_rv;
	_retval.data0 = 0;
	return _retval;
}

res_desc_t cnt_int_stub_rlookup( cap_t param )
{
	cicap_t _cicap;
	cihdr_t *_hdr;
	cnt_t *_inst;
	btcerror_t _rv;
	res_desc_t _retval;
	void **_pb;
	cap_t _obj;
	long _name_offset;
	long _start;
	long _overflow;
	long _recv_off;
	long _recv_len;
	long _num;
	long _value;

	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* unpack parameters */
	_pb = (void**)param.address;
	_cicap = *((cicap_t*)_pb);
	_hdr = (cihdr_t*)_cicap.ref;
	_inst = (cnt_t*)(void *)((char*)_hdr + sizeof(cihdr_t) + (sizeof(cihdr_t)%sizeof(void*)));
	_pb += 2;
	_obj = *((cap_t *)_pb);
	_pb += ((sizeof(cap_t)/sizeof(void*)) + 1);
	_name_offset = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_start = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_overflow = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_recv_off = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_recv_len = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_num = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_value = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);

	/* validate access */
	if( IS_NOT_VALID(_hdr) || validate_cicap(_hdr,_cicap) == 0 ) {
		_retval.type = MCS_STUB_EXCEPTION;
		_retval.data1 = MCS_STE_PROT;
		_retval.data1 <<= 48;
		return _retval;
	}

	/* invoke the method */
	_rv = cnt_int_rlookup( _inst, _obj, _name_offset, _start, _overflow, _recv_off, _recv_len, _num, _value );

	/* setup result descriptor */
	_retval.type = MCS_NO_EXCEPTION;
	_retval.data1 = (unsigned long)_rv;
	_retval.data0 = 0;
	return _retval;
}

res_desc_t cnt_int_stub_fenum( cap_t param )
{
	cicap_t _cicap;
	cihdr_t *_hdr;
	cnt_t *_inst;
	btcerror_t _rv;
	res_desc_t _retval;
	void **_pb;
	cap_t _obj;
	long _name_offset;
	long _start;
	long _overflow;
	long _recv_off;
	long _recv_len;
	long _num;
	long _value;

	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* unpack parameters */
	_pb = (void**)param.address;
	_cicap = *((cicap_t*)_pb);
	_hdr = (cihdr_t*)_cicap.ref;
	_inst = (cnt_t*)(void *)((char*)_hdr + sizeof(cihdr_t) 
			 + (sizeof(cihdr_t)%sizeof(void*)));
	_pb += 2;
	_obj = *((cap_t *)_pb);
	_pb += ((sizeof(cap_t)/sizeof(void*)) + 1);
	_name_offset = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_start = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_overflow = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_recv_off = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_recv_len = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_num = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);
	_value = *((long *)_pb);
	_pb += ((sizeof(long)/sizeof(void*)) + 1);

	/* validate access */
	if( IS_NOT_VALID(_hdr) || validate_cicap(_hdr,_cicap) == 0 ) {
		_retval.type = MCS_STUB_EXCEPTION;
		_retval.data1 = MCS_STE_PROT;
		_retval.data1 <<= 48;
		return _retval;
	}

	/* invoke the method */
	_rv = cnt_int_fenum( _inst, _obj, _name_offset, _start, _overflow, 
			     _recv_off, _recv_len, _num, _value );

	/* setup result descriptor */
	_retval.type = MCS_NO_EXCEPTION;
	_retval.data1 = (unsigned long)_rv;
	_retval.data0 = 0;
	return _retval;
}



/* IClassInterface interface stubs */
static int get_instobj( res_desc_t *res )
{
    apddesc_t apd;
    int i, found = 0, inst_size, inst_word_size, count;
    clist_t *clist = 0;
    cap_t newio;
    void **ptr;
    objinfo_t objinfo;

    /* get the APD and find the inst_obj */
    ApdGet( &apd );
    for( i = 0; i < apd.n_apd; i++ ) {
	clist = (clist_t*)apd.clist[i].address;
	if( clist != 0 && clist->format == CL_PROTCTX ) {
	    _clist = clist;
	    instobj = clist->caps[0].address;
	    found = 1;
	    break;
	}
    }

    /* if there is no such clist then throw an exception */
    if( found == 0 ) {
	res->type = MCS_STUB_EXCEPTION;
	res->data1 = MCS_STE_NOINSTOBJ;
	res->data1 <<= 48;
	return -1;
    }

    /* if there is no inst_obj then create and format it */
    if( instobj == NULL ) {

	/* create the object and add cap to PD */
	memset( &objinfo, 0, sizeof(objinfo) );
	objinfo.userinfo = (void*)CID_CNAMETREE;
	newio.passwd = generate_password();
	newio.address = ObjCreate( PAGESIZE, newio.passwd, &objinfo );
	if( newio.address == NULL ) {
	    res->type = MCS_STUB_EXCEPTION;
	    res->data1 = MCS_STE_CREIOFAILED;
	    res->data1 <<=48;
	    return -1;
	}
	clist->caps[0] = newio;
	
	/* format the object */
	inst_size = sizeof(cihdr_t) + (sizeof(cihdr_t)%sizeof(void*)) + sizeof(cnt_t) + ((sizeof(cnt_t)%sizeof(void*)));
	inst_word_size = inst_size / sizeof(void*);
	count = (PAGESIZE - sizeof(void*))/inst_size;
	ptr = (void**)newio.address;
	*ptr = (void**)(void *)((char*)ptr + sizeof(void*));
	ptr++;
	for( i = 0; i < (count-1); i++ ) {
	    ptr[i*inst_word_size] = &(ptr[(i+1)*inst_word_size]);
	}
	ptr[i*inst_word_size] = 0;

	/* cache the ptr in a global var */
	instobj = newio.address;
    }
    return 0;
}


res_desc_t cnt_ci_stub_constructor( cap_t param )
{
	cihdr_t *hdr;
	res_desc_t res;
	cnt_t *ref;
	int r;

	/* Find an address to store the new instance */
	if( instobj == 0  ) {
	    r = get_instobj( &res );
	    if( r != 0 )
		return res;		
	}	
	hdr = *instobj;
	if( hdr == NULL ) {
	    res.type = MCS_STUB_EXCEPTION;
	    res.data1 = MCS_STE_INSTLIMIT;
	    res.data1 <<= 48;
	    return res;
	}
	*instobj = *((cihdr_t**)hdr);

	/* Initialise the static header and the d1-cicap */
	memset( hdr, 0, sizeof(cihdr_t) );
	hdr->opasswd = generate_password();
	hdr->hv = 1;
	hdr->vo = hdr->vl = CSGC_MINOR_VERSION;
	hdr->flags = MCS_IF_VALID;

	/* Create a new language instance */
	ref = (cnt_t*)&(hdr->dpasswd[1]);
	memset( ref, 0, sizeof(cnt_t) );

	/* The param is the PB - so add to clist */
	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* Return */
	res.type = MCS_NO_EXCEPTION;
	res.data1 = hdr->opasswd;
	res.data0 = ((unsigned long)hdr)>>2;
	return res;
}


res_desc_t cnt_ci_stub_destructor( cap_t param )
{
	cicap_t _cicap;
	cihdr_t *hdr;
	res_desc_t res;
	int r;

	add_to_clist_pos( param, _clist, PARAM_CAP_POS );

	/* Validate the passed CICAP */
	_cicap = *((cicap_t*)&param);
	hdr = (cihdr_t*)_cicap.ref;
	if( ((hdr->flags & MCS_IF_VALID) == 0) || hdr->opasswd != _cicap.password ) {
		res.type = MCS_STUB_EXCEPTION;
		res.data1 = MCS_STE_PROT;
		res.data1 <<= 48;
		return res;
	}

	/* Get the address of the instance object */
	if( instobj == 0  ) {
	    r = get_instobj( &res );
	    if( r != 0 )
		return res;		
	}	

	/* Free the block */
	hdr->flags = 0;
	*((cihdr_t**)hdr) = *instobj;
	*instobj = hdr;

	/* Return */
	res.type = MCS_NO_EXCEPTION;
	return res;
}

res_desc_t cnt_ci_stub_create_cicap( cap_t param )
{
	cicap_t cicap;
	cihdr_t *hdr;
	res_desc_t res;
	void **pb;
	char slot;
	unsigned short ef;

	/* Unpack parameters */
	pb = (void**)param.address;
	cicap = *((cicap_t*)pb);
	pb += 2;
	slot = *((char*)pb);
	pb += 1;
	ef = *((unsigned short*)pb);

	/* Validate the passed CICAP */
	hdr = (cihdr_t*)cicap.ref;
	if( ((hdr->flags & 0x1) == 0) || hdr->opasswd != cicap.password ) {
		res.type = MCS_STUB_EXCEPTION;
		res.data1 = MCS_STE_PROT;
		return res;
	}

	/* Create new E-CICAP */
	hdr->epasswd[slot%3] = generate_password();
	hdr->ef[slot%3] = ef;

	/* Return */
	res.type = 0;
	res.data1 = hdr->epasswd[slot%3];
	return res;
}

res_desc_t cnt_ci_stub_get_typeinfo( cap_t param )
{
	res_desc_t res;
	res.type = MCS_STUB_EXCEPTION;
	res.data1 = 0;
	return res;
}

