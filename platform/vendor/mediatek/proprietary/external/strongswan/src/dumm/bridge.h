/*
 * Copyright (C) 2007 Martin Willi
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

#ifndef BRIDGE_H
#define BRIDGE_H

#include <library.h>
#include <collections/enumerator.h>

typedef struct bridge_t bridge_t;

#include "iface.h"

struct bridge_t {

	char* (*get_name)(bridge_t *this);

	bool (*connect_iface)(bridge_t *this, iface_t *iface);

	bool (*disconnect_iface)(bridge_t *this, iface_t *iface);

	enumerator_t* (*create_iface_enumerator)(bridge_t *this);

	void (*destroy) (bridge_t *this);
};

bridge_t *bridge_create(char *name);

#endif 

