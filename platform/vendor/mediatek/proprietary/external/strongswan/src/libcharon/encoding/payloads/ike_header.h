/*
 * Copyright (C) 2007 Tobias Brunner
 * Copyright (C) 2005-2011 Martin Willi
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


#ifndef IKE_HEADER_H_
#define IKE_HEADER_H_

typedef enum exchange_type_t exchange_type_t;
typedef struct ike_header_t ike_header_t;

#include <library.h>
#include <encoding/payloads/payload.h>

#define IKEV1_MAJOR_VERSION 1

#define IKEV1_MINOR_VERSION 0

#define IKEV2_MAJOR_VERSION 2

#define IKEV2_MINOR_VERSION 0

#define IKE_HEADER_LENGTH 28

enum exchange_type_t{

	ID_PROT = 2,

	AUTH_ONLY = 3,

	AGGRESSIVE = 4,

	INFORMATIONAL_V1 = 5,

	TRANSACTION = 6,

	QUICK_MODE = 32,

	NEW_GROUP_MODE = 33,

	IKE_SA_INIT = 34,

	IKE_AUTH = 35,

	CREATE_CHILD_SA = 36,

	INFORMATIONAL = 37,

	IKE_SESSION_RESUME = 38,

#ifdef ME
	ME_CONNECT = 240,
#endif 

	EXCHANGE_TYPE_UNDEFINED = 255,
};

extern enum_name_t *exchange_type_names;

struct ike_header_t {
	payload_t payload_interface;

	u_int64_t (*get_initiator_spi) (ike_header_t *this);

	void (*set_initiator_spi) (ike_header_t *this, u_int64_t initiator_spi);

	u_int64_t (*get_responder_spi) (ike_header_t *this);

	void (*set_responder_spi) (ike_header_t *this, u_int64_t responder_spi);

	u_int8_t (*get_maj_version) (ike_header_t *this);

	void (*set_maj_version) (ike_header_t *this, u_int8_t major);

	u_int8_t (*get_min_version) (ike_header_t *this);

	void (*set_min_version) (ike_header_t *this, u_int8_t minor);

	bool (*get_response_flag) (ike_header_t *this);

	void (*set_response_flag) (ike_header_t *this, bool response);

	bool (*get_version_flag) (ike_header_t *this);

	void (*set_version_flag)(ike_header_t *this, bool version);

	bool (*get_initiator_flag) (ike_header_t *this);

	void (*set_initiator_flag) (ike_header_t *this, bool initiator);

	bool (*get_encryption_flag) (ike_header_t *this);

	void (*set_encryption_flag) (ike_header_t *this, bool encryption);

	bool (*get_commit_flag) (ike_header_t *this);

	void (*set_commit_flag) (ike_header_t *this, bool commit);

	bool (*get_authonly_flag) (ike_header_t *this);

	void (*set_authonly_flag) (ike_header_t *this, bool authonly);

	u_int8_t (*get_exchange_type) (ike_header_t *this);

	void (*set_exchange_type) (ike_header_t *this, u_int8_t exchange_type);

	u_int32_t (*get_message_id) (ike_header_t *this);

	void (*set_message_id) (ike_header_t *this, u_int32_t message_id);

	void (*destroy) (ike_header_t *this);
};

ike_header_t *ike_header_create(void);

ike_header_t *ike_header_create_version(int major, int minor);

#endif 
