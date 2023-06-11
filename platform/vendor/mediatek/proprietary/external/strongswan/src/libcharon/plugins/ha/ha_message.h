/*
 * Copyright (C) 2008 Martin Willi
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


#ifndef HA_MESSAGE_H_
#define HA_MESSAGE_H_

#include <library.h>
#include <networking/host.h>
#include <utils/identification.h>
#include <sa/ike_sa_id.h>
#include <selectors/traffic_selector.h>

#define HA_MESSAGE_VERSION 3

typedef struct ha_message_t ha_message_t;
typedef enum ha_message_type_t ha_message_type_t;
typedef enum ha_message_attribute_t ha_message_attribute_t;
typedef union ha_message_value_t ha_message_value_t;

enum ha_message_type_t {
	
	HA_IKE_ADD = 1,
	
	HA_IKE_UPDATE,
	
	HA_IKE_MID_INITIATOR,
	
	HA_IKE_MID_RESPONDER,
	
	HA_IKE_DELETE,
	
	HA_CHILD_ADD,
	
	HA_CHILD_DELETE,
	
	HA_SEGMENT_DROP,
	
	HA_SEGMENT_TAKE,
	
	HA_STATUS,
	
	HA_RESYNC,
	
	HA_IKE_IV,
};

extern enum_name_t *ha_message_type_names;

enum ha_message_attribute_t {
	
	HA_IKE_ID = 1,
	
	HA_IKE_REKEY_ID,
	
	HA_LOCAL_ID,
	
	HA_REMOTE_ID,
	
	HA_REMOTE_EAP_ID,
	
	HA_LOCAL_ADDR,
	
	HA_REMOTE_ADDR,
	
	HA_CONFIG_NAME,
	
	HA_CONDITIONS,
	
	HA_EXTENSIONS,
	
	HA_LOCAL_VIP,
	
	HA_REMOTE_VIP,
	
	HA_PEER_ADDR,
	
	HA_INITIATOR,
	
	HA_NONCE_I,
	
	HA_NONCE_R,
	
	HA_SECRET,
	
	HA_OLD_SKD,
	
	HA_ALG_PRF,
	
	HA_ALG_OLD_PRF,
	
	HA_ALG_ENCR,
	
	HA_ALG_ENCR_LEN,
	
	HA_ALG_INTEG,
	
	HA_IPSEC_MODE,
	
	HA_IPCOMP,
	
	HA_INBOUND_SPI,
	
	HA_OUTBOUND_SPI,
	
	HA_INBOUND_CPI,
	
	HA_OUTBOUND_CPI,
	
	HA_LOCAL_TS,
	
	HA_REMOTE_TS,
	
	HA_MID,
	
	HA_SEGMENT,
	
	HA_ESN,
	
	HA_IKE_VERSION,
	
	HA_LOCAL_DH,
	
	HA_REMOTE_DH,
	
	HA_PSK,
	
	HA_IV,
};

union ha_message_value_t {
	u_int8_t u8;
	u_int16_t u16;
	u_int32_t u32;
	char *str;
	chunk_t chunk;
	ike_sa_id_t *ike_sa_id;
	identification_t *id;
	host_t *host;
	traffic_selector_t *ts;
};

struct ha_message_t {

	ha_message_type_t (*get_type)(ha_message_t *this);

	void (*add_attribute)(ha_message_t *this,
						  ha_message_attribute_t attribute, ...);

	enumerator_t* (*create_attribute_enumerator)(ha_message_t *this);

	chunk_t (*get_encoding)(ha_message_t *this);

	void (*destroy)(ha_message_t *this);
};

ha_message_t *ha_message_create(ha_message_type_t type);

ha_message_t *ha_message_parse(chunk_t data);

#endif 
