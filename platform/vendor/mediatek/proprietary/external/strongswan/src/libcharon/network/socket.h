/*
 * Copyright (C) 2006-2013 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
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


#ifndef SOCKET_H_
#define SOCKET_H_

typedef struct socket_t socket_t;
typedef enum socket_family_t socket_family_t;

#include <library.h>
#include <networking/packet.h>
#include <collections/enumerator.h>
#include <plugins/plugin.h>

typedef socket_t *(*socket_constructor_t)();

enum socket_family_t {
	SOCKET_FAMILY_NONE = 0,

	SOCKET_FAMILY_IPV4 = (1 << 0),

	SOCKET_FAMILY_IPV6 = (1 << 1),

	SOCKET_FAMILY_BOTH = (1 << 2) - 1,
};

struct socket_t {

	status_t (*receive)(socket_t *this, packet_t **packet);

	status_t (*send)(socket_t *this, packet_t *packet);

	u_int16_t (*get_port)(socket_t *this, bool nat_t);

	socket_family_t (*supported_families)(socket_t *this);

	void (*destroy)(socket_t *this);
};

bool socket_register(plugin_t *plugin, plugin_feature_t *feature,
					 bool reg, void *data);

#endif 
