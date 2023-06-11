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


#ifndef NM_HANDLER_H_
#define NM_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct nm_handler_t nm_handler_t;

struct nm_handler_t {

	attribute_handler_t handler;

	enumerator_t* (*create_enumerator)(nm_handler_t *this,
									   configuration_attribute_type_t type);
	void (*reset)(nm_handler_t *this);

	void (*destroy)(nm_handler_t *this);
};

nm_handler_t *nm_handler_create();

#endif 
