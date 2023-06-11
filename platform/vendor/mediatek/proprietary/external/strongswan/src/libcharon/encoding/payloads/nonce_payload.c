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

#include <stddef.h>

#include "nonce_payload.h"

#include <daemon.h>
#include <encoding/payloads/encodings.h>

typedef struct private_nonce_payload_t private_nonce_payload_t;

struct private_nonce_payload_t {

	nonce_payload_t public;

	u_int8_t  next_payload;

	bool critical;

	bool reserved[7];

	u_int16_t payload_length;

	chunk_t nonce;

	payload_type_t type;
};

static encoding_rule_t encodings[] = {
	
	{ U_INT_8,			offsetof(private_nonce_payload_t, next_payload)		},
	
	{ FLAG,				offsetof(private_nonce_payload_t, critical)			},
	
	{ RESERVED_BIT,		offsetof(private_nonce_payload_t, reserved[0])		},
	{ RESERVED_BIT,		offsetof(private_nonce_payload_t, reserved[1])		},
	{ RESERVED_BIT,		offsetof(private_nonce_payload_t, reserved[2])		},
	{ RESERVED_BIT,		offsetof(private_nonce_payload_t, reserved[3])		},
	{ RESERVED_BIT,		offsetof(private_nonce_payload_t, reserved[4])		},
	{ RESERVED_BIT,		offsetof(private_nonce_payload_t, reserved[5])		},
	{ RESERVED_BIT,		offsetof(private_nonce_payload_t, reserved[6])		},
	
	{ PAYLOAD_LENGTH,	offsetof(private_nonce_payload_t, payload_length)	},
	
	{ CHUNK_DATA,		offsetof(private_nonce_payload_t, nonce)			},
};


METHOD(payload_t, verify, status_t,
	private_nonce_payload_t *this)
{
	bool bad_length = FALSE;

	if (this->nonce.len > 256)
	{
		bad_length = TRUE;
	}
	if (this->type == NONCE &&
		this->nonce.len < 16)
	{
		bad_length = TRUE;
	}
	if (this->type == NONCE_V1 &&
		this->nonce.len < 8)
	{
		bad_length = TRUE;
	}
	if (bad_length)
	{
		DBG1(DBG_ENC, "%N payload has invalid length (%d bytes)",
			 payload_type_names, this->type, this->nonce.len);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_nonce_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_nonce_payload_t *this)
{
	return 4;
}

METHOD(payload_t, get_type, payload_type_t,
	private_nonce_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_nonce_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_nonce_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_nonce_payload_t *this)
{
	return this->payload_length;
}

METHOD(nonce_payload_t, set_nonce, void,
	 private_nonce_payload_t *this, chunk_t nonce)
{
	this->nonce = chunk_clone(nonce);
	this->payload_length = get_header_length(this) + nonce.len;
}

METHOD(nonce_payload_t, get_nonce, chunk_t,
	private_nonce_payload_t *this)
{
	return chunk_clone(this->nonce);
}

METHOD2(payload_t, nonce_payload_t, destroy, void,
	private_nonce_payload_t *this)
{
	free(this->nonce.ptr);
	free(this);
}

nonce_payload_t *nonce_payload_create(payload_type_t type)
{
	private_nonce_payload_t *this;

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
			.set_nonce = _set_nonce,
			.get_nonce = _get_nonce,
			.destroy = _destroy,
		},
		.next_payload = NO_PAYLOAD,
		.payload_length = get_header_length(this),
		.type = type,
	);
	return &this->public;
}
