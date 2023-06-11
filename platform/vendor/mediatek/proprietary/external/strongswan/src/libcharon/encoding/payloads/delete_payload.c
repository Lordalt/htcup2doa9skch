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

#include "delete_payload.h"


typedef struct private_delete_payload_t private_delete_payload_t;

struct private_delete_payload_t {

	delete_payload_t public;

	u_int8_t  next_payload;

	bool critical;

	bool reserved[8];

	u_int16_t payload_length;

	u_int32_t doi;

	u_int8_t protocol_id;

	u_int8_t spi_size;

	u_int16_t spi_count;

	chunk_t spis;

	payload_type_t type;
};

static encoding_rule_t encodings_v2[] = {
	
	{ U_INT_8,			offsetof(private_delete_payload_t, next_payload)	},
	
	{ FLAG,				offsetof(private_delete_payload_t, critical)		},
	
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[0])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[1])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[2])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[3])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[4])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[5])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[6])		},
	
	{ PAYLOAD_LENGTH,	offsetof(private_delete_payload_t, payload_length)	},
	{ U_INT_8,			offsetof(private_delete_payload_t, protocol_id)		},
	{ U_INT_8,			offsetof(private_delete_payload_t, spi_size)		},
	{ U_INT_16,			offsetof(private_delete_payload_t, spi_count)		},
	
	{ CHUNK_DATA,		offsetof(private_delete_payload_t, spis)			},
};


static encoding_rule_t encodings_v1[] = {
	
	{ U_INT_8,			offsetof(private_delete_payload_t, next_payload)	},
	
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[0])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[1])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[2])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[3])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[4])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[5])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[6])		},
	{ RESERVED_BIT,		offsetof(private_delete_payload_t, reserved[7])		},
	
	{ PAYLOAD_LENGTH,	offsetof(private_delete_payload_t, payload_length)	},
	
	{ U_INT_32,			offsetof(private_delete_payload_t, doi)				},
	{ U_INT_8,			offsetof(private_delete_payload_t, protocol_id)		},
	{ U_INT_8,			offsetof(private_delete_payload_t, spi_size)		},
	{ U_INT_16,			offsetof(private_delete_payload_t, spi_count)		},
	
	{ CHUNK_DATA,		offsetof(private_delete_payload_t, spis)			},
};


METHOD(payload_t, verify, status_t,
	private_delete_payload_t *this)
{
	switch (this->protocol_id)
	{
		case PROTO_AH:
		case PROTO_ESP:
			if (this->spi_size != 4)
			{
				return FAILED;
			}
			break;
		case PROTO_IKE:
		case 0:
			if (this->type == DELETE)
			{	
				if (this->spi_size != 0)
				{
					return FAILED;
				}
			}
			else
			{	
				if (this->spi_size != 16)
				{
					return FAILED;
				}
			}
			break;
		default:
			return FAILED;
	}
	if (this->spis.len != (this->spi_count * this->spi_size))
	{
		return FAILED;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_delete_payload_t *this, encoding_rule_t **rules)
{
	if (this->type == DELETE)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_delete_payload_t *this)
{
	if (this->type == DELETE)
	{
		return 8;
	}
	return 12;
}

METHOD(payload_t, get_payload_type, payload_type_t,
	private_delete_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_delete_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_delete_payload_t *this,payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_delete_payload_t *this)
{
	return this->payload_length;
}

METHOD(delete_payload_t, get_protocol_id, protocol_id_t,
	private_delete_payload_t *this)
{
	return this->protocol_id;
}

METHOD(delete_payload_t, add_spi, void,
	private_delete_payload_t *this, u_int32_t spi)
{
	switch (this->protocol_id)
	{
		case PROTO_AH:
		case PROTO_ESP:
			this->spi_count++;
			this->payload_length += sizeof(spi);
			this->spis = chunk_cat("mc", this->spis, chunk_from_thing(spi));
			break;
		default:
			break;
	}
}

METHOD(delete_payload_t, set_ike_spi, void,
	private_delete_payload_t *this, u_int64_t spi_i, u_int64_t spi_r)
{
	free(this->spis.ptr);
	this->spis = chunk_cat("cc", chunk_from_thing(spi_i),
								 chunk_from_thing(spi_r));
	this->spi_count = 1;
	this->payload_length = get_header_length(this) + this->spi_size;
}

typedef struct {
	
	enumerator_t public;
	
	chunk_t spis;
} spi_enumerator_t;

METHOD(enumerator_t, spis_enumerate, bool,
	spi_enumerator_t *this, u_int32_t *spi)
{
	if (this->spis.len >= sizeof(*spi))
	{
		memcpy(spi, this->spis.ptr, sizeof(*spi));
		this->spis = chunk_skip(this->spis, sizeof(*spi));
		return TRUE;
	}
	return FALSE;
}

METHOD(delete_payload_t, create_spi_enumerator, enumerator_t*,
	private_delete_payload_t *this)
{
	spi_enumerator_t *e;

	if (this->spi_size != sizeof(u_int32_t))
	{
		return enumerator_create_empty();
	}
	INIT(e,
		.public = {
			.enumerate = (void*)_spis_enumerate,
			.destroy = (void*)free,
		},
		.spis = this->spis,
	);
	return &e->public;
}

METHOD2(payload_t, delete_payload_t, destroy, void,
	private_delete_payload_t *this)
{
	free(this->spis.ptr);
	free(this);
}

delete_payload_t *delete_payload_create(payload_type_t type,
										protocol_id_t protocol_id)
{
	private_delete_payload_t *this;

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
			.get_protocol_id = _get_protocol_id,
			.add_spi = _add_spi,
			.set_ike_spi = _set_ike_spi,
			.create_spi_enumerator = _create_spi_enumerator,
			.destroy = _destroy,
		},
		.next_payload = NO_PAYLOAD,
		.protocol_id = protocol_id,
		.doi = IKEV1_DOI_IPSEC,
		.type = type,
	);
	this->payload_length = get_header_length(this);

	if (protocol_id == PROTO_IKE)
	{
		if (type == DELETE_V1)
		{
			this->spi_size = 16;
		}
	}
	else
	{
		this->spi_size = 4;
	}
	return &this->public;
}