/*
 * Copyright (C) 2008 Martin Willi
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


#ifndef STROKE_ATTRIBUTE_H_
#define STROKE_ATTRIBUTE_H_

#include <stroke_msg.h>
#include <attributes/attribute_provider.h>
#include <attributes/mem_pool.h>

typedef struct stroke_attribute_t stroke_attribute_t;

struct stroke_attribute_t {

	attribute_provider_t provider;

	void (*add_pool)(stroke_attribute_t *this, mem_pool_t *pool);

	void (*add_dns)(stroke_attribute_t *this, stroke_msg_t *msg);

	void (*del_dns)(stroke_attribute_t *this, stroke_msg_t *msg);

	enumerator_t* (*create_pool_enumerator)(stroke_attribute_t *this);

	enumerator_t* (*create_lease_enumerator)(stroke_attribute_t *this,
											 char *pool);

	void (*destroy)(stroke_attribute_t *this);
};

stroke_attribute_t *stroke_attribute_create();

#endif 
