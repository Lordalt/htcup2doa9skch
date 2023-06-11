/*
 * Copyright (C) 2009 Martin Willi
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


#ifndef ATTRIBUTE_HANDLER_H_
#define ATTRIBUTE_HANDLER_H_

#include <utils/chunk.h>
#include <utils/identification.h>
#include <collections/linked_list.h>

#include "attributes.h"

typedef struct attribute_handler_t attribute_handler_t;

struct attribute_handler_t {

	bool (*handle)(attribute_handler_t *this, identification_t *server,
				   configuration_attribute_type_t type, chunk_t data);

	void (*release)(attribute_handler_t *this, identification_t *server,
					configuration_attribute_type_t type, chunk_t data);

	enumerator_t* (*create_attribute_enumerator)(attribute_handler_t *this,
								identification_t *server, linked_list_t *vips);
};

#endif 
