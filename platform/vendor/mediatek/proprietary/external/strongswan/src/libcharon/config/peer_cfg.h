/*
 * Copyright (C) 2007-2008 Tobias Brunner
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


#ifndef PEER_CFG_H_
#define PEER_CFG_H_

typedef enum cert_policy_t cert_policy_t;
typedef enum unique_policy_t unique_policy_t;
typedef struct peer_cfg_t peer_cfg_t;

#include <library.h>
#include <utils/identification.h>
#include <collections/enumerator.h>
#include <selectors/traffic_selector.h>
#include <config/proposal.h>
#include <config/ike_cfg.h>
#include <config/child_cfg.h>
#include <credentials/auth_cfg.h>

enum cert_policy_t {
	
	CERT_ALWAYS_SEND =		0,
	
	CERT_SEND_IF_ASKED =	1,
	
	CERT_NEVER_SEND =		2,
};

extern enum_name_t *cert_policy_names;

enum unique_policy_t {
	
	UNIQUE_NEVER,
	
	UNIQUE_NO,
	
	UNIQUE_REPLACE,
	
	UNIQUE_KEEP,
};

extern enum_name_t *unique_policy_names;

struct peer_cfg_t {

	char* (*get_name) (peer_cfg_t *this);

	ike_version_t (*get_ike_version)(peer_cfg_t *this);

	ike_cfg_t* (*get_ike_cfg) (peer_cfg_t *this);

	void (*add_child_cfg) (peer_cfg_t *this, child_cfg_t *child_cfg);

	void (*remove_child_cfg)(peer_cfg_t *this, enumerator_t *enumerator);

	enumerator_t* (*create_child_cfg_enumerator) (peer_cfg_t *this);

	child_cfg_t* (*select_child_cfg) (peer_cfg_t *this,
							linked_list_t *my_ts, linked_list_t *other_ts,
							linked_list_t *my_hosts, linked_list_t *other_hosts);

	void (*add_auth_cfg)(peer_cfg_t *this, auth_cfg_t *cfg, bool local);

	enumerator_t* (*create_auth_cfg_enumerator)(peer_cfg_t *this, bool local);

	cert_policy_t (*get_cert_policy) (peer_cfg_t *this);

	unique_policy_t (*get_unique_policy) (peer_cfg_t *this);

	u_int32_t (*get_keyingtries) (peer_cfg_t *this);

	u_int32_t (*get_rekey_time)(peer_cfg_t *this, bool jitter);

	u_int32_t (*get_reauth_time)(peer_cfg_t *this, bool jitter);

	u_int32_t (*get_over_time)(peer_cfg_t *this);

	bool (*use_mobike) (peer_cfg_t *this);

	bool (*use_aggressive)(peer_cfg_t *this);

	bool (*use_pull_mode)(peer_cfg_t *this);

	u_int32_t (*get_dpd) (peer_cfg_t *this);

	u_int32_t (*get_dpd_timeout) (peer_cfg_t *this);

	void (*add_virtual_ip)(peer_cfg_t *this, host_t *vip);

	void (*add_pcscf)(peer_cfg_t *this, host_t *pcscf);
	void (*set_imei)(peer_cfg_t *this, const char *imei);
	char* (*get_imei)(peer_cfg_t *this);
	void (*add_intsubnet)(peer_cfg_t *this, host_t *subnet);
	void (*add_intnetmask)(peer_cfg_t *this, host_t *netmask);
	
	enumerator_t* (*create_virtual_ip_enumerator)(peer_cfg_t *this);

	enumerator_t* (*create_pcscf_enumerator)(peer_cfg_t *this);
	enumerator_t* (*create_intsubnet_enumerator)(peer_cfg_t *this);
	enumerator_t* (*create_intnetmask_enumerator)(peer_cfg_t *this);
	
	void (*add_pool)(peer_cfg_t *this, char *name);

	enumerator_t* (*create_pool_enumerator)(peer_cfg_t *this);

#ifdef ME
	bool (*is_mediation) (peer_cfg_t *this);

	peer_cfg_t* (*get_mediated_by) (peer_cfg_t *this);

	identification_t* (*get_peer_id) (peer_cfg_t *this);
#endif 

	bool (*equals)(peer_cfg_t *this, peer_cfg_t *other);

	peer_cfg_t* (*get_ref) (peer_cfg_t *this);

	void (*destroy) (peer_cfg_t *this);
};

peer_cfg_t *peer_cfg_create(char *name,
							ike_cfg_t *ike_cfg, cert_policy_t cert_policy,
							unique_policy_t unique, u_int32_t keyingtries,
							u_int32_t rekey_time, u_int32_t reauth_time,
							u_int32_t jitter_time, u_int32_t over_time,
							bool mobike, bool aggressive, bool pull_mode,
							u_int32_t dpd, u_int32_t dpd_timeout,
							bool mediation, peer_cfg_t *mediated_by,
							identification_t *peer_id);

#endif 
