/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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


#ifndef XAUTH_EAP_H_
#define XAUTH_EAP_H_

typedef struct xauth_eap_t xauth_eap_t;

#include <sa/xauth/xauth_method.h>

struct xauth_eap_t {

	xauth_method_t xauth_method;
};

xauth_eap_t *xauth_eap_create_server(identification_t *server,
									 identification_t *peer,
									 char *profile);

#endif 
