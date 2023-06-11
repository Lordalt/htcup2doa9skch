/*
 * Copyright (C) 2009 Tobias Brunner
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


#ifndef EAP_MSCHAPV2_H_
#define EAP_MSCHAPV2_H_

typedef struct eap_mschapv2_t eap_mschapv2_t;

#include <sa/eap/eap_method.h>

struct eap_mschapv2_t {

	eap_method_t eap_method_interface;
};

eap_mschapv2_t *eap_mschapv2_create_server(identification_t *server, identification_t *peer);

eap_mschapv2_t *eap_mschapv2_create_peer(identification_t *server, identification_t *peer);

#endif 
