/*
 * Copyright (C) 2007 Tobias Brunner
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

#include "endpoint_notify.h"

#include <math.h>

#include <daemon.h>

typedef struct private_endpoint_notify_t private_endpoint_notify_t;

struct private_endpoint_notify_t {
	endpoint_notify_t public;

	u_int32_t priority;

	me_endpoint_family_t family;

	me_endpoint_type_t type;

	host_t *endpoint;

	host_t *base;
};


ENUM(me_endpoint_type_names, HOST, RELAYED,
	"HOST",
	"PEER_REFLEXIVE",
	"SERVER_REFLEXIVE",
	"RELAYED"
);

static private_endpoint_notify_t *endpoint_notify_create();

static status_t parse_uint8(u_int8_t **cur, u_int8_t *top, u_int8_t *val)
{
	if (*cur + sizeof(u_int8_t) > top)
	{
		return FAILED;
	}
	*val =  *(u_int8_t*)*cur;
	*cur += sizeof(u_int8_t);
	return SUCCESS;
}

static status_t parse_uint16(u_int8_t **cur, u_int8_t *top, u_int16_t *val)
{
	if (*cur + sizeof(u_int16_t) > top)
	{
		return FAILED;
	}
	*val =  ntohs(*(u_int16_t*)*cur);
	*cur += sizeof(u_int16_t);
	return SUCCESS;
}

static status_t parse_uint32(u_int8_t **cur, u_int8_t *top, u_int32_t *val)
{
	if (*cur + sizeof(u_int32_t) > top)
	{
		return FAILED;
	}
	*val =  ntohl(*(u_int32_t*)*cur);
	*cur += sizeof(u_int32_t);
	return SUCCESS;
}

static status_t parse_notification_data(private_endpoint_notify_t *this, chunk_t data)
{
	u_int8_t family, type, addr_family;
	u_int16_t port;
	chunk_t addr;
	u_int8_t *cur = data.ptr;
	u_int8_t *top = data.ptr + data.len;

	DBG3(DBG_IKE, "me_endpoint_data %B", &data);

	if (parse_uint32(&cur, top, &this->priority) != SUCCESS)
	{
		DBG1(DBG_IKE, "failed to parse ME_ENDPOINT: invalid priority");
		return FAILED;
	}

	if (parse_uint8(&cur, top, &family) != SUCCESS || family >= MAX_FAMILY)
	{
		DBG1(DBG_IKE, "failed to parse ME_ENDPOINT: invalid family");
		return FAILED;
	}
	this->family = (me_endpoint_family_t)family;

	if (parse_uint8(&cur, top, &type) != SUCCESS ||
		type == NO_TYPE || type >= MAX_TYPE)
	{
		DBG1(DBG_IKE, "failed to parse ME_ENDPOINT: invalid type");
		return FAILED;
	}
	this->type = (me_endpoint_type_t)type;

	addr_family = AF_INET;
	addr.len = 4;

	switch(this->family)
	{
		case IPv6:
			addr_family = AF_INET6;
			addr.len = 16;
			
		case IPv4:
			if (parse_uint16(&cur, top, &port) != SUCCESS)
			{
				DBG1(DBG_IKE, "failed to parse ME_ENDPOINT: invalid port");
				return FAILED;
			}

			if (cur + addr.len > top)
			{
				DBG1(DBG_IKE, "failed to parse ME_ENDPOINT: invalid IP address");
				return FAILED;
			}

			addr.ptr = cur;
			this->endpoint = host_create_from_chunk(addr_family, addr, port);
			break;
		case NO_FAMILY:
		default:
			this->endpoint = NULL;
			break;
	}
	return SUCCESS;
}


static chunk_t build_notification_data(private_endpoint_notify_t *this)
{
	chunk_t prio_chunk, family_chunk, type_chunk, port_chunk, addr_chunk;
	chunk_t data;
	u_int32_t prio;
	u_int16_t port;
	u_int8_t family, type;

	prio = htonl(this->priority);
	prio_chunk = chunk_from_thing(prio);
	family = this->family;
	family_chunk = chunk_from_thing(family);
	type = this->type;
	type_chunk = chunk_from_thing(type);

	if (this->endpoint)
	{
		port = htons(this->endpoint->get_port(this->endpoint));
		addr_chunk = this->endpoint->get_address(this->endpoint);
	}
	else
	{
		port = 0;
		addr_chunk = chunk_empty;
	}
	port_chunk = chunk_from_thing(port);

	
	data = chunk_cat("ccccc", prio_chunk, family_chunk, type_chunk,
					 port_chunk, addr_chunk);
	DBG3(DBG_IKE, "me_endpoint_data %B", &data);
	return data;
}

METHOD(endpoint_notify_t, build_notify, notify_payload_t*,
	private_endpoint_notify_t *this)
{
	chunk_t data;
	notify_payload_t *notify;

	notify = notify_payload_create(NOTIFY);
	notify->set_notify_type(notify, ME_ENDPOINT);
	data = build_notification_data(this);
	notify->set_notification_data(notify, data);
	chunk_free(&data);

	return notify;
}


METHOD(endpoint_notify_t, get_priority, u_int32_t,
	private_endpoint_notify_t *this)
{
	return this->priority;
}

METHOD(endpoint_notify_t, set_priority, void,
	private_endpoint_notify_t *this, u_int32_t priority)
{
	this->priority = priority;
}

METHOD(endpoint_notify_t, get_type,  me_endpoint_type_t,
	private_endpoint_notify_t *this)
{
	return this->type;
}

METHOD(endpoint_notify_t, get_family, me_endpoint_family_t,
	private_endpoint_notify_t *this)
{
	return this->family;
}

METHOD(endpoint_notify_t, get_host, host_t*,
	private_endpoint_notify_t *this)
{
	return this->endpoint;
}

METHOD(endpoint_notify_t, get_base, host_t*,
	private_endpoint_notify_t *this)
{
	return (!this->base) ? this->endpoint : this->base;
}

METHOD(endpoint_notify_t, clone_, endpoint_notify_t*,
	private_endpoint_notify_t *this)
{
	private_endpoint_notify_t *clone;

	clone = endpoint_notify_create();
	clone->priority = this->priority;
	clone->type = this->type;
	clone->family = this->family;

	if (this->endpoint)
	{
		clone->endpoint = this->endpoint->clone(this->endpoint);
	}

	if (this->base)
	{
		clone->base = this->base->clone(this->base);
	}

	return &clone->public;
}

METHOD(endpoint_notify_t, destroy, void,
	private_endpoint_notify_t *this)
{
	DESTROY_IF(this->endpoint);
	DESTROY_IF(this->base);
	free(this);
}

static private_endpoint_notify_t *endpoint_notify_create()
{
	private_endpoint_notify_t *this;

	INIT(this,
		.public = {
			.get_priority = _get_priority,
			.set_priority = _set_priority,
			.get_type = _get_type,
			.get_family = _get_family,
			.get_host = _get_host,
			.get_base = _get_base,
			.build_notify = _build_notify,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.family = NO_FAMILY,
		.type = NO_TYPE,
	);

	return this;
}

endpoint_notify_t *endpoint_notify_create_from_host(me_endpoint_type_t type,
													host_t *host, host_t *base)
{
	private_endpoint_notify_t *this = endpoint_notify_create();
	this->type = type;

	switch(type)
	{
		case HOST:
			this->priority = pow(2, 16) * ME_PRIO_HOST;
			break;
		case PEER_REFLEXIVE:
			this->priority = pow(2, 16) * ME_PRIO_PEER;
			break;
		case SERVER_REFLEXIVE:
			this->priority = pow(2, 16) * ME_PRIO_SERVER;
			break;
		case RELAYED:
		default:
			this->priority = pow(2, 16) * ME_PRIO_RELAY;
			break;
	}

	
	this->priority += 65535;

	if (!host)
	{
		return &this->public;
	}

	switch(host->get_family(host))
	{
		case AF_INET:
			this->family = IPv4;
			break;
		case AF_INET6:
			this->family = IPv6;
			break;
		default:
			return &this->public;
	}

	this->endpoint = host->clone(host);

	if (base)
	{
		this->base = base->clone(base);
	}

	return &this->public;
}

endpoint_notify_t *endpoint_notify_create_from_payload(notify_payload_t *notify)
{
	private_endpoint_notify_t *this;
	chunk_t data;

	if (notify->get_notify_type(notify) != ME_ENDPOINT)
	{
		return NULL;
	}

	this = endpoint_notify_create();
	data = notify->get_notification_data(notify);

	if (parse_notification_data(this, data) != SUCCESS)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}