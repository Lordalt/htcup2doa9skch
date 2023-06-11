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

#ifndef IFACE_H
#define IFACE_H

#include <library.h>
#include <collections/enumerator.h>
#include <networking/host.h>

#define TAP_DEVICE "/dev/net/tun"

typedef struct iface_t iface_t;

#include "mconsole.h"
#include "bridge.h"
#include "guest.h"

struct iface_t {

	char* (*get_guestif)(iface_t *this);

	char* (*get_hostif)(iface_t *this);

	bool (*add_address)(iface_t *this, host_t *addr, int bits);

	enumerator_t* (*create_address_enumerator)(iface_t *this);

	bool (*delete_address)(iface_t *this, host_t *addr, int bits);

	void (*set_bridge)(iface_t *this, bridge_t *bridge);

	bridge_t* (*get_bridge)(iface_t *this);

	guest_t* (*get_guest)(iface_t *this);

	void (*destroy) (iface_t *this);
};

iface_t *iface_create(char *name, guest_t *guest, mconsole_t *mconsole);

#endif 

