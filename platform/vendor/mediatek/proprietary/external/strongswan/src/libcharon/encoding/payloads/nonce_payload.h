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


#ifndef NONCE_PAYLOAD_H_
#define NONCE_PAYLOAD_H_

typedef struct nonce_payload_t nonce_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

#define NONCE_SIZE 32

struct nonce_payload_t {
	payload_t payload_interface;

	void (*set_nonce) (nonce_payload_t *this, chunk_t nonce);

	chunk_t (*get_nonce) (nonce_payload_t *this);

	void (*destroy) (nonce_payload_t *this);
};

nonce_payload_t *nonce_payload_create(payload_type_t type);

#endif 
