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


#ifndef KE_PAYLOAD_H_
#define KE_PAYLOAD_H_

typedef struct ke_payload_t ke_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/transform_substructure.h>
#include <collections/linked_list.h>
#include <crypto/diffie_hellman.h>

struct ke_payload_t {

	payload_t payload_interface;

	chunk_t (*get_key_exchange_data) (ke_payload_t *this);

	diffie_hellman_group_t (*get_dh_group_number) (ke_payload_t *this);

	void (*destroy) (ke_payload_t *this);
};

ke_payload_t *ke_payload_create(payload_type_t type);

ke_payload_t *ke_payload_create_from_diffie_hellman(payload_type_t type,
													diffie_hellman_t *dh);

#endif 
