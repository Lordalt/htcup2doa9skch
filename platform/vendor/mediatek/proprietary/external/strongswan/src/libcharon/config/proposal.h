/*
 * Copyright (C) 2006 Martin Willi
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


#ifndef PROPOSAL_H_
#define PROPOSAL_H_

typedef enum protocol_id_t protocol_id_t;
typedef enum extended_sequence_numbers_t extended_sequence_numbers_t;
typedef struct proposal_t proposal_t;

#include <library.h>
#include <utils/identification.h>
#include <collections/linked_list.h>
#include <networking/host.h>
#include <crypto/transform.h>
#include <crypto/crypters/crypter.h>
#include <crypto/signers/signer.h>
#include <crypto/diffie_hellman.h>
#include <selectors/traffic_selector.h>

enum protocol_id_t {
	PROTO_NONE = 0,
	PROTO_IKE = 1,
	PROTO_AH = 2,
	PROTO_ESP = 3,
	PROTO_IPCOMP = 4, 
};

extern enum_name_t *protocol_id_names;

struct proposal_t {

	void (*add_algorithm) (proposal_t *this, transform_type_t type,
						   u_int16_t alg, u_int16_t key_size);

	enumerator_t *(*create_enumerator) (proposal_t *this, transform_type_t type);

	bool (*get_algorithm) (proposal_t *this, transform_type_t type,
						   u_int16_t *alg, u_int16_t *key_size);

	bool (*has_dh_group) (proposal_t *this, diffie_hellman_group_t group);

	void (*strip_dh)(proposal_t *this, diffie_hellman_group_t keep);

	proposal_t *(*select) (proposal_t *this, proposal_t *other, bool private);

	protocol_id_t (*get_protocol) (proposal_t *this);

	u_int64_t (*get_spi) (proposal_t *this);

	void (*set_spi) (proposal_t *this, u_int64_t spi);

	u_int (*get_number)(proposal_t *this);

	bool (*equals)(proposal_t *this, proposal_t *other);

	proposal_t *(*clone) (proposal_t *this);

	void (*destroy) (proposal_t *this);
};

proposal_t *proposal_create(protocol_id_t protocol, u_int number);

proposal_t *proposal_create_default(protocol_id_t protocol);

proposal_t *proposal_create_from_string(protocol_id_t protocol, const char *algs);

int proposal_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
						 const void *const *args);

#endif 
