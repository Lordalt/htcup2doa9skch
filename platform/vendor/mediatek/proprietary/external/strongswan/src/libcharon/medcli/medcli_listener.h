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


#ifndef MEDCLI_LISTENER_H_
#define MEDCLI_LISTENER_H_

#include <bus/bus.h>
#include <database/database.h>

typedef struct medcli_listener_t medcli_listener_t;

struct medcli_listener_t {

	listener_t listener;

	void (*destroy)(medcli_listener_t *this);
};

medcli_listener_t *medcli_listener_create(database_t *database);

#endif 
