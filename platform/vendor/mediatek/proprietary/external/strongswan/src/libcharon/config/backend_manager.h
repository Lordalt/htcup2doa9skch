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


#ifndef BACKEND_MANAGER_H_
#define BACKEND_MANAGER_H_

typedef struct backend_manager_t backend_manager_t;

#include <library.h>
#include <networking/host.h>
#include <utils/identification.h>
#include <config/ike_cfg.h>
#include <config/peer_cfg.h>
#include <config/backend.h>


struct backend_manager_t {

	ike_cfg_t* (*get_ike_cfg)(backend_manager_t *this,
							  host_t *my_host, host_t *other_host,
							  ike_version_t version);

	peer_cfg_t* (*get_peer_cfg_by_name)(backend_manager_t *this, char *name);

	enumerator_t* (*create_peer_cfg_enumerator)(backend_manager_t *this,
							host_t *me, host_t *other, identification_t *my_id,
							identification_t *other_id, ike_version_t version);
	void (*add_backend)(backend_manager_t *this, backend_t *backend);

	void (*remove_backend)(backend_manager_t *this, backend_t *backend);

	void (*destroy) (backend_manager_t *this);
};

backend_manager_t* backend_manager_create(void);

#endif 
