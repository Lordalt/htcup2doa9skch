/*
 * Copyright (C) 2008-2009 Martin Willi
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


#ifndef ATTRIBUTE_MANAGER_H_
#define ATTRIBUTE_MANAGER_H_

#include "attribute_provider.h"
#include "attribute_handler.h"

typedef struct attribute_manager_t attribute_manager_t;

struct attribute_manager_t {

	host_t* (*acquire_address)(attribute_manager_t *this,
							   linked_list_t *pool, identification_t *id,
							   host_t *requested);

	bool (*release_address)(attribute_manager_t *this,
							linked_list_t *pools, host_t *address,
							identification_t *id);

	enumerator_t* (*create_responder_enumerator)(attribute_manager_t *this,
									linked_list_t *pool, identification_t *id,
									linked_list_t *vips);

	void (*add_provider)(attribute_manager_t *this,
						 attribute_provider_t *provider);
	void (*remove_provider)(attribute_manager_t *this,
							attribute_provider_t *provider);

	attribute_handler_t* (*handle)(attribute_manager_t *this,
						identification_t *server, attribute_handler_t *handler,
						configuration_attribute_type_t type, chunk_t data);

	void (*release)(attribute_manager_t *this, attribute_handler_t *handler,
						identification_t *server,
						configuration_attribute_type_t type,
						chunk_t data);

	enumerator_t* (*create_initiator_enumerator)(attribute_manager_t *this,
									identification_t *id, linked_list_t *vips);

	void (*add_handler)(attribute_manager_t *this,
						attribute_handler_t *handler);

	void (*remove_handler)(attribute_manager_t *this,
						   attribute_handler_t *handler);

	void (*destroy)(attribute_manager_t *this);
};

attribute_manager_t *attribute_manager_create();

#endif 
