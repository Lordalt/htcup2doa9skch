/*
 * Copyright (C) 2010-2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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


#ifndef SOCKET_MANAGER_H_
#define SOCKET_MANAGER_H_

#include <network/socket.h>

typedef struct socket_manager_t socket_manager_t;

struct socket_manager_t {

	status_t (*receive)(socket_manager_t *this, packet_t **packet);

	status_t (*send)(socket_manager_t *this, packet_t *packet);

	u_int16_t (*get_port)(socket_manager_t *this, bool nat_t);

	socket_family_t (*supported_families)(socket_manager_t *this);

	void (*add_socket)(socket_manager_t *this, socket_constructor_t create);

	void (*remove_socket)(socket_manager_t *this, socket_constructor_t create);

	void (*destroy)(socket_manager_t *this);
};

socket_manager_t *socket_manager_create();

#endif 
