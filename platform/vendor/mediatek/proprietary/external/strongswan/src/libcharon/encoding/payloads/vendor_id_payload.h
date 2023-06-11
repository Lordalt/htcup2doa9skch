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


#ifndef VENDOR_ID_PAYLOAD_H_
#define VENDOR_ID_PAYLOAD_H_

typedef struct vendor_id_payload_t vendor_id_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

struct vendor_id_payload_t {

	payload_t payload_interface;

	chunk_t (*get_data)(vendor_id_payload_t *this);

	void (*destroy)(vendor_id_payload_t *this);
};

vendor_id_payload_t *vendor_id_payload_create(payload_type_t type);

vendor_id_payload_t *vendor_id_payload_create_data(payload_type_t type,
												   chunk_t data);

#endif 
