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


#ifndef RESOLVE_HANDLER_H_
#define RESOLVE_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct resolve_handler_t resolve_handler_t;

struct resolve_handler_t {

	attribute_handler_t handler;

	void (*destroy)(resolve_handler_t *this);
};

resolve_handler_t *resolve_handler_create();

#endif 
