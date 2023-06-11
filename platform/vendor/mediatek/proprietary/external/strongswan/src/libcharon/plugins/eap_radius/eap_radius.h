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


#ifndef EAP_RADIUS_H_
#define EAP_RADIUS_H_

typedef struct eap_radius_t eap_radius_t;

#include <sa/eap/eap_method.h>
#include <radius_message.h>

struct eap_radius_t {

	eap_method_t eap_method;
};

eap_radius_t *eap_radius_create(identification_t *server, identification_t *peer);

void eap_radius_process_attributes(radius_message_t *message);

void eap_radius_build_attributes(radius_message_t *message);

#endif 
