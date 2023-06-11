/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
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


#ifndef IKE_CFG_H_
#define IKE_CFG_H_

typedef enum ike_version_t ike_version_t;
typedef enum fragmentation_t fragmentation_t;
typedef struct ike_cfg_t ike_cfg_t;

#include <library.h>
#include <networking/host.h>
#include <collections/linked_list.h>
#include <utils/identification.h>
#include <config/proposal.h>
#include <crypto/diffie_hellman.h>

enum ike_version_t {
	
	IKE_ANY = 0,
	
	IKEV1 = 1,
	
	IKEV2 = 2,
};

enum fragmentation_t {
	
	FRAGMENTATION_NO,
	
	FRAGMENTATION_YES,
	
	FRAGMENTATION_FORCE,
};

extern enum_name_t *ike_version_names;

struct ike_cfg_t {

	ike_version_t (*get_version)(ike_cfg_t *this);

	host_t* (*resolve_me)(ike_cfg_t *this, int family);

	host_t* (*resolve_other)(ike_cfg_t *this, int family);

	u_int (*match_me)(ike_cfg_t *this, host_t *host);

	u_int (*match_other)(ike_cfg_t *this, host_t *host);

	char* (*get_my_addr) (ike_cfg_t *this);

	char* (*get_other_addr) (ike_cfg_t *this);

	u_int16_t (*get_my_port)(ike_cfg_t *this);

	u_int16_t (*get_other_port)(ike_cfg_t *this);

	u_int8_t (*get_dscp)(ike_cfg_t *this);

	void (*add_proposal) (ike_cfg_t *this, proposal_t *proposal);

	linked_list_t* (*get_proposals) (ike_cfg_t *this);

	proposal_t *(*select_proposal) (ike_cfg_t *this, linked_list_t *proposals,
									bool private);

	bool (*send_certreq) (ike_cfg_t *this);

	bool (*force_encap) (ike_cfg_t *this);

	fragmentation_t (*fragmentation) (ike_cfg_t *this);

	diffie_hellman_group_t (*get_dh_group)(ike_cfg_t *this);

	bool (*equals)(ike_cfg_t *this, ike_cfg_t *other);

	ike_cfg_t* (*get_ref) (ike_cfg_t *this);

	void (*destroy) (ike_cfg_t *this);
	
	
	char* (*get_vif) (ike_cfg_t *this);
};

ike_cfg_t *ike_cfg_create(ike_version_t version, bool certreq, bool force_encap,
						  char *me, u_int16_t my_port,
						  char *other, u_int16_t other_port,
						  fragmentation_t fragmentation, u_int8_t dscp);
ike_cfg_t *ike_cfg_create2(ike_version_t version, bool certreq, bool force_encap,
						  char *me, u_int16_t my_port,
						  char *other, u_int16_t other_port,
						  fragmentation_t fragmentation, u_int8_t dscp, char *vif);

#endif 
