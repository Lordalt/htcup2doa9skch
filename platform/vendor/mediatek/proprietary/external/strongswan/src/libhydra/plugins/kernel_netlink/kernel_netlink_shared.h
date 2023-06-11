/*
 * Copyright (C) 2008 Tobias Brunner
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

#ifndef KERNEL_NETLINK_SHARED_H_
#define KERNEL_NETLINK_SHARED_H_

#include <library.h>

#include <linux/rtnetlink.h>

typedef u_char netlink_buf_t[1024] __attribute__((aligned(RTA_ALIGNTO)));

typedef struct netlink_socket_t netlink_socket_t;

struct netlink_socket_t {

	status_t (*send)(netlink_socket_t *this, struct nlmsghdr *in,
					 struct nlmsghdr **out, size_t *out_len);

	status_t (*send_ack)(netlink_socket_t *this, struct nlmsghdr *in);

	void (*destroy)(netlink_socket_t *this);
};

netlink_socket_t *netlink_socket_create(int protocol);

void netlink_add_attribute(struct nlmsghdr *hdr, int rta_type, chunk_t data,
						   size_t buflen);

void* netlink_reserve(struct nlmsghdr *hdr, int buflen, int type, int len);

#endif 
