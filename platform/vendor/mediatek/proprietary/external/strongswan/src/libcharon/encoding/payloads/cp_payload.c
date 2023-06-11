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

#include "cp_payload.h"

#include <encoding/payloads/encodings.h>
#include <collections/linked_list.h>

ENUM(config_type_names, CFG_REQUEST, CFG_ACK,
	"CFG_REQUEST",
	"CFG_REPLY",
	"CFG_SET",
	"CFG_ACK",
);

typedef struct private_cp_payload_t private_cp_payload_t;

struct private_cp_payload_t {

	cp_payload_t public;

	u_int8_t next_payload;

	bool critical;

	bool reserved_bit[7];

	u_int8_t reserved_byte[3];

	u_int16_t payload_length;

	u_int16_t identifier;

	linked_list_t *attributes;

	u_int8_t cfg_type;

	payload_type_t type;
};

static encoding_rule_t encodings_v2[] = {
	
	{ U_INT_8,			offsetof(private_cp_payload_t, next_payload)	},
	
	{ FLAG,				offsetof(private_cp_payload_t, critical)		},
	
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[0])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[1])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[2])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[3])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[4])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[5])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[6])	},
	
	{ PAYLOAD_LENGTH,	offsetof(private_cp_payload_t, payload_length)	},
	{ U_INT_8,			offsetof(private_cp_payload_t, cfg_type)		},
	
	{ RESERVED_BYTE,	offsetof(private_cp_payload_t, reserved_byte[0])},
	{ RESERVED_BYTE,	offsetof(private_cp_payload_t, reserved_byte[1])},
	{ RESERVED_BYTE,	offsetof(private_cp_payload_t, reserved_byte[2])},
	
	{ PAYLOAD_LIST + CONFIGURATION_ATTRIBUTE,
						offsetof(private_cp_payload_t, attributes)		},
};


static encoding_rule_t encodings_v1[] = {
	
	{ U_INT_8,			offsetof(private_cp_payload_t, next_payload)	},
	
	{ FLAG,				offsetof(private_cp_payload_t, critical)		},
	
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[0])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[1])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[2])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[3])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[4])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[5])	},
	{ RESERVED_BIT,		offsetof(private_cp_payload_t, reserved_bit[6])	},
	
	{ PAYLOAD_LENGTH,	offsetof(private_cp_payload_t, payload_length)	},
	{ U_INT_8,			offsetof(private_cp_payload_t, cfg_type)		},
	
	{ RESERVED_BYTE,	offsetof(private_cp_payload_t, reserved_byte[0])},
	{ U_INT_16,			offsetof(private_cp_payload_t, identifier)},
	
	{ PAYLOAD_LIST + CONFIGURATION_ATTRIBUTE_V1,
						offsetof(private_cp_payload_t, attributes)		},
};


METHOD(payload_t, verify, status_t,
	private_cp_payload_t *this)
{
	status_t status = SUCCESS;
	enumerator_t *enumerator;
	payload_t *attribute;

	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &attribute))
	{
		status = attribute->verify(attribute);
		if (status != SUCCESS)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return status;
}

METHOD(payload_t, get_encoding_rules, int,
	private_cp_payload_t *this, encoding_rule_t **rules)
{
	if (this->type == CONFIGURATION)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_cp_payload_t *this)
{
	return 8;
}

METHOD(payload_t, get_type, payload_type_t,
	private_cp_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_cp_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_cp_payload_t *this,payload_type_t type)
{
	this->next_payload = type;
}

static void compute_length(private_cp_payload_t *this)
{
	enumerator_t *enumerator;
	payload_t *attribute;

	this->payload_length = get_header_length(this);

	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &attribute))
	{
		this->payload_length += attribute->get_length(attribute);
	}
	enumerator->destroy(enumerator);
}

METHOD(payload_t, get_length, size_t,
	private_cp_payload_t *this)
{
	return this->payload_length;
}

METHOD(cp_payload_t, create_attribute_enumerator, enumerator_t*,
	private_cp_payload_t *this)
{
	return this->attributes->create_enumerator(this->attributes);
}

METHOD(cp_payload_t, add_attribute, void,
	private_cp_payload_t *this, configuration_attribute_t *attribute)
{
	this->attributes->insert_last(this->attributes, attribute);
	compute_length(this);
}

METHOD(cp_payload_t, get_config_type, config_type_t,
	private_cp_payload_t *this)
{
	return this->cfg_type;
}

METHOD(cp_payload_t, get_identifier, u_int16_t,
			 private_cp_payload_t *this)
{
	return this->identifier;
}
METHOD(cp_payload_t, set_identifier, void,
			 private_cp_payload_t *this, u_int16_t identifier)
{
	this->identifier = identifier;
}

METHOD2(payload_t, cp_payload_t, destroy, void,
	private_cp_payload_t *this)
{
	this->attributes->destroy_offset(this->attributes,
								offsetof(configuration_attribute_t, destroy));
	free(this);
}

cp_payload_t *cp_payload_create_type(payload_type_t type, config_type_t cfg_type)
{
	private_cp_payload_t *this;

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
			.create_attribute_enumerator = _create_attribute_enumerator,
			.add_attribute = _add_attribute,
			.get_type = _get_config_type,
			.get_identifier = _get_identifier,
			.set_identifier = _set_identifier,
			.destroy = _destroy,
		},
		.next_payload = NO_PAYLOAD,
		.payload_length = get_header_length(this),
		.attributes = linked_list_create(),
		.cfg_type = cfg_type,
		.type = type,
	);
	return &this->public;
}

cp_payload_t *cp_payload_create(payload_type_t type)
{
	return cp_payload_create_type(type, CFG_REQUEST);
}
