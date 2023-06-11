/*
 * Copyright (C) 2010 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */


#ifndef MEM_POOL_H
#define MEM_POOL_H

typedef struct mem_pool_t mem_pool_t;
typedef enum mem_pool_op_t mem_pool_op_t;

#include <networking/host.h>
#include <utils/identification.h>

enum mem_pool_op_t {
	
	MEM_POOL_EXISTING,
	
	MEM_POOL_NEW,
	
	MEM_POOL_REASSIGN,
};

struct mem_pool_t {

	const char* (*get_name)(mem_pool_t *this);

	host_t* (*get_base)(mem_pool_t *this);

	u_int (*get_size)(mem_pool_t *this);

	u_int (*get_online)(mem_pool_t *this);

	u_int (*get_offline)(mem_pool_t *this);

	host_t* (*acquire_address)(mem_pool_t *this, identification_t *id,
							   host_t *requested, mem_pool_op_t operation);

	bool (*release_address)(mem_pool_t *this, host_t *address,
							identification_t *id);

	enumerator_t* (*create_lease_enumerator)(mem_pool_t *this);

	void (*destroy)(mem_pool_t *this);
};

mem_pool_t *mem_pool_create(char *name, host_t *base, int bits);

mem_pool_t *mem_pool_create_range(char *name, host_t *from, host_t *to);

#endif 
