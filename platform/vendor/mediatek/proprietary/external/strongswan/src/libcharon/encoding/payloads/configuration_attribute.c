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

#include "configuration_attribute.h"

#include <utils/cust_settings.h>
#include <encoding/payloads/encodings.h>
#include <library.h>
#include <daemon.h>

typedef struct private_configuration_attribute_t private_configuration_attribute_t;

struct private_configuration_attribute_t {

	configuration_attribute_t public;

	bool af_flag;

	bool reserved;

	u_int16_t attr_type;

	u_int16_t length_or_value;

	chunk_t value;

	payload_type_t type;
};

static encoding_rule_t encodings_v2[] = {
	
	{ RESERVED_BIT,					offsetof(private_configuration_attribute_t, reserved)		},
	
	{ ATTRIBUTE_TYPE,				offsetof(private_configuration_attribute_t, attr_type)		},
	
	{ ATTRIBUTE_LENGTH,				offsetof(private_configuration_attribute_t, length_or_value)},
	
	{ ATTRIBUTE_VALUE,				offsetof(private_configuration_attribute_t, value)			},
};


static encoding_rule_t encodings_v1[] = {
	
	{ ATTRIBUTE_FORMAT,				offsetof(private_configuration_attribute_t, af_flag)		},
	
	{ ATTRIBUTE_TYPE,				offsetof(private_configuration_attribute_t, attr_type)		},
	
	{ ATTRIBUTE_LENGTH_OR_VALUE,	offsetof(private_configuration_attribute_t, length_or_value)},
	
	{ ATTRIBUTE_VALUE,				offsetof(private_configuration_attribute_t, value)			},
};



