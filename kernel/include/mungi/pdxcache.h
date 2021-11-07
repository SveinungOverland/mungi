/****************************************************************************
 *
 *      $Id: pdxcache.h,v 1.2 2002/08/23 08:24:20 cgray Exp $
 *      Copyright (C) 2002 Operating Systems Research Group, UNSW, Australia.
 *
 *      This file is part of the Mungi operating system distribution.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      version 2 as published by the Free Software Foundation.
 *      A copy of this license is included in the top level directory of
 *      the Mungi distribution.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 ****************************************************************************/


/* PDX cache interface fuctions */
void pdxcache_init(void);
void pdxcache_flush(void);
void pdxcache_add(apd_t *, pdx_t, const apd_t *, struct apd_cache_id *);
apd_t *pdxcache_lookup(apd_t *, pdx_t, const apd_t *);

