/*
 * Copyright (C) 2010-2012 Andreas Steffen
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


#ifndef EAP_TNC_H_
#define EAP_TNC_H_

typedef struct eap_tnc_t eap_tnc_t;

#include <sa/eap/eap_inner_method.h>

struct eap_tnc_t {

	eap_inner_method_t eap_inner_method;
};

eap_tnc_t *eap_tnc_create_server(identification_t *server, identification_t *peer);

eap_tnc_t *eap_tnc_create_peer(identification_t *server, identification_t *peer);

#endif 