METHOD(payload_t, verify, status_t,
	private_configuration_attribute_t *this)
{
	bool failed = FALSE;

	switch (this->attr_type)
	{
		case INTERNAL_IP4_ADDRESS:
		case INTERNAL_IP4_NETMASK:
		case INTERNAL_IP4_DNS:
		case INTERNAL_IP4_NBNS:
		case INTERNAL_ADDRESS_EXPIRY:
		case INTERNAL_IP4_DHCP:
		case P_CSCF_IP4_ADDRESS:
			if (this->length_or_value != 0 && this->length_or_value != 4)
			{
				failed = TRUE;
			}
			break;
		case INTERNAL_IP4_SUBNET:
			if (this->length_or_value != 0 && this->length_or_value != 8)
			{
				failed = TRUE;
			}
			break;
		case INTERNAL_IP6_ADDRESS:
		case INTERNAL_IP6_SUBNET:
			if (this->length_or_value != 0 && this->length_or_value != 17)
			{
				failed = TRUE;
			}
			break;
		case INTERNAL_IP6_DNS:
		case INTERNAL_IP6_NBNS:
		case INTERNAL_IP6_DHCP:
			if (this->length_or_value != 0 && this->length_or_value != 16)
			{
				failed = TRUE;
			}
			break;
		case P_CSCF_IP6_ADDRESS:
			if (this->length_or_value != 0 && this->length_or_value != 16 && this->length_or_value != 17)
			{
				failed = TRUE;
			}
			break;
		case SUPPORTED_ATTRIBUTES:
			if (this->length_or_value % 2)
			{
				failed = TRUE;
			}
			break;
		case APPLICATION_VERSION:
		case INTERNAL_IP4_SERVER:
		case INTERNAL_IP6_SERVER:
		case XAUTH_TYPE:
		case XAUTH_USER_NAME:
		case XAUTH_USER_PASSWORD:
		case XAUTH_PASSCODE:
		case XAUTH_MESSAGE:
		case XAUTH_CHALLENGE:
		case XAUTH_DOMAIN:
		case XAUTH_STATUS:
		case XAUTH_NEXT_PIN:
		case XAUTH_ANSWER:
		case UNITY_BANNER:
		case UNITY_SAVE_PASSWD:
		case UNITY_DEF_DOMAIN:
		case UNITY_SPLITDNS_NAME:
		case UNITY_SPLIT_INCLUDE:
		case UNITY_NATT_PORT:
		case UNITY_LOCAL_LAN:
		case UNITY_PFS:
		case UNITY_FW_TYPE:
		case UNITY_BACKUP_SERVERS:
		case UNITY_DDNS_HOSTNAME:
			
			break;
		default:
			if (this->attr_type == get_cust_setting_int(CUST_PCSCF_IP4_VALUE)) {
				if (this->length_or_value != 0 && this->length_or_value != 4) {
					failed = TRUE;
				}
			}
			else if (this->attr_type == get_cust_setting_int(CUST_PCSCF_IP6_VALUE)) {
				if (this->length_or_value != 0 && this->length_or_value != 16 && this->length_or_value != 17) {
					failed = TRUE;
				}
			}
			else {
				DBG1(DBG_ENC, "unknown attribute type %N",
					configuration_attribute_type_names, this->attr_type);
			}
			break;
	}

	if (failed)
	{
		DBG1(DBG_ENC, "invalid attribute length %d for %N",
			 this->length_or_value, configuration_attribute_type_names,
			 this->attr_type);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_configuration_attribute_t *this, encoding_rule_t **rules)
{
	if (this->type == CONFIGURATION_ATTRIBUTE)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_configuration_attribute_t *this)
{
	return 4;
}

METHOD(payload_t, get_type, payload_type_t,
	private_configuration_attribute_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_configuration_attribute_t *this)
{
	return NO_PAYLOAD;
}

METHOD(payload_t, set_next_type, void,
	private_configuration_attribute_t *this, payload_type_t type)
{
}

METHOD(payload_t, get_length, size_t,
	private_configuration_attribute_t *this)
{
	return get_header_length(this) + this->value.len;
}

METHOD(configuration_attribute_t, get_cattr_type, configuration_attribute_type_t,
	private_configuration_attribute_t *this)
{
	return this->attr_type;
}

METHOD(configuration_attribute_t, get_chunk, chunk_t,
	private_configuration_attribute_t *this)
{
	if (this->af_flag)
	{
		return chunk_from_thing(this->length_or_value);
	}
	return this->value;
}

METHOD(configuration_attribute_t, get_value, u_int16_t,
	private_configuration_attribute_t *this)
{
	if (this->af_flag)
	{
		return this->length_or_value;
	}
	return 0;
}

METHOD2(payload_t, configuration_attribute_t, destroy, void,
	private_configuration_attribute_t *this)
{
	free(this->value.ptr);
	free(this);
}

configuration_attribute_t *configuration_attribute_create(payload_type_t type)
{
	private_configuration_attribute_t *this;

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
			.get_chunk = _get_chunk,
			.get_value = _get_value,
			.get_type = _get_cattr_type,
			.destroy = _destroy,
		},
		.type = type
	);
	return &this->public;
}

configuration_attribute_t *configuration_attribute_create_chunk(
	payload_type_t type, configuration_attribute_type_t attr_type, chunk_t chunk)
{
	private_configuration_attribute_t *this;

	this = (private_configuration_attribute_t*)
							configuration_attribute_create(type);
	this->attr_type = ((u_int16_t)attr_type) & 0x7FFF;
	this->value = chunk_clone(chunk);
	this->length_or_value = chunk.len;
	return &this->public;
}

configuration_attribute_t *configuration_attribute_create_value(
					configuration_attribute_type_t attr_type, u_int16_t value)
{
	private_configuration_attribute_t *this;

	this = (private_configuration_attribute_t*)
					configuration_attribute_create(CONFIGURATION_ATTRIBUTE_V1);
	this->attr_type = ((u_int16_t)attr_type) & 0x7FFF;
	this->length_or_value = value;
	this->af_flag = TRUE;

	return &this->public;
}
