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


#ifndef ENDPOINT_NOTIFY_H_
#define ENDPOINT_NOTIFY_H_

#define ME_PRIO_HOST   255
#define ME_PRIO_PEER   128
#define ME_PRIO_SERVER 64
#define ME_PRIO_RELAY  0

typedef enum me_endpoint_family_t me_endpoint_family_t;
typedef enum me_endpoint_type_t me_endpoint_type_t;
typedef struct endpoint_notify_t endpoint_notify_t;

#include <encoding/payloads/notify_payload.h>

enum me_endpoint_family_t {

	NO_FAMILY = 0,

	IPv4 = 1,

	IPv6 = 2,

	MAX_FAMILY = 3

};

enum me_endpoint_type_t {

	NO_TYPE = 0,

	HOST = 1,

	PEER_REFLEXIVE = 2,

	SERVER_REFLEXIVE = 3,

	RELAYED = 4,

	MAX_TYPE = 5

};

extern enum_name_t *me_endpoint_type_names;

struct endpoint_notify_t {
	u_int32_t (*get_priority) (endpoint_notify_t *this);

	void (*set_priority) (endpoint_notify_t *this, u_int32_t priority);

	me_endpoint_type_t (*get_type) (endpoint_notify_t *this);

	me_endpoint_family_t (*get_family) (endpoint_notify_t *this);

	host_t *(*get_host) (endpoint_notify_t *this);

	host_t *(*get_base) (endpoint_notify_t *this);

	notify_payload_t *(*build_notify) (endpoint_notify_t *this);

	endpoint_notify_t *(*clone) (endpoint_notify_t *this);

	void (*destroy) (endpoint_notify_t *this);
};

endpoint_notify_t *endpoint_notify_create_from_host(me_endpoint_type_t type,
													host_t *host, host_t *base);

endpoint_notify_t *endpoint_notify_create_from_payload(notify_payload_t *notify);

#endif 
