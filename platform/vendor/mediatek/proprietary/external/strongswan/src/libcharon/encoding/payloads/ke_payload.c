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

#include "ke_payload.h"

#include <encoding/payloads/encodings.h>

typedef struct private_ke_payload_t private_ke_payload_t;

struct private_ke_payload_t {

	ke_payload_t public;

	u_int8_t  next_payload;

	bool critical;

	bool reserved_bit[7];

	u_int8_t reserved_byte[2];

	u_int16_t payload_length;

	u_int16_t dh_group_number;

	chunk_t key_exchange_data;

	payload_type_t type;
};

static encoding_rule_t encodings_v2[] = {
	
	{ U_INT_8,				offsetof(private_ke_payload_t, next_payload)	},
	
	{ FLAG,					offsetof(private_ke_payload_t, critical)		},
	
	{ RESERVED_BIT,			offsetof(private_ke_payload_t, reserved_bit[0])	},
	{ RESERVED_BIT,			offsetof(private_ke_payload_t, reserved_bit[1])	},
	{ RESERVED_BIT,			offsetof(private_ke_payload_t, reserved_bit[2])	},
	{ RESERVED_BIT,			offsetof(private_ke_payload_t, reserved_bit[3])	},
	{ RESERVED_BIT,			offsetof(private_ke_payload_t, reserved_bit[4])	},
	{ RESERVED_BIT,			offsetof(private_ke_payload_t, reserved_bit[5])	},
	{ RESERVED_BIT,			offsetof(private_ke_payload_t, reserved_bit[6])	},
	
	{ PAYLOAD_LENGTH,		offsetof(private_ke_payload_t, payload_length)	},
	
	{ U_INT_16,				offsetof(private_ke_payload_t, dh_group_number)	},
	
	{ RESERVED_BYTE,		offsetof(private_ke_payload_t, reserved_byte[0])},
	{ RESERVED_BYTE,		offsetof(private_ke_payload_t, reserved_byte[1])},
	
	{ CHUNK_DATA,			offsetof(private_ke_payload_t, key_exchange_data)},
};


static encoding_rule_t encodings_v1[] = {
	
	{ U_INT_8,				offsetof(private_ke_payload_t, next_payload) 	},
	
	{ RESERVED_BYTE,		offsetof(private_ke_payload_t, reserved_byte[0])},
	
	{ PAYLOAD_LENGTH,		offsetof(private_ke_payload_t, payload_length)	},
	
	{ CHUNK_DATA,			offsetof(private_ke_payload_t, key_exchange_data)},
};



METHOD(payload_t, verify, status_t,
	private_ke_payload_t *this)
{
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_ke_payload_t *this, encoding_rule_t **rules)
{
	if (this->type == KEY_EXCHANGE)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_ke_payload_t *this)
{
	if (this->type == KEY_EXCHANGE)
	{
		return 8;
	}
	return 4;
}

METHOD(payload_t, get_type, payload_type_t,
	private_ke_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_ke_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_ke_payload_t *this,payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_ke_payload_t *this)
{
	return this->payload_length;
}

METHOD(ke_payload_t, get_key_exchange_data, chunk_t,
	private_ke_payload_t *this)
{
	return this->key_exchange_data;
}

METHOD(ke_payload_t, get_dh_group_number, diffie_hellman_group_t,
	private_ke_payload_t *this)
{
	return this->dh_group_number;
}

METHOD2(payload_t, ke_payload_t, destroy, void,
	private_ke_payload_t *this)
{
	free(this->key_exchange_data.ptr);
	free(this);
}

ke_payload_t *ke_payload_create(payload_type_t type)
{
	private_ke_payload_t *this;

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
			.get_key_exchange_data = _get_key_exchange_data,
			.get_dh_group_number = _get_dh_group_number,
			.destroy = _destroy,
		},
		.next_payload = NO_PAYLOAD,
		.dh_group_number = MODP_NONE,
		.type = type,
	);
	this->payload_length = get_header_length(this);
	return &this->public;
}

ke_payload_t *ke_payload_create_from_diffie_hellman(payload_type_t type,
													diffie_hellman_t *dh)
{
	private_ke_payload_t *this = (private_ke_payload_t*)ke_payload_create(type);

	dh->get_my_public_value(dh, &this->key_exchange_data);
	this->dh_group_number = dh->get_dh_group(dh);
	this->payload_length += this->key_exchange_data.len;

	return &this->public;
}
