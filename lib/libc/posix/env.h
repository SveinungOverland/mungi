/****************************************************************************
 *
 *      $Id: env.h,v 1.3.2.1 2002/08/30 06:00:06 cgray Exp $
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


#ifndef __ENV_H 
#define __ENV_H

struct env_header {
	char   magic[8];
	size_t size ;

};

struct env_rec {
	uintptr_t type; 
	size_t    size;
	/*  large chunk of data */
};

/*  FIXME: clean me up */
#define ENV_OBJ_SIZE  4096 /* page size from mlib  */
#define ENV_SIZE      (10 * ENV_OBJ_SIZE)
#define ENV_NOPASSWD  0
#define ENV_MAGIC     "PANTS!"

#define ENV_END        0 
#define ENV_ENV_VARS   1 
#define ENV_FILE_TABLE 2
#define ENV_NAMESPACE  3

struct env_filetable {
	int num_files ;
	_open_file  files[1] ;

};


/*FIX ME:  need more.*/


extern mthreadid_t  fexec(void *entry_point );
extern int  env_decode(void * env_data);

#endif /* __ENV_H*/
