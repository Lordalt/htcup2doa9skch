/*
 * Copyright (C) 2008-2009 Martin Willi
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


#ifndef EAP_AKA_PEER_H_
#define EAP_AKA_PEER_H_

typedef struct eap_aka_peer_t eap_aka_peer_t;

#include <sa/eap/eap_method.h>

struct eap_aka_peer_t {

	eap_method_t interface;
};

eap_aka_peer_t *eap_aka_peer_create(identification_t *server,
									identification_t *peer);

#endif 
