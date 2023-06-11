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


#ifndef HA_SOCKET_H_
#define HA_SOCKET_H_

#include "ha_message.h"

#include <sa/ike_sa.h>

typedef struct ha_socket_t ha_socket_t;

struct ha_socket_t {

	void (*push)(ha_socket_t *this, ha_message_t *message);

	ha_message_t *(*pull)(ha_socket_t *this);

	void (*destroy)(ha_socket_t *this);
};

ha_socket_t *ha_socket_create(char *local, char *remote);

#endif 
