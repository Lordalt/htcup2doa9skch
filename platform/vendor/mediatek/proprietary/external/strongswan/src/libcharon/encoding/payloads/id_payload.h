/*
 * Copyright (C) 2007 Tobias Brunner
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


#ifndef ID_PAYLOAD_H_
#define ID_PAYLOAD_H_

typedef struct id_payload_t id_payload_t;

#include <library.h>
#include <utils/identification.h>
#include <encoding/payloads/payload.h>
#include <selectors/traffic_selector.h>

struct id_payload_t {

	payload_t payload_interface;

	identification_t *(*get_identification) (id_payload_t *this);

	traffic_selector_t* (*get_ts)(id_payload_t *this);

	chunk_t (*get_encoded)(id_payload_t *this);

	void (*destroy) (id_payload_t *this);
};

id_payload_t *id_payload_create(payload_type_t type);

id_payload_t *id_payload_create_from_identification(payload_type_t type,
													identification_t *id);

id_payload_t *id_payload_create_from_ts(traffic_selector_t *ts);

#endif 
