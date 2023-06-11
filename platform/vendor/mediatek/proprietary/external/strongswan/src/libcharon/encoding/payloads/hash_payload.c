/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include <stddef.h>

#include "hash_payload.h"

#include <encoding/payloads/encodings.h>

typedef struct private_hash_payload_t private_hash_payload_t;

struct private_hash_payload_t {

	hash_payload_t public;

	u_int8_t next_payload;

	u_int8_t reserved;

	u_int16_t payload_length;

	chunk_t hash;

	payload_type_t type;
};

static encoding_rule_t encodings[] = {
	
	{ U_INT_8,			offsetof(private_hash_payload_t, next_payload)		},
	{ RESERVED_BYTE,	offsetof(private_hash_payload_t, reserved)			},
	
	{ PAYLOAD_LENGTH,	offsetof(private_hash_payload_t, payload_length)	},
	
	{ CHUNK_DATA,		offsetof(private_hash_payload_t, hash)				},
};


METHOD(payload_t, verify, status_t,
	private_hash_payload_t *this)
{
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_hash_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_hash_payload_t *this)
{
	return 4;
}

METHOD(payload_t, get_type, payload_type_t,
	private_hash_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_hash_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_hash_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_hash_payload_t *this)
{
	return this->payload_length;
}

METHOD(hash_payload_t, set_hash, void,
	 private_hash_payload_t *this, chunk_t hash)
{
	free(this->hash.ptr);
	this->hash = chunk_clone(hash);
	this->payload_length = get_header_length(this) + hash.len;
}

METHOD(hash_payload_t, get_hash, chunk_t,
	private_hash_payload_t *this)
{
	return this->hash;
}

METHOD2(payload_t, hash_payload_t, destroy, void,
	private_hash_payload_t *this)
{
	free(this->hash.ptr);
	free(this);
}

hash_payload_t *hash_payload_create(payload_type_t type)
{
	private_hash_payload_t *this;

	INIT(this,
		.public = {
			.payload_interface = {
				.verify = _verify,
				.get_encoding_rules = _get_encoding_rules,
				.get_header_length = _get_header_length,
				.get_length = _get_length,
				.get_next_type = _get_next_type,
				.set_next_type = _set_next_type,
				.get_type = _get_type,
				.destroy = _destroy,
			},
			.set_hash = _set_hash,
			.get_hash = _get_hash,
			.destroy = _destroy,
		},
		.next_payload = NO_PAYLOAD,
		.payload_length = get_header_length(this),
		.type = type,
	);
	return &this->public;
}
