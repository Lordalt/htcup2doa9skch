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


#ifndef EAP_RADIUS_PLUGIN_H_
#define EAP_RADIUS_PLUGIN_H_

#include <plugins/plugin.h>

#include <radius_client.h>
#include <daemon.h>

typedef struct eap_radius_plugin_t eap_radius_plugin_t;

struct eap_radius_plugin_t {

	plugin_t plugin;
};

radius_client_t *eap_radius_create_client();

void eap_radius_handle_timeout(ike_sa_id_t *id);

#endif 
