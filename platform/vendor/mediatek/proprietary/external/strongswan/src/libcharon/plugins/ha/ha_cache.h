/*
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


#ifndef HA_CACHE_H_
#define HA_CACHE_H_

typedef struct ha_cache_t ha_cache_t;

#include "ha_message.h"
#include "ha_kernel.h"
#include "ha_socket.h"

#include <collections/enumerator.h>

#include <sa/ike_sa.h>

struct ha_cache_t {

	void (*cache)(ha_cache_t *this, ike_sa_t *ike_sa, ha_message_t *message);

	void (*delete)(ha_cache_t *this, ike_sa_t *ike_sa);

	void (*resync)(ha_cache_t *this, u_int segment);

	void (*destroy)(ha_cache_t *this);
};

ha_cache_t *ha_cache_create(ha_kernel_t *kernel, ha_socket_t *socket,
							bool resync, u_int count);

#endif 
