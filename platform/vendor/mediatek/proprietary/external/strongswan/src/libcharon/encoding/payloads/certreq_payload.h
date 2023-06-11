/*
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


#ifndef CERTREQ_PAYLOAD_H_
#define CERTREQ_PAYLOAD_H_

typedef struct certreq_payload_t certreq_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/cert_payload.h>
#include <utils/identification.h>

struct certreq_payload_t {

	payload_t payload_interface;

	enumerator_t* (*create_keyid_enumerator)(certreq_payload_t *this);

	certificate_type_t (*get_cert_type)(certreq_payload_t *this);

	void (*add_keyid)(certreq_payload_t *this, chunk_t keyid);

	identification_t* (*get_dn)(certreq_payload_t *this);

	void (*destroy) (certreq_payload_t *this);
};

certreq_payload_t *certreq_payload_create(payload_type_t payload_type);

certreq_payload_t *certreq_payload_create_type(certificate_type_t type);

certreq_payload_t *certreq_payload_create_dn(identification_t *id);

#endif 
