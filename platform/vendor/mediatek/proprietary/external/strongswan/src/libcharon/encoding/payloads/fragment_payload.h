/*
 * Copyright (C) 2012 Tobias Brunner
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


#ifndef FRAGMENT_PAYLOAD_H_
#define FRAGMENT_PAYLOAD_H_

typedef struct fragment_payload_t fragment_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

struct fragment_payload_t {

	payload_t payload_interface;

	u_int16_t (*get_id)(fragment_payload_t *this);

	u_int8_t (*get_number)(fragment_payload_t *this);

	bool (*is_last)(fragment_payload_t *this);

	chunk_t (*get_data)(fragment_payload_t *this);

	void (*destroy)(fragment_payload_t *this);
};

fragment_payload_t *fragment_payload_create();

fragment_payload_t *fragment_payload_create_from_data(u_int8_t num, bool last,
													  chunk_t data);

#endif 
