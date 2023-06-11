/*
 * Copyright (C) 2010-2013 Tobias Brunner
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


#ifndef KERNEL_LISTENER_H_
#define KERNEL_LISTENER_H_

typedef struct kernel_listener_t kernel_listener_t;

#include <networking/host.h>
#include <networking/tun_device.h>
#include <selectors/traffic_selector.h>
#include <kernel_wrap/kernel_ipsec.h>

struct kernel_listener_t {

	bool (*acquire)(kernel_listener_t *this, u_int32_t reqid,
					traffic_selector_t *src_ts, traffic_selector_t *dst_ts);

	bool (*expire)(kernel_listener_t *this, u_int32_t reqid,
				   u_int8_t protocol, u_int32_t spi, bool hard);

	bool (*mapping)(kernel_listener_t *this, u_int32_t reqid, u_int32_t spi,
					host_t *remote);

	bool (*migrate)(kernel_listener_t *this, u_int32_t reqid,
					traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
					policy_dir_t direction, host_t *local, host_t *remote);

	bool (*roam)(kernel_listener_t *this, bool address);

	bool (*tun)(kernel_listener_t *this, tun_device_t *tun, bool created);
};

#endif 
