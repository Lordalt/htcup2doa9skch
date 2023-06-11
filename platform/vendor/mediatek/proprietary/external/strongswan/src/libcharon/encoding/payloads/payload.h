/*
 * Copyright (C) 2007 Tobias Brunner
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


#ifndef PAYLOAD_H_
#define PAYLOAD_H_

typedef enum payload_type_t payload_type_t;
typedef struct payload_t payload_t;

#include <library.h>
#include <encoding/payloads/encodings.h>

#define IKEV1_DOI_IPSEC 1

enum payload_type_t {

	NO_PAYLOAD = 0,

	SECURITY_ASSOCIATION_V1 = 1,

	PROPOSAL_V1 = 2,

	TRANSFORM_V1 = 3,

	KEY_EXCHANGE_V1 = 4,

	ID_V1 = 5,

	CERTIFICATE_V1 = 6,

	CERTIFICATE_REQUEST_V1 = 7,

	HASH_V1 = 8,

	SIGNATURE_V1 = 9,

	NONCE_V1 = 10,

	NOTIFY_V1 = 11,

	DELETE_V1 = 12,

	VENDOR_ID_V1 = 13,

	CONFIGURATION_V1 = 14,

	NAT_D_V1 = 20,

	NAT_OA_V1 = 21,

	SECURITY_ASSOCIATION = 33,

	KEY_EXCHANGE = 34,

	ID_INITIATOR = 35,

	ID_RESPONDER = 36,

	CERTIFICATE = 37,

	CERTIFICATE_REQUEST = 38,

	AUTHENTICATION = 39,

	NONCE = 40,

	NOTIFY = 41,

	DELETE = 42,

	VENDOR_ID = 43,

	TRAFFIC_SELECTOR_INITIATOR = 44,

	TRAFFIC_SELECTOR_RESPONDER = 45,

	ENCRYPTED = 46,

	CONFIGURATION = 47,

	EXTENSIBLE_AUTHENTICATION = 48,

	GENERIC_SECURE_PASSWORD_METHOD = 49,

#ifdef ME
	ID_PEER = 128,
#endif 

	NAT_D_DRAFT_00_03_V1 = 130,

	NAT_OA_DRAFT_00_03_V1 = 131,

	FRAGMENT_V1 = 132,

	HEADER = 256,

	PROPOSAL_SUBSTRUCTURE,

	PROPOSAL_SUBSTRUCTURE_V1,

	TRANSFORM_SUBSTRUCTURE,

	TRANSFORM_SUBSTRUCTURE_V1,

	TRANSFORM_ATTRIBUTE,

	TRANSFORM_ATTRIBUTE_V1,

	TRAFFIC_SELECTOR_SUBSTRUCTURE,

	CONFIGURATION_ATTRIBUTE,

	CONFIGURATION_ATTRIBUTE_V1,

	ENCRYPTED_V1,
};

extern enum_name_t *payload_type_names;

extern enum_name_t *payload_type_short_names;

struct payload_t {

	int (*get_encoding_rules) (payload_t *this, encoding_rule_t **rules);

	int (*get_header_length)(payload_t *this);

	payload_type_t (*get_type) (payload_t *this);

	payload_type_t (*get_next_type) (payload_t *this);

	void (*set_next_type) (payload_t *this,payload_type_t type);

	size_t (*get_length) (payload_t *this);

	status_t (*verify) (payload_t *this);

	void (*destroy) (payload_t *this);
};

payload_t *payload_create(payload_type_t type);

bool payload_is_known(payload_type_t type);

void* payload_get_field(payload_t *payload, encoding_type_t type, u_int skip);

#endif 
