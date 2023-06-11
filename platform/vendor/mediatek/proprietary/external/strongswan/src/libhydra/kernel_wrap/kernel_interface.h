/*
 * Copyright (C) 2006-2013 Tobias Brunner
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

/*
 * Copyright (c) 2012 Nanoteq Pty Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef KERNEL_INTERFACE_H_
#define KERNEL_INTERFACE_H_

typedef struct kernel_interface_t kernel_interface_t;
typedef enum kernel_feature_t kernel_feature_t;

#include <networking/host.h>
#include <crypto/prf_plus.h>

#include <kernel_wrap/kernel_listener.h>
#include <kernel_wrap/kernel_ipsec.h>
#include <kernel_wrap/kernel_net.h>

enum kernel_feature_t {
	
	KERNEL_ESP_V3_TFC = (1<<0),
	
	KERNEL_REQUIRE_EXCLUDE_ROUTE = (1<<1),
	
	KERNEL_REQUIRE_UDP_ENCAPSULATION = (1<<2),
};

typedef kernel_ipsec_t* (*kernel_ipsec_constructor_t)(void);

typedef kernel_net_t* (*kernel_net_constructor_t)(void);

struct kernel_interface_t {

	kernel_feature_t (*get_features)(kernel_interface_t *this);

	status_t (*get_spi)(kernel_interface_t *this, host_t *src, host_t *dst,
						u_int8_t protocol, u_int32_t reqid, u_int32_t *spi);

	status_t (*get_cpi)(kernel_interface_t *this, host_t *src, host_t *dst,
						u_int32_t reqid, u_int16_t *cpi);

	status_t (*add_sa) (kernel_interface_t *this,
						host_t *src, host_t *dst, u_int32_t spi,
						u_int8_t protocol, u_int32_t reqid, mark_t mark,
						u_int32_t tfc, lifetime_cfg_t *lifetime,
						u_int16_t enc_alg, chunk_t enc_key,
						u_int16_t int_alg, chunk_t int_key,
						ipsec_mode_t mode, u_int16_t ipcomp, u_int16_t cpi,
						bool initiator, bool encap, bool esn, bool inbound,
						traffic_selector_t *src_ts, traffic_selector_t *dst_ts);

	status_t (*update_sa)(kernel_interface_t *this,
						  u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
						  host_t *src, host_t *dst,
						  host_t *new_src, host_t *new_dst,
						  bool encap, bool new_encap, mark_t mark);

	status_t (*query_sa) (kernel_interface_t *this, host_t *src, host_t *dst,
						  u_int32_t spi, u_int8_t protocol, mark_t mark,
						  u_int64_t *bytes, u_int64_t *packets, time_t *time);

	status_t (*del_sa) (kernel_interface_t *this, host_t *src, host_t *dst,
						u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
						mark_t mark);

	status_t (*flush_sas) (kernel_interface_t *this);

	status_t (*add_policy) (kernel_interface_t *this,
							host_t *src, host_t *dst,
							traffic_selector_t *src_ts,
							traffic_selector_t *dst_ts,
							policy_dir_t direction, policy_type_t type,
							ipsec_sa_cfg_t *sa, mark_t mark,
							policy_priority_t priority);

	status_t (*query_policy) (kernel_interface_t *this,
							  traffic_selector_t *src_ts,
							  traffic_selector_t *dst_ts,
							  policy_dir_t direction, mark_t mark,
							  time_t *use_time);

	status_t (*del_policy) (kernel_interface_t *this,
							traffic_selector_t *src_ts,
							traffic_selector_t *dst_ts,
							policy_dir_t direction, u_int32_t reqid,
							mark_t mark, policy_priority_t priority);

	status_t (*flush_policies) (kernel_interface_t *this);

	host_t* (*get_source_addr)(kernel_interface_t *this,
							   host_t *dest, host_t *src);

	host_t* (*get_nexthop)(kernel_interface_t *this, host_t *dest, host_t *src);

	bool (*get_interface)(kernel_interface_t *this, host_t *host, char **name);

	enumerator_t *(*create_address_enumerator) (kernel_interface_t *this,
												kernel_address_type_t which);

	status_t (*add_ip) (kernel_interface_t *this, host_t *virtual_ip, int prefix,
						char *iface);

	status_t (*del_ip) (kernel_interface_t *this, host_t *virtual_ip,
						int prefix, bool wait);

	status_t (*add_route) (kernel_interface_t *this, chunk_t dst_net,
						   u_int8_t prefixlen, host_t *gateway, host_t *src_ip,
						   char *if_name);

	status_t (*del_route) (kernel_interface_t *this, chunk_t dst_net,
						   u_int8_t prefixlen, host_t *gateway, host_t *src_ip,
						   char *if_name);

	bool (*bypass_socket)(kernel_interface_t *this, int fd, int family);

	bool (*enable_udp_decap)(kernel_interface_t *this, int fd, int family,
							 u_int16_t port);



	bool (*is_interface_usable)(kernel_interface_t *this, const char *iface);

	bool (*all_interfaces_usable)(kernel_interface_t *this);

	status_t (*get_address_by_ts)(kernel_interface_t *this,
								  traffic_selector_t *ts, host_t **ip, bool *vip);

	void (*add_ipsec_interface)(kernel_interface_t *this,
								kernel_ipsec_constructor_t create);

	void (*remove_ipsec_interface)(kernel_interface_t *this,
								   kernel_ipsec_constructor_t create);

	void (*add_net_interface)(kernel_interface_t *this,
							  kernel_net_constructor_t create);

	void (*remove_net_interface)(kernel_interface_t *this,
								 kernel_net_constructor_t create);

	void (*add_listener)(kernel_interface_t *this,
						 kernel_listener_t *listener);

	void (*remove_listener)(kernel_interface_t *this,
							kernel_listener_t *listener);

	void (*acquire)(kernel_interface_t *this, u_int32_t reqid,
					traffic_selector_t *src_ts, traffic_selector_t *dst_ts);

	void (*expire)(kernel_interface_t *this, u_int32_t reqid,
				   u_int8_t protocol, u_int32_t spi, bool hard);

	void (*mapping)(kernel_interface_t *this, u_int32_t reqid, u_int32_t spi,
					host_t *remote);

	void (*migrate)(kernel_interface_t *this, u_int32_t reqid,
					traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
					policy_dir_t direction, host_t *local, host_t *remote);

	void (*roam)(kernel_interface_t *this, bool address);

	void (*tun)(kernel_interface_t *this, tun_device_t *tun, bool created);

	void (*register_algorithm)(kernel_interface_t *this, u_int16_t alg_id,
							   transform_type_t type, u_int16_t kernel_id,
							   char *kernel_name);

	bool (*lookup_algorithm)(kernel_interface_t *this, u_int16_t alg_id,
							 transform_type_t type, u_int16_t *kernel_id,
							 char **kernel_name);

	void (*destroy) (kernel_interface_t *this);
};

kernel_interface_t *kernel_interface_create(void);

#endif 
