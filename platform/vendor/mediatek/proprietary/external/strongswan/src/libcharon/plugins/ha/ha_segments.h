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


#ifndef HA_SEGMENTS_H_
#define HA_SEGMENTS_H_

#include <daemon.h>

typedef struct ha_segments_t ha_segments_t;

typedef u_int16_t segment_mask_t;

#define SEGMENTS_MAX (sizeof(segment_mask_t)*8)

#define SEGMENTS_BIT(segment) (0x01 << (segment - 1))

#include "ha_socket.h"
#include "ha_tunnel.h"
#include "ha_kernel.h"

struct ha_segments_t {

	listener_t listener;

	void (*activate)(ha_segments_t *this, u_int segment, bool notify);

	void (*deactivate)(ha_segments_t *this, u_int segment, bool notify);

	void (*handle_status)(ha_segments_t *this, segment_mask_t mask);

	bool (*is_active)(ha_segments_t *this, u_int segment);

	void (*destroy)(ha_segments_t *this);
};

ha_segments_t *ha_segments_create(ha_socket_t *socket, ha_kernel_t *kernel,
								  ha_tunnel_t *tunnel, u_int count, u_int node,
								  bool monitor);

#endif 
