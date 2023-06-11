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


#ifndef TRANSFORM_SUBSTRUCTURE_H_
#define TRANSFORM_SUBSTRUCTURE_H_

typedef struct transform_substructure_t transform_substructure_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/transform_attribute.h>
#include <collections/linked_list.h>
#include <crypto/diffie_hellman.h>
#include <crypto/signers/signer.h>
#include <crypto/prfs/prf.h>
#include <crypto/crypters/crypter.h>
#include <config/proposal.h>

#define TRANSFORM_TYPE_VALUE 3

struct transform_substructure_t {

	payload_t payload_interface;

	void (*add_transform_attribute) (transform_substructure_t *this,
									 transform_attribute_t *attribute);

	void (*set_is_last_transform) (transform_substructure_t *this, bool is_last);

	u_int8_t (*get_transform_type_or_number) (transform_substructure_t *this);

	u_int16_t (*get_transform_id) (transform_substructure_t *this);

	enumerator_t* (*create_attribute_enumerator)(transform_substructure_t *this);

	void (*destroy) (transform_substructure_t *this);
};

transform_substructure_t *transform_substructure_create(payload_type_t type);

transform_substructure_t *transform_substructure_create_type(payload_type_t type,
										u_int8_t type_or_number, u_int16_t id);

#endif 
