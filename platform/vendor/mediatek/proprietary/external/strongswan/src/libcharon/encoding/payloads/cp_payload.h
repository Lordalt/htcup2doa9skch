/*
 * Copyright (C) 2005-2009 Martin Willi
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


#ifndef CP_PAYLOAD_H_
#define CP_PAYLOAD_H_

typedef enum config_type_t config_type_t;
typedef struct cp_payload_t cp_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/configuration_attribute.h>
#include <collections/enumerator.h>

enum config_type_t {
	CFG_REQUEST = 1,
	CFG_REPLY = 2,
	CFG_SET = 3,
	CFG_ACK = 4,
};

extern enum_name_t *config_type_names;

struct cp_payload_t {

	payload_t payload_interface;

	enumerator_t *(*create_attribute_enumerator) (cp_payload_t *this);

	void (*add_attribute)(cp_payload_t *this,
						  configuration_attribute_t *attribute);

	config_type_t (*get_type) (cp_payload_t *this);

	void (*set_identifier) (cp_payload_t *this, u_int16_t identifier);

	u_int16_t (*get_identifier) (cp_payload_t *this);

	void (*destroy) (cp_payload_t *this);
};

cp_payload_t *cp_payload_create(payload_type_t type);

cp_payload_t *cp_payload_create_type(payload_type_t type, config_type_t cfg_type);

#endif 
