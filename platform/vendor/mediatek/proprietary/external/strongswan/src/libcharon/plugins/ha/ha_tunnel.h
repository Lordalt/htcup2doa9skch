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


#ifndef HA_TUNNEL_H_
#define HA_TUNNEL_H_

#include <sa/ike_sa.h>

typedef struct ha_tunnel_t ha_tunnel_t;

struct ha_tunnel_t {

	bool (*is_sa)(ha_tunnel_t *this, ike_sa_t *ike_sa);

	void (*destroy)(ha_tunnel_t *this);
};

ha_tunnel_t *ha_tunnel_create(char *local, char *remote, char *secret);

#endif 
