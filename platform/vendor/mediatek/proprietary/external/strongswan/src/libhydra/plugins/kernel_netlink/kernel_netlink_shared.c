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

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <errno.h>
#include <unistd.h>

#include "kernel_netlink_shared.h"

#include <utils/debug.h>
#include <threading/mutex.h>

typedef struct private_netlink_socket_t private_netlink_socket_t;

struct private_netlink_socket_t {
	netlink_socket_t public;

	mutex_t *mutex;

	int seq;

	int protocol;

	int socket;
};

extern enum_name_t *xfrm_msg_names;

METHOD(netlink_socket_t, netlink_send, status_t,
	private_netlink_socket_t *this, struct nlmsghdr *in, struct nlmsghdr **out,
	size_t *out_len)
{
	int len, addr_len;
	struct sockaddr_nl addr;
	chunk_t result = chunk_empty, tmp;
	struct nlmsghdr *msg, peek;

	this->mutex->lock(this->mutex);

	in->nlmsg_seq = ++this->seq;
	in->nlmsg_pid = getpid();

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = 0;
	addr.nl_groups = 0;

	if (this->protocol == NETLINK_XFRM)
	{
		chunk_t in_chunk = { (u_char*)in, in->nlmsg_len };

		DBG3(DBG_KNL, "sending %N: %B", xfrm_msg_names, in->nlmsg_type, &in_chunk);
	}

	while (TRUE)
	{
		len = sendto(this->socket, in, in->nlmsg_len, 0,
					 (struct sockaddr*)&addr, sizeof(addr));

		if (len != in->nlmsg_len)
		{
			if (errno == EINTR)
			{
				
				continue;
			}
			this->mutex->unlock(this->mutex);
			DBG1(DBG_KNL, "error sending to netlink socket: %s", strerror(errno));
			return FAILED;
		}
		break;
	}

	while (TRUE)
	{
		char buf[4096];
		tmp.len = sizeof(buf);
		tmp.ptr = buf;
		msg = (struct nlmsghdr*)tmp.ptr;

		memset(&addr, 0, sizeof(addr));
		addr.nl_family = AF_NETLINK;
		addr.nl_pid = getpid();
		addr.nl_groups = 0;
		addr_len = sizeof(addr);

		len = recvfrom(this->socket, tmp.ptr, tmp.len, 0,
					   (struct sockaddr*)&addr, &addr_len);

		if (len < 0)
		{
			if (errno == EINTR)
			{
				DBG1(DBG_KNL, "got interrupted");
				
				continue;
			}
			DBG1(DBG_KNL, "error reading from netlink socket: %s", strerror(errno));
			this->mutex->unlock(this->mutex);
			free(result.ptr);
			return FAILED;
		}
		if (!NLMSG_OK(msg, len))
		{
			DBG1(DBG_KNL, "received corrupted netlink message");
			this->mutex->unlock(this->mutex);
			free(result.ptr);
			return FAILED;
		}
		if (msg->nlmsg_seq != this->seq)
		{
			DBG1(DBG_KNL, "received invalid netlink sequence number");
			if (msg->nlmsg_seq < this->seq)
			{
				continue;
			}
			this->mutex->unlock(this->mutex);
			free(result.ptr);
			return FAILED;
		}

		tmp.len = len;
		result.ptr = realloc(result.ptr, result.len + tmp.len);
		memcpy(result.ptr + result.len, tmp.ptr, tmp.len);
		result.len += tmp.len;

		len = recvfrom(this->socket, &peek, sizeof(peek), MSG_PEEK | MSG_DONTWAIT,
					   (struct sockaddr*)&addr, &addr_len);

		if (len == sizeof(peek) && peek.nlmsg_seq == this->seq)
		{
			
			continue;
		}
		break;
	}

	*out_len = result.len;
	*out = (struct nlmsghdr*)result.ptr;

	this->mutex->unlock(this->mutex);

	return SUCCESS;
}

METHOD(netlink_socket_t, netlink_send_ack, status_t,
	private_netlink_socket_t *this, struct nlmsghdr *in)
{
	struct nlmsghdr *out, *hdr;
	size_t len;

	if (netlink_send(this, in, &out, &len) != SUCCESS)
	{
		return FAILED;
	}
	hdr = out;
	while (NLMSG_OK(hdr, len))
	{
		switch (hdr->nlmsg_type)
		{
			case NLMSG_ERROR:
			{
				struct nlmsgerr* err = (struct nlmsgerr*)NLMSG_DATA(hdr);

				if (err->error)
				{
					if (-err->error == EEXIST)
					{	
						free(out);
						return ALREADY_DONE;
					}
					if (-err->error == ESRCH)
					{	
						free(out);
						return NOT_FOUND;
					}
					DBG1(DBG_KNL, "received netlink error: %s (%d)",
						 strerror(-err->error), -err->error);
					free(out);
					return FAILED;
				}
				free(out);
				return SUCCESS;
			}
			default:
				hdr = NLMSG_NEXT(hdr, len);
				continue;
			case NLMSG_DONE:
				break;
		}
		break;
	}
	DBG1(DBG_KNL, "netlink request not acknowledged");
	free(out);
	return FAILED;
}

METHOD(netlink_socket_t, destroy, void,
	private_netlink_socket_t *this)
{
	if (this->socket > 0)
	{
		close(this->socket);
	}
	this->mutex->destroy(this->mutex);
	free(this);
}

netlink_socket_t *netlink_socket_create(int protocol)
{
	private_netlink_socket_t *this;
	struct sockaddr_nl addr;

	INIT(this,
		.public = {
			.send = _netlink_send,
			.send_ack = _netlink_send_ack,
			.destroy = _destroy,
		},
		.seq = 200,
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.protocol = protocol,
	);

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;

	this->socket = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (this->socket < 0)
	{
		DBG1(DBG_KNL, "unable to create netlink socket");
		destroy(this);
		return NULL;
	}

	addr.nl_groups = 0;
	if (bind(this->socket, (struct sockaddr*)&addr, sizeof(addr)))
	{
		DBG1(DBG_KNL, "unable to bind netlink socket");
		destroy(this);
		return NULL;
	}

	return &this->public;
}

void netlink_add_attribute(struct nlmsghdr *hdr, int rta_type, chunk_t data,
						  size_t buflen)
{
	struct rtattr *rta;

	if (NLMSG_ALIGN(hdr->nlmsg_len) + RTA_LENGTH(data.len) > buflen)
	{
		DBG1(DBG_KNL, "unable to add attribute, buffer too small");
		return;
	}

	rta = (struct rtattr*)(((char*)hdr) + NLMSG_ALIGN(hdr->nlmsg_len));
	rta->rta_type = rta_type;
	rta->rta_len = RTA_LENGTH(data.len);
	memcpy(RTA_DATA(rta), data.ptr, data.len);
	hdr->nlmsg_len = NLMSG_ALIGN(hdr->nlmsg_len) + rta->rta_len;
}

void* netlink_reserve(struct nlmsghdr *hdr, int buflen, int type, int len)
{
	struct rtattr *rta;

	if (NLMSG_ALIGN(hdr->nlmsg_len) + RTA_LENGTH(len) > buflen)
	{
		DBG1(DBG_KNL, "unable to add attribute, buffer too small");
		return NULL;
	}

	rta = ((void*)hdr) + NLMSG_ALIGN(hdr->nlmsg_len);
	rta->rta_type = type;
	rta->rta_len = RTA_LENGTH(len);
	hdr->nlmsg_len = NLMSG_ALIGN(hdr->nlmsg_len) + rta->rta_len;

	return RTA_DATA(rta);
}
