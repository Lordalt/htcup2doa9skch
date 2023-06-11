/*
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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


#ifndef ENCRYPTION_PAYLOAD_H_
#define ENCRYPTION_PAYLOAD_H_

typedef struct encryption_payload_t encryption_payload_t;

#include <library.h>
#include <crypto/aead.h>
#include <encoding/payloads/payload.h>

struct encryption_payload_t {

	payload_t payload_interface;

	size_t (*get_length)(encryption_payload_t *this);

	void (*add_payload) (encryption_payload_t *this, payload_t *payload);

	payload_t* (*remove_payload)(encryption_payload_t *this);

	void (*set_transform) (encryption_payload_t *this, aead_t *aead);

	status_t (*encrypt) (encryption_payload_t *this, u_int64_t mid,
						 chunk_t assoc);

	status_t (*decrypt) (encryption_payload_t *this, chunk_t assoc);

	void (*destroy) (encryption_payload_t *this);
};

encryption_payload_t *encryption_payload_create(payload_type_t type);

#endif 
