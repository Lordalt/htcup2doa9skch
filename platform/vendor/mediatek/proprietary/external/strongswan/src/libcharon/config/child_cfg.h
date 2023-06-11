/*
 * Copyright (C) 2008-2009 Tobias Brunner
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


#ifndef CHILD_CFG_H_
#define CHILD_CFG_H_

typedef enum action_t action_t;
typedef struct child_cfg_t child_cfg_t;

#include <library.h>
#include <selectors/traffic_selector.h>
#include <config/proposal.h>
#include <kernel_wrap/kernel_ipsec.h>

enum action_t {
	
	ACTION_NONE,
	
	ACTION_ROUTE,
	
	ACTION_RESTART,
};

extern enum_name_t *action_names;

struct child_cfg_t {

	char *(*get_name) (child_cfg_t *this);

	void (*add_proposal) (child_cfg_t *this, proposal_t *proposal);

	linked_list_t* (*get_proposals)(child_cfg_t *this, bool strip_dh);

	proposal_t* (*select_proposal)(child_cfg_t*this, linked_list_t *proposals,
								   bool strip_dh, bool private);

	void (*add_traffic_selector)(child_cfg_t *this, bool local,
								 traffic_selector_t *ts);

	linked_list_t *(*get_traffic_selectors)(child_cfg_t *this, bool local,
											linked_list_t *supplied,
											linked_list_t *hosts);
	char* (*get_updown)(child_cfg_t *this);

	bool (*get_hostaccess) (child_cfg_t *this);

	lifetime_cfg_t* (*get_lifetime) (child_cfg_t *this);

	ipsec_mode_t (*get_mode) (child_cfg_t *this);

	action_t (*get_start_action) (child_cfg_t *this);

	action_t (*get_dpd_action) (child_cfg_t *this);

	action_t (*get_close_action) (child_cfg_t *this);

	diffie_hellman_group_t (*get_dh_group)(child_cfg_t *this);

	bool (*use_ipcomp)(child_cfg_t *this);

	u_int32_t (*get_inactivity)(child_cfg_t *this);

	u_int32_t (*get_reqid)(child_cfg_t *this);

	mark_t (*get_mark)(child_cfg_t *this, bool inbound);

	u_int32_t (*get_tfc)(child_cfg_t *this);

	void (*set_mipv6_options)(child_cfg_t *this, bool proxy_mode,
												 bool install_policy);

	bool (*use_proxy_mode)(child_cfg_t *this);

	bool (*install_policy)(child_cfg_t *this);

	child_cfg_t* (*get_ref) (child_cfg_t *this);

	void (*destroy) (child_cfg_t *this);
};

child_cfg_t *child_cfg_create(char *name, lifetime_cfg_t *lifetime,
							  char *updown, bool hostaccess,
							  ipsec_mode_t mode, action_t start_action,
							  action_t dpd_action, action_t close_action,
							  bool ipcomp, u_int32_t inactivity, u_int32_t reqid,
							  mark_t *mark_in, mark_t *mark_out, u_int32_t tfc);

#endif 
