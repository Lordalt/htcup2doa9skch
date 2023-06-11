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


#ifndef AUTH_PAYLOAD_H_
#define AUTH_PAYLOAD_H_

typedef struct auth_payload_t auth_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <sa/authenticator.h>

struct auth_payload_t {

	payload_t payload_interface;

	void (*set_auth_method) (auth_payload_t *this, auth_method_t method);

	auth_method_t (*get_auth_method) (auth_payload_t *this);

	void (*set_data) (auth_payload_t *this, chunk_t data);

	chunk_t (*get_data) (auth_payload_t *this);

	bool (*get_reserved_bit)(auth_payload_t *this, u_int nr);

	void (*set_reserved_bit)(auth_payload_t *this, u_int nr);

	void (*destroy) (auth_payload_t *this);
};

auth_payload_t *auth_payload_create(void);

#endif 
