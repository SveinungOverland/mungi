/****************************************************************************
 *
 *      $Id: pending_requests.c,v 1.2 2002/05/31 06:27:40 danielp Exp $
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

/* table of pending user requests, used by the syscall handler and mpager */

#include "mungi/kernel.h"
#include "mungi/lock.h"
#include "mungi/syscallbits.h"
#include "mungi/pending_requests.h"

#define MAX_PENDING_REQUESTS 64

static struct pending_request requests[MAX_PENDING_REQUESTS];
static lock_t requests_lock;

void
pending_requests_init(void)
{
	int i;

	_lock_init(&requests_lock);
	for (i = 0; i < MAX_PENDING_REQUESTS; i++)
		requests[i].slot_used = false;
}

struct pending_request *
pending_requests_get_slot(void)
{
	int i;
	struct pending_request *ret = NULL;

	_lock_lock(&requests_lock);
	for (i = 0; i < MAX_PENDING_REQUESTS; i++)
		if (!requests[i].slot_used) {
			requests[i].slot_used = true;
			ret = &requests[i];
			break;
		}
	_lock_unlock(&requests_lock);

	if (ret == NULL)
		assert(!"out of space for pending requests, expect badness");
	return ret;
}

void
pending_requests_free_slot(struct pending_request *request)
{
	request->slot_used = false;
}
