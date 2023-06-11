/*
 * Copyright (C) 2006-2011 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
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


#ifndef MESSAGE_H_
#define MESSAGE_H_

typedef struct message_t message_t;

#include <library.h>
#include <encoding/payloads/ike_header.h>
#include <encoding/payloads/notify_payload.h>
#include <sa/keymat.h>
#include <sa/ike_sa_id.h>
#include <networking/packet.h>
#include <collections/linked_list.h>

struct message_t {

	void (*set_major_version) (message_t *this, u_int8_t major_version);

	u_int8_t (*get_major_version) (message_t *this);

	void (*set_minor_version) (message_t *this, u_int8_t minor_version);

	u_int8_t (*get_minor_version) (message_t *this);

	void (*set_message_id) (message_t *this, u_int32_t message_id);

	u_int32_t (*get_message_id) (message_t *this);

	u_int64_t (*get_initiator_spi) (message_t *this);

	u_int64_t (*get_responder_spi) (message_t *this);

	void (*set_ike_sa_id) (message_t *this, ike_sa_id_t *ike_sa_id);

	ike_sa_id_t *(*get_ike_sa_id) (message_t *this);

	void (*set_exchange_type) (message_t *this, exchange_type_t exchange_type);

	exchange_type_t (*get_exchange_type) (message_t *this);

	payload_type_t (*get_first_payload_type) (message_t *this);

	void (*set_request) (message_t *this, bool request);

	bool (*get_request) (message_t *this);

	void (*set_version_flag)(message_t *this);

	bool (*get_reserved_header_bit)(message_t *this, u_int nr);

	void (*set_reserved_header_bit)(message_t *this, u_int nr);

	void (*add_payload) (message_t *this, payload_t *payload);

	void (*add_notify) (message_t *this, bool flush, notify_type_t type,
						chunk_t data);

	void (*disable_sort)(message_t *this);

	status_t (*parse_header) (message_t *this);

	status_t (*parse_body) (message_t *this, keymat_t *keymat);

	status_t (*generate) (message_t *this, keymat_t *keymat, packet_t **packet);

	bool (*is_encoded)(message_t *this);

	host_t * (*get_source) (message_t *this);

	void (*set_source) (message_t *this, host_t *host);

	host_t * (*get_destination) (message_t *this);

	void (*set_destination) (message_t *this, host_t *host);

	enumerator_t * (*create_payload_enumerator) (message_t *this);

	void (*remove_payload_at)(message_t *this, enumerator_t *enumerator);

	payload_t* (*get_payload) (message_t *this, payload_type_t type);

	notify_payload_t* (*get_notify)(message_t *this, notify_type_t type);

	packet_t * (*get_packet) (message_t *this);

	chunk_t (*get_packet_data) (message_t *this);

	void (*destroy) (message_t *this);
};

message_t *message_create_from_packet(packet_t *packet);

message_t *message_create(int major, int minor);

#endif 
