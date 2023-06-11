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


#ifndef TRAFFIC_SELECTOR_SUBSTRUCTURE_H_
#define TRAFFIC_SELECTOR_SUBSTRUCTURE_H_

typedef struct traffic_selector_substructure_t traffic_selector_substructure_t;

#include <library.h>
#include <networking/host.h>
#include <selectors/traffic_selector.h>
#include <encoding/payloads/payload.h>

struct traffic_selector_substructure_t {
	payload_t payload_interface;

	ts_type_t (*get_ts_type) (traffic_selector_substructure_t *this);

	void (*set_ts_type) (traffic_selector_substructure_t *this,
						 ts_type_t ts_type);

	u_int8_t (*get_protocol_id) (traffic_selector_substructure_t *this);

	void (*set_protocol_id) (traffic_selector_substructure_t *this,
							  u_int8_t protocol_id);

	host_t *(*get_start_host) (traffic_selector_substructure_t *this);

	void (*set_start_host) (traffic_selector_substructure_t *this,
							host_t *start_host);

	host_t *(*get_end_host) (traffic_selector_substructure_t *this);

	void (*set_end_host) (traffic_selector_substructure_t *this,
						  host_t *end_host);

	traffic_selector_t *(*get_traffic_selector) (
										traffic_selector_substructure_t *this);

	void (*destroy) (traffic_selector_substructure_t *this);
};

traffic_selector_substructure_t *traffic_selector_substructure_create(void);

traffic_selector_substructure_t *traffic_selector_substructure_create_from_traffic_selector(
										traffic_selector_t *traffic_selector);

#endif 
