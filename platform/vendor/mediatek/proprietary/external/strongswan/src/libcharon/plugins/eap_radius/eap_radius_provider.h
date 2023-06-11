/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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


#ifndef EAP_RADIUS_PROVIDER_H_
#define EAP_RADIUS_PROVIDER_H_

#include <attributes/attributes.h>
#include <attributes/attribute_provider.h>

typedef struct eap_radius_provider_t eap_radius_provider_t;

struct eap_radius_provider_t {

	attribute_provider_t provider;

	void (*add_framed_ip)(eap_radius_provider_t *this, u_int32_t id,
						  host_t *ip);

	void (*add_attribute)(eap_radius_provider_t *this, u_int32_t id,
						  configuration_attribute_type_t type, chunk_t data);

	void (*destroy)(eap_radius_provider_t *this);
};

eap_radius_provider_t *eap_radius_provider_create();

eap_radius_provider_t *eap_radius_provider_get();

#endif 
