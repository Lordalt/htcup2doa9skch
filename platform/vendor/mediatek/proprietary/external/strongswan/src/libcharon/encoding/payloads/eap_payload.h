/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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


#ifndef EAP_PAYLOAD_H_
#define EAP_PAYLOAD_H_

typedef struct eap_payload_t eap_payload_t;

#include <library.h>
#include <eap/eap.h>
#include <encoding/payloads/payload.h>

struct eap_payload_t {

	payload_t payload_interface;

	void (*set_data) (eap_payload_t *this, chunk_t data);

	chunk_t (*get_data) (eap_payload_t *this);

	eap_code_t (*get_code) (eap_payload_t *this);

	u_int8_t (*get_identifier) (eap_payload_t *this);

	eap_type_t (*get_type) (eap_payload_t *this, u_int32_t *vendor);

	enumerator_t* (*get_types) (eap_payload_t *this);

	bool (*is_expanded) (eap_payload_t *this);

	void (*destroy) (eap_payload_t *this);
};

eap_payload_t *eap_payload_create(void);

eap_payload_t *eap_payload_create_data(chunk_t data);

eap_payload_t *eap_payload_create_data_own(chunk_t data);

eap_payload_t *eap_payload_create_code(eap_code_t code, u_int8_t identifier);

eap_payload_t *eap_payload_create_nak(u_int8_t identifier, eap_type_t type,
									  u_int32_t vendor, bool expanded);

#endif 
