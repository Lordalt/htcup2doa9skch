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


#ifndef EAP_RADIUS_XAUTH_H_
#define EAP_RADIUS_XAUTH_H_

#include <sa/xauth/xauth_method.h>

typedef struct eap_radius_xauth_t eap_radius_xauth_t;

struct eap_radius_xauth_t {

	xauth_method_t xauth_method;
};

eap_radius_xauth_t *eap_radius_xauth_create_server(identification_t *server,
												   identification_t *peer,
												   char *profile);

#endif 
