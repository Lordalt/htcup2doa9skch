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


#ifndef HA_CHILD_H_
#define HA_CHILD_H_

typedef struct ha_child_t ha_child_t;

#include "ha_socket.h"
#include "ha_tunnel.h"
#include "ha_segments.h"
#include "ha_kernel.h"

#include <daemon.h>

struct ha_child_t {

	listener_t listener;

	void (*destroy)(ha_child_t *this);
};

ha_child_t *ha_child_create(ha_socket_t *socket, ha_tunnel_t *tunnel,
							ha_segments_t *segments, ha_kernel_t *kernel);

#endif 
