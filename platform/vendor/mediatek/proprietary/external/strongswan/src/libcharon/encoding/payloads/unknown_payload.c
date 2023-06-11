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

#include <stddef.h>

#include "unknown_payload.h"

typedef struct private_unknown_payload_t private_unknown_payload_t;

struct private_unknown_payload_t {

	unknown_payload_t public;

	payload_type_t type;

	u_int8_t next_payload;

	bool critical;

	bool reserved[7];

	u_int16_t payload_length;

	chunk_t data;
};

static encoding_rule_t encodings[] = {
	
	{ U_INT_8,			offsetof(private_unknown_payload_t, next_payload)	},
	
	{ FLAG,				offsetof(private_unknown_payload_t, critical)		},
	
	{ RESERVED_BIT,		offsetof(private_unknown_payload_t, reserved[0])	},
	{ RESERVED_BIT,		offsetof(private_unknown_payload_t, reserved[1])	},
	{ RESERVED_BIT,		offsetof(private_unknown_payload_t, reserved[2])	},
	{ RESERVED_BIT,		offsetof(private_unknown_payload_t, reserved[3])	},
	{ RESERVED_BIT,		offsetof(private_unknown_payload_t, reserved[4])	},
	{ RESERVED_BIT,		offsetof(private_unknown_payload_t, reserved[5])	},
	{ RESERVED_BIT,		offsetof(private_unknown_payload_t, reserved[6])	},
	
	{ PAYLOAD_LENGTH,	offsetof(private_unknown_payload_t, payload_length)	},
	
	{ CHUNK_DATA,		offsetof(private_unknown_payload_t, data)			},
};


METHOD(payload_t, verify, status_t,
	private_unknown_payload_t *this)
{
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_unknown_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_unknown_payload_t *this)
{
	return 4;
}

METHOD(payload_t, get_payload_type, payload_type_t,
	private_unknown_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_unknown_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_unknown_payload_t *this,payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_unknown_payload_t *this)
{
	return this->payload_length;
}

METHOD(unknown_payload_t, is_critical, bool,
	private_unknown_payload_t *this)
{
	return this->critical;
}

METHOD(unknown_payload_t, get_data, chunk_t,
	private_unknown_payload_t *this)
{
	return this->data;
}

METHOD2(payload_t, unknown_payload_t, destroy, void,
	private_unknown_payload_t *this)
{
	free(this->data.ptr);
	free(this);
}

unknown_payload_t *unknown_payload_create(payload_type_t type)
{
	private_unknown_payload_t *this;

	INIT(this,
		.public = {
			.payload_interface = {
				.verify = _verify,
				.get_encoding_rules = _get_encoding_rules,
				.get_header_length = _get_header_length,
				.get_length = _get_length,
				.get_next_type = _get_next_type,
				.set_next_type = _set_next_type,
				.get_type = _get_payload_type,
				.destroy = _destroy,
			},
			.is_critical = _is_critical,
			.get_data = _get_data,
			.destroy = _destroy,
		},
		.next_payload = NO_PAYLOAD,
		.payload_length = get_header_length(this),
		.type = type,
	);

	return &this->public;
}


unknown_payload_t *unknown_payload_create_data(payload_type_t type,
											   bool critical, chunk_t data)
{
	private_unknown_payload_t *this;

	this = (private_unknown_payload_t*)unknown_payload_create(type);
	this->data = data;
	this->critical = critical;
	this->payload_length = get_header_length(this) + data.len;

	return &this->public;
}
