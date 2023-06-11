/*
 * Copyright (C) 2008-2012 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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


#ifndef KERNEL_NET_H_
#define KERNEL_NET_H_

typedef struct kernel_net_t kernel_net_t;
typedef enum kernel_address_type_t kernel_address_type_t;

#include <collections/enumerator.h>
#include <networking/host.h>
#include <plugins/plugin.h>
#include <kernel_wrap/kernel_interface.h>

enum kernel_address_type_t {
	
	ADDR_TYPE_REGULAR = (1 << 0),
	
	ADDR_TYPE_DOWN =  (1 << 1),
	
	ADDR_TYPE_IGNORED = (1 << 2),
	
	ADDR_TYPE_LOOPBACK = (1 << 3),
	
	ADDR_TYPE_VIRTUAL = (1 << 4),
	
	ADDR_TYPE_ALL = (1 << 5) - 1,
};

struct kernel_net_t {

	kernel_feature_t (*get_features)(kernel_net_t *this);

	host_t* (*get_source_addr)(kernel_net_t *this, host_t *dest, host_t *src);

	host_t* (*get_nexthop)(kernel_net_t *this, host_t *dest, host_t *src);

	bool (*get_interface) (kernel_net_t *this, host_t *host, char **name);

	enumerator_t *(*create_address_enumerator) (kernel_net_t *this,
												kernel_address_type_t which);

	status_t (*add_ip) (kernel_net_t *this, host_t *virtual_ip, int prefix,
						char *iface);

	status_t (*del_ip) (kernel_net_t *this, host_t *virtual_ip, int prefix,
						bool wait);

	status_t (*add_route) (kernel_net_t *this, chunk_t dst_net,
						   u_int8_t prefixlen, host_t *gateway, host_t *src_ip,
						   char *if_name);

	status_t (*del_route) (kernel_net_t *this, chunk_t dst_net,
						   u_int8_t prefixlen, host_t *gateway, host_t *src_ip,
						   char *if_name);

	void (*destroy) (kernel_net_t *this);
};

bool kernel_net_register(plugin_t *plugin, plugin_feature_t *feature,
						 bool reg, void *data);

#endif 
