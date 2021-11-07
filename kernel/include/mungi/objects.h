/****************************************************************************
 *
 *      $Id: objects.h,v 1.8.2.1 2002/08/29 04:31:57 cgray Exp $
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

#ifndef __M_OBJECTS_H__
#define __M_OBJECTS_H__

typedef struct object object_t;

#include "mungi/types.h"
#include "mungi/lock.h"
#include "mungi/objects.h"
#include "mungi/semaphore.h"

struct object {
	void * base;
	void * end;
	objinfo_t info;
	lock_t lock;
	sem_t * semp;
};

#define IS_NOT_PDOBJECT(o)	(((o) == NULL) || (!((o)->info.special & O_PD)))

void objtable_init(void *);
object_t *object_find(const void *);
object_t *object_add(const void *, size_t, passwd_t);
object_t *object_create(size_t, objinfo_t *, passwd_t);
int object_resize(object_t *, size_t);
int object_info(object_t *, objinfo_t *, int);
int object_passwd(object_t *, passwd_t, access_t);
int object_delete(void *);
int object_pdx(object_t *, passwd_t, cap_t, uint, pdx_t *);
cap_t object_pdx_get_clist(object_t *, pdx_t, passwd_t);

void save_objtable(void **saved_objtable);
#endif /* __M_OBJECTS_H__ */
