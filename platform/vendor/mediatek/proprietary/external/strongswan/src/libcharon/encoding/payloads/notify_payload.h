/*
 * Copyright (C) 2006-2008 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
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


#ifndef NOTIFY_PAYLOAD_H_
#define NOTIFY_PAYLOAD_H_

typedef enum notify_type_t notify_type_t;
typedef struct notify_payload_t notify_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/proposal_substructure.h>
#include <collections/linked_list.h>

enum notify_type_t {
	
	UNSUPPORTED_CRITICAL_PAYLOAD = 1,
	
	INVALID_PAYLOAD_TYPE = 1,
	INVALID_IKE_SPI = 4,
	INVALID_MAJOR_VERSION = 5,
	INVALID_SYNTAX = 7,
	
	INVALID_EXCHANGE_TYPE = 7,
	INVALID_MESSAGE_ID = 9,
	INVALID_SPI = 11,
	
	ATTRIBUTES_NOT_SUPPORTED = 13,
	
	NO_PROPOSAL_CHOSEN = 14,
	
	PAYLOAD_MALFORMED = 16,
	INVALID_KE_PAYLOAD = 17,
	
	INVALID_KEY_INFORMATION = 17,
	
	INVALID_ID_INFORMATION = 18,
	INVALID_CERT_ENCODING = 19,
	INVALID_CERTIFICATE = 20,
	CERT_TYPE_UNSUPPORTED = 21,
	INVALID_CERT_AUTHORITY = 22,
	INVALID_HASH_INFORMATION = 23,
	AUTHENTICATION_FAILED = 24,
	SINGLE_PAIR_REQUIRED = 34,
	NO_ADDITIONAL_SAS = 35,
	INTERNAL_ADDRESS_FAILURE = 36,
	FAILED_CP_REQUIRED = 37,
	TS_UNACCEPTABLE = 38,
	INVALID_SELECTORS = 39,
	
	UNACCEPTABLE_ADDRESSES = 40,
	UNEXPECTED_NAT_DETECTED = 41,
	
	USE_ASSIGNED_HoA = 42,
	
	TEMPORARY_FAILURE = 43,
	CHILD_SA_NOT_FOUND = 44,

	
	ME_CONNECT_FAILED = 8192,

	
	MS_NOTIFY_STATUS = 12345,

	
	INITIAL_CONTACT = 16384,
	SET_WINDOW_SIZE = 16385,
	ADDITIONAL_TS_POSSIBLE = 16386,
	IPCOMP_SUPPORTED = 16387,
	NAT_DETECTION_SOURCE_IP = 16388,
	NAT_DETECTION_DESTINATION_IP = 16389,
	COOKIE = 16390,
	USE_TRANSPORT_MODE = 16391,
	HTTP_CERT_LOOKUP_SUPPORTED = 16392,
	REKEY_SA = 16393,
	ESP_TFC_PADDING_NOT_SUPPORTED = 16394,
	NON_FIRST_FRAGMENTS_ALSO = 16395,
	
	MOBIKE_SUPPORTED = 16396,
	ADDITIONAL_IP4_ADDRESS = 16397,
	ADDITIONAL_IP6_ADDRESS = 16398,
	NO_ADDITIONAL_ADDRESSES = 16399,
	UPDATE_SA_ADDRESSES = 16400,
	COOKIE2 = 16401,
	NO_NATS_ALLOWED = 16402,
	
	AUTH_LIFETIME = 16403,
	
	MULTIPLE_AUTH_SUPPORTED = 16404,
	ANOTHER_AUTH_FOLLOWS = 16405,
	
	REDIRECT_SUPPORTED = 16406,
	REDIRECT = 16407,
	REDIRECTED_FROM = 16408,
	
	TICKET_LT_OPAQUE = 16409,
	TICKET_REQUEST = 16410,
	TICKET_ACK = 16411,
	TICKET_NACK = 16412,
	TICKET_OPAQUE = 16413,
	
	LINK_ID = 16414,
	
	USE_WESP_MODE = 16415,
	
	ROHC_SUPPORTED = 16416,
	
	EAP_ONLY_AUTHENTICATION = 16417,
	
	CHILDLESS_IKEV2_SUPPORTED = 16418,
	
	QUICK_CRASH_DETECTION = 16419,
	
	IKEV2_MESSAGE_ID_SYNC_SUPPORTED = 16420,
	IKEV2_REPLAY_COUNTER_SYNC_SUPPORTED = 16421,
	IKEV2_MESSAGE_ID_SYNC = 16422,
	IPSEC_REPLAY_COUNTER_SYNC = 16423,
	
	SECURE_PASSWORD_METHOD = 16424,
	
	PSK_PERSIST = 16425,
	PSK_CONFIRM = 16426,
	
	ERX_SUPPORTED = 16427,
	
	IFOM_CAPABILITY = 16428,
	
	INITIAL_CONTACT_IKEV1 = 24578,
	
	DPD_R_U_THERE = 36136,
	DPD_R_U_THERE_ACK = 36137,
	
	UNITY_LOAD_BALANCE = 40501,
	
	USE_BEET_MODE = 40961,
	
	ME_MEDIATION = 40962,
	ME_ENDPOINT = 40963,
	ME_CALLBACK = 40964,
	ME_CONNECTID = 40965,
	ME_CONNECTKEY = 40966,
	ME_CONNECTAUTH = 40967,
	ME_RESPONSE = 40968,
	
	RADIUS_ATTRIBUTE = 40969,
};

extern enum_name_t *notify_type_names;

extern enum_name_t *notify_type_short_names;

struct notify_payload_t {
	payload_t payload_interface;

	u_int8_t (*get_protocol_id) (notify_payload_t *this);

	void (*set_protocol_id) (notify_payload_t *this, u_int8_t protocol_id);

	notify_type_t (*get_notify_type) (notify_payload_t *this);

	void (*set_notify_type) (notify_payload_t *this, notify_type_t type);

	u_int32_t (*get_spi) (notify_payload_t *this);

	void (*set_spi) (notify_payload_t *this, u_int32_t spi);

	chunk_t (*get_spi_data) (notify_payload_t *this);

	void (*set_spi_data) (notify_payload_t *this, chunk_t spi);

	chunk_t (*get_notification_data) (notify_payload_t *this);

	void (*set_notification_data) (notify_payload_t *this,
								   chunk_t notification_data);

	void (*destroy) (notify_payload_t *this);
};

notify_payload_t *notify_payload_create(payload_type_t type);

notify_payload_t *notify_payload_create_from_protocol_and_type(
			payload_type_t type, protocol_id_t protocol, notify_type_t notify);


inline bool is_error_notify(notify_payload_t*);
inline bool is_status_notify(notify_payload_t*);

#endif 
