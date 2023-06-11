/*
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


#ifndef TS_PAYLOAD_H_
#define TS_PAYLOAD_H_

typedef struct ts_payload_t ts_payload_t;

#include <library.h>
#include <collections/linked_list.h>
#include <selectors/traffic_selector.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/traffic_selector_substructure.h>

struct ts_payload_t {

	payload_t payload_interface;

	bool (*get_initiator) (ts_payload_t *this);

	void (*set_initiator) (ts_payload_t *this,bool is_initiator);

	linked_list_t *(*get_traffic_selectors) (ts_payload_t *this);

	void (*destroy) (ts_payload_t *this);
};

ts_payload_t *ts_payload_create(bool is_initiator);

ts_payload_t *ts_payload_create_from_traffic_selectors(bool is_initiator,
											linked_list_t *traffic_selectors);

#endif 