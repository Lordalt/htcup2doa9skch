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


#ifndef SA_PAYLOAD_H_
#define SA_PAYLOAD_H_

typedef struct sa_payload_t sa_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/proposal_substructure.h>
#include <collections/linked_list.h>
#include <kernel_wrap/kernel_ipsec.h>
#include <sa/authenticator.h>

struct sa_payload_t {

	payload_t payload_interface;

	linked_list_t *(*get_proposals) (sa_payload_t *this);

	linked_list_t *(*get_ipcomp_proposals) (sa_payload_t *this, u_int16_t *cpi);

	u_int32_t (*get_lifetime)(sa_payload_t *this);

	u_int64_t (*get_lifebytes)(sa_payload_t *this);

	auth_method_t (*get_auth_method)(sa_payload_t *this);

	ipsec_mode_t (*get_encap_mode)(sa_payload_t *this, bool *udp);

	enumerator_t* (*create_substructure_enumerator)(sa_payload_t *this);

	void (*destroy) (sa_payload_t *this);
};

sa_payload_t *sa_payload_create(payload_type_t type);

sa_payload_t *sa_payload_create_from_proposals_v2(linked_list_t *proposals);

sa_payload_t *sa_payload_create_from_proposal_v2(proposal_t *proposal);

sa_payload_t *sa_payload_create_from_proposals_v1(linked_list_t *proposals,
							u_int32_t lifetime, u_int64_t lifebytes,
							auth_method_t auth, ipsec_mode_t mode, encap_t udp,
							u_int16_t cpi);

sa_payload_t *sa_payload_create_from_proposal_v1(proposal_t *proposal,
							u_int32_t lifetime, u_int64_t lifebytes,
							auth_method_t auth, ipsec_mode_t mode, encap_t udp,
							u_int16_t cpi);

#endif 
