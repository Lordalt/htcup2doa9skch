/*
 * Copyright (C) 2006-2012 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
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


#ifndef KERNEL_IPSEC_H_
#define KERNEL_IPSEC_H_

typedef struct kernel_ipsec_t kernel_ipsec_t;

#include <networking/host.h>
#include <ipsec/ipsec_types.h>
#include <selectors/traffic_selector.h>
#include <plugins/plugin.h>
#include <kernel_wrap/kernel_interface.h>

struct kernel_ipsec_t {

	kernel_feature_t (*get_features)(kernel_ipsec_t *this);

	status_t (*get_spi)(kernel_ipsec_t *this, host_t *src, host_t *dst,
						u_int8_t protocol, u_int32_t reqid, u_int32_t *spi);

	status_t (*get_cpi)(kernel_ipsec_t *this, host_t *src, host_t *dst,
						u_int32_t reqid, u_int16_t *cpi);

	status_t (*add_sa) (kernel_ipsec_t *this,
						host_t *src, host_t *dst, u_int32_t spi,
						u_int8_t protocol, u_int32_t reqid,
						mark_t mark, u_int32_t tfc, lifetime_cfg_t *lifetime,
						u_int16_t enc_alg, chunk_t enc_key,
						u_int16_t int_alg, chunk_t int_key,
						ipsec_mode_t mode, u_int16_t ipcomp, u_int16_t cpi,
						bool initiator, bool encap, bool esn, bool inbound,
						traffic_selector_t *src_ts, traffic_selector_t *dst_ts);

	status_t (*update_sa)(kernel_ipsec_t *this,
						  u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
						  host_t *src, host_t *dst,
						  host_t *new_src, host_t *new_dst,
						  bool encap, bool new_encap, mark_t mark);

	status_t (*query_sa) (kernel_ipsec_t *this, host_t *src, host_t *dst,
						  u_int32_t spi, u_int8_t protocol, mark_t mark,
						  u_int64_t *bytes, u_int64_t *packets, time_t *time);

	status_t (*del_sa) (kernel_ipsec_t *this, host_t *src, host_t *dst,
						u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
						mark_t mark);

	status_t (*flush_sas) (kernel_ipsec_t *this);

	status_t (*add_policy) (kernel_ipsec_t *this,
							host_t *src, host_t *dst,
							traffic_selector_t *src_ts,
							traffic_selector_t *dst_ts,
							policy_dir_t direction, policy_type_t type,
							ipsec_sa_cfg_t *sa, mark_t mark,
							policy_priority_t priority);

	status_t (*query_policy) (kernel_ipsec_t *this,
							  traffic_selector_t *src_ts,
							  traffic_selector_t *dst_ts,
							  policy_dir_t direction, mark_t mark,
							  time_t *use_time);

	status_t (*del_policy) (kernel_ipsec_t *this,
							traffic_selector_t *src_ts,
							traffic_selector_t *dst_ts,
							policy_dir_t direction, u_int32_t reqid,
							mark_t mark, policy_priority_t priority);

	status_t (*flush_policies) (kernel_ipsec_t *this);

	bool (*bypass_socket)(kernel_ipsec_t *this, int fd, int family);

	bool (*enable_udp_decap)(kernel_ipsec_t *this, int fd, int family,
							 u_int16_t port);

	void (*destroy) (kernel_ipsec_t *this);
};

bool kernel_ipsec_register(plugin_t *plugin, plugin_feature_t *feature,
						   bool reg, void *data);

#endif 
