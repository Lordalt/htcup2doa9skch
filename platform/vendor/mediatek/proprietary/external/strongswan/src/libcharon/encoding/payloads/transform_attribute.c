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

#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include "transform_attribute.h"

#include <encoding/payloads/encodings.h>
#include <library.h>

ENUM(tattr_ph1_names, TATTR_PH1_ENCRYPTION_ALGORITHM, TATTR_PH1_GROUP_ORDER,
	"ENCRYPTION_ALGORITHM",
	"HASH_ALGORITHM",
	"AUTH_METHOD",
	"GROUP",
	"GROUP_TYPE",
	"GROUP_PRIME",
	"GROUP_GENONE",
	"GROUP_GENTWO",
	"GROUP_CURVE_A",
	"GROUP_CURVE_B",
	"LIFE_TYPE",
	"LIFE_DURATION",
	"PRF",
	"KEY_LENGTH",
	"FIELD_SIZE",
	"GROUP_ORDER",
);

ENUM(tattr_ph2_names, TATTR_PH2_SA_LIFE_TYPE, TATTR_PH2_EXT_SEQ_NUMBER,
	"SA_LIFE_TYPE",
	"SA_LIFE_DURATION",
	"GROUP",
	"ENCAP_MODE",
	"AUTH_ALGORITHM",
	"KEY_LENGTH",
	"KEY_ROUNDS",
	"COMP_DICT_SIZE",
	"COMP_PRIV_ALGORITHM",
	"ECN_TUNNEL",
	"EXT_SEQ_NUMBER",
);

ENUM(tattr_ikev2_names, TATTR_IKEV2_KEY_LENGTH, TATTR_IKEV2_KEY_LENGTH,
	"KEY_LENGTH",
);


typedef struct private_transform_attribute_t private_transform_attribute_t;

struct private_transform_attribute_t {

	transform_attribute_t public;

	bool attribute_format;

	u_int16_t attribute_type;

	u_int16_t attribute_length_or_value;

	chunk_t attribute_value;

	payload_type_t type;
};

static encoding_rule_t encodings[] = {
	
	{ ATTRIBUTE_FORMAT,			offsetof(private_transform_attribute_t, attribute_format)			},
	
	{ ATTRIBUTE_TYPE,			offsetof(private_transform_attribute_t, attribute_type)				},
	
	{ ATTRIBUTE_LENGTH_OR_VALUE,offsetof(private_transform_attribute_t, attribute_length_or_value)	},
	
	{ ATTRIBUTE_VALUE,			offsetof(private_transform_attribute_t, attribute_value)			}
};


METHOD(payload_t, verify, status_t,
	private_transform_attribute_t *this)
{
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_transform_attribute_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_transform_attribute_t *this)
{
	return 0;
}

METHOD(payload_t, get_type, payload_type_t,
	private_transform_attribute_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_transform_attribute_t *this)
{
	return NO_PAYLOAD;
}

METHOD(payload_t, set_next_type, void,
	private_transform_attribute_t *this, payload_type_t type)
{
}

METHOD(payload_t, get_length, size_t,
	private_transform_attribute_t *this)
{
	if (this->attribute_format)
	{
		return 4;
	}
	return this->attribute_length_or_value + 4;
}

METHOD(transform_attribute_t, get_value_chunk, chunk_t,
	private_transform_attribute_t *this)
{
	if (this->attribute_format)
	{
		return chunk_from_thing(this->attribute_length_or_value);
	}
	return this->attribute_value;
}

METHOD(transform_attribute_t, get_value, u_int64_t,
	private_transform_attribute_t *this)
{
	u_int64_t value = 0;

	if (this->attribute_format)
	{
		return this->attribute_length_or_value;
	}
	if (this->attribute_value.len > sizeof(value))
	{
		return UINT64_MAX;
	}
	memcpy(((char*)&value) + sizeof(value) - this->attribute_value.len,
		   this->attribute_value.ptr, this->attribute_value.len);
	return untoh64((char*)&value);
}

METHOD(transform_attribute_t, get_attribute_type, u_int16_t,
	private_transform_attribute_t *this)
{
	return this->attribute_type;
}

METHOD2(payload_t, transform_attribute_t, destroy, void,
	private_transform_attribute_t *this)
{
	free(this->attribute_value.ptr);
	free(this);
}

transform_attribute_t *transform_attribute_create(payload_type_t type)
{
	private_transform_attribute_t *this;

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
			.get_value_chunk = _get_value_chunk,
			.get_value = _get_value,
			.get_attribute_type = _get_attribute_type,
			.destroy = _destroy,
		},
		.attribute_format = FALSE,
		.type = type,
	);
	return &this->public;
}

transform_attribute_t *transform_attribute_create_value(payload_type_t type,
							transform_attribute_type_t kind, u_int64_t value)
{
	private_transform_attribute_t *this;

	this = (private_transform_attribute_t*)transform_attribute_create(type);

	this->attribute_type = kind & 0x7FFF;

	if (value <= UINT16_MAX)
	{
		this->attribute_length_or_value = value;
		this->attribute_format = TRUE;
	}
	else if (value <= UINT32_MAX)
	{
		u_int32_t val32;

		val32 = htonl(value);
		this->attribute_value = chunk_clone(chunk_from_thing(val32));
		this->attribute_length_or_value = sizeof(val32);
	}
	else
	{
		htoun64(&value, value);
		this->attribute_value = chunk_clone(chunk_from_thing(value));
		this->attribute_length_or_value = sizeof(value);
	}
	return &this->public;
}
