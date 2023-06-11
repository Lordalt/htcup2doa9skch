/*
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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


#ifndef CONFIGURATION_ATTRIBUTE_H_
#define CONFIGURATION_ATTRIBUTE_H_

typedef struct configuration_attribute_t configuration_attribute_t;

#include <library.h>
#include <attributes/attributes.h>
#include <encoding/payloads/payload.h>

struct configuration_attribute_t {

	payload_t payload_interface;

	configuration_attribute_type_t (*get_type)(configuration_attribute_t *this);

	chunk_t (*get_chunk) (configuration_attribute_t *this);

	u_int16_t (*get_value) (configuration_attribute_t *this);

	void (*destroy) (configuration_attribute_t *this);
};

configuration_attribute_t *configuration_attribute_create(payload_type_t type);

configuration_attribute_t *configuration_attribute_create_chunk(
	payload_type_t type, configuration_attribute_type_t attr_type, chunk_t chunk);

configuration_attribute_t *configuration_attribute_create_value(
					configuration_attribute_type_t attr_type, u_int16_t value);

#endif 
