/*
 * Copyright (C) 2012 Tobias Brunner
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


#ifndef PROPOSAL_SUBSTRUCTURE_H_
#define PROPOSAL_SUBSTRUCTURE_H_

typedef enum encap_t encap_t;
typedef struct proposal_substructure_t proposal_substructure_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/transform_substructure.h>
#include <config/proposal.h>
#include <collections/linked_list.h>
#include <kernel_wrap/kernel_ipsec.h>
#include <sa/authenticator.h>

enum encap_t {
	ENCAP_NONE = 0,
	ENCAP_UDP,
	ENCAP_UDP_DRAFT_00_03,
};

struct proposal_substructure_t {

	payload_t payload_interface;

	void (*set_proposal_number) (proposal_substructure_t *this,
								 u_int8_t proposal_number);
	u_int8_t (*get_proposal_number) (proposal_substructure_t *this);

	void (*set_protocol_id) (proposal_substructure_t *this,
							 u_int8_t protocol_id);

	u_int8_t (*get_protocol_id) (proposal_substructure_t *this);

	void (*set_is_last_proposal) (proposal_substructure_t *this, bool is_last);

	chunk_t (*get_spi) (proposal_substructure_t *this);

	void (*set_spi) (proposal_substructure_t *this, chunk_t spi);

	bool (*get_cpi) (proposal_substructure_t *this, u_int16_t *cpi);

	void (*get_proposals) (proposal_substructure_t *this, linked_list_t *list);

	enumerator_t* (*create_substructure_enumerator)(proposal_substructure_t *this);

	u_int32_t (*get_lifetime)(proposal_substructure_t *this);

	u_int64_t (*get_lifebytes)(proposal_substructure_t *this);

	auth_method_t (*get_auth_method)(proposal_substructure_t *this);

	ipsec_mode_t (*get_encap_mode)(proposal_substructure_t *this, bool *udp);

	void (*destroy) (proposal_substructure_t *this);
};

proposal_substructure_t *proposal_substructure_create(payload_type_t type);

proposal_substructure_t *proposal_substructure_create_from_proposal_v2(
														proposal_t *proposal);
proposal_substructure_t *proposal_substructure_create_from_proposal_v1(
			proposal_t *proposal,  u_int32_t lifetime, u_int64_t lifebytes,
			auth_method_t auth, ipsec_mode_t mode, encap_t udp);

proposal_substructure_t *proposal_substructure_create_from_proposals_v1(
			linked_list_t *proposals, u_int32_t lifetime, u_int64_t lifebytes,
			auth_method_t auth, ipsec_mode_t mode, encap_t udp);

proposal_substructure_t *proposal_substructure_create_for_ipcomp_v1(
			u_int32_t lifetime, u_int64_t lifebytes, u_int16_t cpi,
			ipsec_mode_t mode, encap_t udp, u_int8_t proposal_number);

#endif 
