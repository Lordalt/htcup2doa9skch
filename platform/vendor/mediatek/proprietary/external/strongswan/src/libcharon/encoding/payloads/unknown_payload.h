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


#ifndef UNKNOWN_PAYLOAD_H_
#define UNKNOWN_PAYLOAD_H_

typedef struct unknown_payload_t unknown_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

struct unknown_payload_t {

	payload_t payload_interface;

	chunk_t (*get_data) (unknown_payload_t *this);

	bool (*is_critical) (unknown_payload_t *this);

	void (*destroy) (unknown_payload_t *this);
};

unknown_payload_t *unknown_payload_create(payload_type_t type);

unknown_payload_t *unknown_payload_create_data(payload_type_t type,
											   bool critical, chunk_t data);

#endif 
