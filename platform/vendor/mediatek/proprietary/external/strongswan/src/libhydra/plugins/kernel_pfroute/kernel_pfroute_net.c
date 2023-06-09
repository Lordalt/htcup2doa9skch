/*
 * Copyright (C) 2009-2013 Tobias Brunner
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

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/route.h>
#include <unistd.h>
#include <errno.h>

#include "kernel_pfroute_net.h"

#include <hydra.h>
#include <utils/debug.h>
#include <networking/host.h>
#include <networking/tun_device.h>
#include <threading/thread.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <threading/rwlock.h>
#include <threading/spinlock.h>
#include <collections/hashtable.h>
#include <collections/linked_list.h>
#include <processing/jobs/callback_job.h>

#ifndef HAVE_STRUCT_SOCKADDR_SA_LEN
#error Cannot compile this plugin on systems where 'struct sockaddr' has no sa_len member.
#endif

#ifdef __APPLE__
#define SA_ALIGN 4
#else
#define SA_ALIGN sizeof(long)
#endif
#define SA_LEN(len) ((len) > 0 ? (((len)+SA_ALIGN-1) & ~(SA_ALIGN-1)) : SA_ALIGN)

#define ROAM_DELAY 100

#define ROUTE_DELAY 100

typedef struct addr_entry_t addr_entry_t;

struct addr_entry_t {

	
	host_t *ip;

	
	bool virtual;
};

static void addr_entry_destroy(addr_entry_t *this)
{
	this->ip->destroy(this->ip);
	free(this);
}

typedef struct iface_entry_t iface_entry_t;

struct iface_entry_t {

	
	int ifindex;

	
	char ifname[IFNAMSIZ];

	
	u_int flags;

	
	linked_list_t *addrs;

	
	bool usable;
};

static void iface_entry_destroy(iface_entry_t *this)
{
	this->addrs->destroy_function(this->addrs, (void*)addr_entry_destroy);
	free(this);
}

static inline bool iface_entry_up(iface_entry_t *iface)
{
	return (iface->flags & IFF_UP) == IFF_UP;
}

static inline bool iface_entry_up_and_usable(iface_entry_t *iface)
{
	return iface->usable && iface_entry_up(iface);
}

typedef struct addr_map_entry_t addr_map_entry_t;

struct addr_map_entry_t {
	
	host_t *ip;

	
	addr_entry_t *addr;

	
	iface_entry_t *iface;
};

static u_int addr_map_entry_hash(addr_map_entry_t *this)
{
	return chunk_hash(this->ip->get_address(this->ip));
}

static bool addr_map_entry_equals(addr_map_entry_t *a, addr_map_entry_t *b)
{
	return a->iface->ifindex == b->iface->ifindex &&
		   a->ip->ip_equals(a->ip, b->ip);
}

static bool addr_map_entry_match_up_and_usable(addr_map_entry_t *a,
											   addr_map_entry_t *b)
{
	return !b->addr->virtual && iface_entry_up_and_usable(b->iface) &&
			a->ip->ip_equals(a->ip, b->ip);
}

static bool addr_map_entry_match_virtual(addr_map_entry_t *a, addr_map_entry_t *b)
{
	return b->addr->virtual && a->ip->ip_equals(a->ip, b->ip);
}

static bool addr_map_entry_match_up(addr_map_entry_t *a, addr_map_entry_t *b)
{
	return !b->addr->virtual && iface_entry_up(b->iface) &&
			a->ip->ip_equals(a->ip, b->ip);
}

typedef struct route_entry_t route_entry_t;

struct route_entry_t {
	
	char *if_name;

	
	host_t *gateway;

	
	chunk_t dst_net;

	
	u_int8_t prefixlen;
};

static route_entry_t *route_entry_clone(route_entry_t *this)
{
	route_entry_t *route;

	INIT(route,
		.if_name = strdup(this->if_name),
		.gateway = this->gateway ? this->gateway->clone(this->gateway) : NULL,
		.dst_net = chunk_clone(this->dst_net),
		.prefixlen = this->prefixlen,
	);
	return route;
}

static void route_entry_destroy(route_entry_t *this)
{
	free(this->if_name);
	DESTROY_IF(this->gateway);
	chunk_free(&this->dst_net);
	free(this);
}

static u_int route_entry_hash(route_entry_t *this)
{
	return chunk_hash_inc(chunk_from_thing(this->prefixlen),
						  chunk_hash(this->dst_net));
}

static bool route_entry_equals(route_entry_t *a, route_entry_t *b)
{
	if (a->if_name && b->if_name && streq(a->if_name, b->if_name) &&
		chunk_equals(a->dst_net, b->dst_net) && a->prefixlen == b->prefixlen)
	{
		return (!a->gateway && !b->gateway) || (a->gateway && b->gateway &&
					a->gateway->ip_equals(a->gateway, b->gateway));
	}
	return FALSE;
}

typedef struct net_change_t net_change_t;

struct net_change_t {
	
	char *if_name;
};

static void net_change_destroy(net_change_t *this)
{
	free(this->if_name);
	free(this);
}

static u_int net_change_hash(net_change_t *this)
{
	return chunk_hash(chunk_create(this->if_name, strlen(this->if_name)));
}

static bool net_change_equals(net_change_t *a, net_change_t *b)
{
	return streq(a->if_name, b->if_name);
}

typedef struct private_kernel_pfroute_net_t private_kernel_pfroute_net_t;

struct private_kernel_pfroute_net_t
{
	kernel_pfroute_net_t public;

	rwlock_t *lock;

	linked_list_t *ifaces;

	hashtable_t *addrs;

	linked_list_t *tuns;

	mutex_t *mutex;

	condvar_t *condvar;

	hashtable_t *routes;

	mutex_t *routes_lock;

	hashtable_t *net_changes;

	mutex_t *net_changes_lock;

	timeval_t last_route_reinstall;

	pid_t pid;

	int socket;

	int seq;

	int waiting_seq;

	struct rt_msghdr *reply;

	timeval_t next_roam;

	bool roam_address;

	spinlock_t *roam_lock;

	int vip_wait;
};


static status_t manage_route(private_kernel_pfroute_net_t *this, int op,
							 chunk_t dst_net, u_int8_t prefixlen,
							 host_t *gateway, char *if_name);

static void net_changes_clear(private_kernel_pfroute_net_t *this)
{
	enumerator_t *enumerator;
	net_change_t *change;

	enumerator = this->net_changes->create_enumerator(this->net_changes);
	while (enumerator->enumerate(enumerator, NULL, (void**)&change))
	{
		this->net_changes->remove_at(this->net_changes, enumerator);
		net_change_destroy(change);
	}
	enumerator->destroy(enumerator);
}

static job_requeue_t reinstall_routes(private_kernel_pfroute_net_t *this)
{
	enumerator_t *enumerator;
	route_entry_t *route;

	this->net_changes_lock->lock(this->net_changes_lock);
	this->routes_lock->lock(this->routes_lock);

	enumerator = this->routes->create_enumerator(this->routes);
	while (enumerator->enumerate(enumerator, NULL, (void**)&route))
	{
		net_change_t *change, lookup = {
			.if_name = route->if_name,
		};
		
		change = this->net_changes->get(this->net_changes, &lookup);
		if (change)
		{
			manage_route(this, RTM_ADD, route->dst_net, route->prefixlen,
						 route->gateway, route->if_name);
		}
	}
	enumerator->destroy(enumerator);
	this->routes_lock->unlock(this->routes_lock);

	net_changes_clear(this);
	this->net_changes_lock->unlock(this->net_changes_lock);
	return JOB_REQUEUE_NONE;
}

static void queue_route_reinstall(private_kernel_pfroute_net_t *this,
								  char *if_name)
{
	net_change_t *update, *found;
	timeval_t now;
	job_t *job;

	INIT(update,
		.if_name = if_name
	);

	this->net_changes_lock->lock(this->net_changes_lock);
	found = this->net_changes->put(this->net_changes, update, update);
	if (found)
	{
		net_change_destroy(found);
	}
	time_monotonic(&now);
	if (timercmp(&now, &this->last_route_reinstall, >))
	{
		timeval_add_ms(&now, ROUTE_DELAY);
		this->last_route_reinstall = now;

		job = (job_t*)callback_job_create((callback_job_cb_t)reinstall_routes,
										  this, NULL, NULL);
		lib->scheduler->schedule_job_ms(lib->scheduler, job, ROUTE_DELAY);
	}
	this->net_changes_lock->unlock(this->net_changes_lock);
}

static void addr_map_entry_add(private_kernel_pfroute_net_t *this,
							   addr_entry_t *addr, iface_entry_t *iface)
{
	addr_map_entry_t *entry;

	INIT(entry,
		.ip = addr->ip,
		.addr = addr,
		.iface = iface,
	);
	entry = this->addrs->put(this->addrs, entry, entry);
	free(entry);
}

static void addr_map_entry_remove(addr_entry_t *addr, iface_entry_t *iface,
								  private_kernel_pfroute_net_t *this)
{
	addr_map_entry_t *entry, lookup = {
		.ip = addr->ip,
		.addr = addr,
		.iface = iface,
	};

	entry = this->addrs->remove(this->addrs, &lookup);
	free(entry);
}

static job_requeue_t roam_event(private_kernel_pfroute_net_t *this)
{
	bool address;

	this->roam_lock->lock(this->roam_lock);
	address = this->roam_address;
	this->roam_address = FALSE;
	this->roam_lock->unlock(this->roam_lock);
	hydra->kernel_interface->roam(hydra->kernel_interface, address);
	return JOB_REQUEUE_NONE;
}

static void fire_roam_event(private_kernel_pfroute_net_t *this, bool address)
{
	timeval_t now;
	job_t *job;

	time_monotonic(&now);
	this->roam_lock->lock(this->roam_lock);
	this->roam_address |= address;
	if (!timercmp(&now, &this->next_roam, >))
	{
		this->roam_lock->unlock(this->roam_lock);
		return;
	}
	timeval_add_ms(&now, ROAM_DELAY);
	this->next_roam = now;
	this->roam_lock->unlock(this->roam_lock);

	job = (job_t*)callback_job_create((callback_job_cb_t)roam_event,
									  this, NULL, NULL);
	lib->scheduler->schedule_job_ms(lib->scheduler, job, ROAM_DELAY);
}

typedef struct {
	
	enumerator_t public;
	
	int types;
	
	int remaining;
	
	struct sockaddr *addr;
} rt_enumerator_t;

METHOD(enumerator_t, rt_enumerate, bool,
	rt_enumerator_t *this, int *xtype, struct sockaddr **addr)
{
	int i, type;

	if (this->remaining < sizeof(this->addr->sa_len) ||
		this->remaining < this->addr->sa_len)
	{
		return FALSE;
	}
	for (i = 0; i < RTAX_MAX; i++)
	{
		type = (1 << i);
		if (this->types & type)
		{
			this->types &= ~type;
			*addr = this->addr;
			*xtype = i;
			this->remaining -= SA_LEN(this->addr->sa_len);
			this->addr = (struct sockaddr*)((char*)this->addr +
											SA_LEN(this->addr->sa_len));
			return TRUE;
		}
	}
	return FALSE;
}

static enumerator_t *create_rt_enumerator(int types, int remaining,
										  struct sockaddr *addr)
{
	rt_enumerator_t *this;

	INIT(this,
		.public = {
			.enumerate = (void*)_rt_enumerate,
			.destroy = (void*)free,
		},
		.types = types,
		.remaining = remaining,
		.addr = addr,
	);
	return &this->public;
}

static enumerator_t *create_rtmsg_enumerator(struct rt_msghdr *hdr)
{
	return create_rt_enumerator(hdr->rtm_addrs, hdr->rtm_msglen - sizeof(*hdr),
								(struct sockaddr *)(hdr + 1));
}

static enumerator_t *create_ifamsg_enumerator(struct ifa_msghdr *hdr)
{
	return create_rt_enumerator(hdr->ifam_addrs, hdr->ifam_msglen - sizeof(*hdr),
								(struct sockaddr *)(hdr + 1));
}

static void process_addr(private_kernel_pfroute_net_t *this,
						 struct ifa_msghdr *ifa)
{
	struct sockaddr *sockaddr;
	host_t *host = NULL;
	enumerator_t *ifaces, *addrs;
	iface_entry_t *iface;
	addr_entry_t *addr;
	bool found = FALSE, changed = FALSE, roam = FALSE;
	enumerator_t *enumerator;
	char *ifname = NULL;
	int type;

	enumerator = create_ifamsg_enumerator(ifa);
	while (enumerator->enumerate(enumerator, &type, &sockaddr))
	{
		if (type == RTAX_IFA)
		{
			host = host_create_from_sockaddr(sockaddr);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!host || host->is_anyaddr(host))
	{
		DESTROY_IF(host);
		return;
	}

	this->lock->write_lock(this->lock);
	ifaces = this->ifaces->create_enumerator(this->ifaces);
	while (ifaces->enumerate(ifaces, &iface))
	{
		if (iface->ifindex == ifa->ifam_index)
		{
			addrs = iface->addrs->create_enumerator(iface->addrs);
			while (addrs->enumerate(addrs, &addr))
			{
				if (host->ip_equals(host, addr->ip))
				{
					found = TRUE;
					if (ifa->ifam_type == RTM_DELADDR)
					{
						iface->addrs->remove_at(iface->addrs, addrs);
						if (!addr->virtual && iface->usable)
						{
							changed = TRUE;
							DBG1(DBG_KNL, "%H disappeared from %s",
								 host, iface->ifname);
						}
						addr_map_entry_remove(addr, iface, this);
						addr_entry_destroy(addr);
					}
				}
			}
			addrs->destroy(addrs);

			if (!found && ifa->ifam_type == RTM_NEWADDR)
			{
				INIT(addr,
					.ip = host->clone(host),
				);
				changed = TRUE;
				ifname = strdup(iface->ifname);
				iface->addrs->insert_last(iface->addrs, addr);
				addr_map_entry_add(this, addr, iface);
				if (iface->usable)
				{
					DBG1(DBG_KNL, "%H appeared on %s", host, iface->ifname);
				}
			}

			if (changed && iface_entry_up_and_usable(iface))
			{
				roam = TRUE;
			}
			break;
		}
	}
	ifaces->destroy(ifaces);
	this->lock->unlock(this->lock);
	host->destroy(host);

	if (roam && ifname)
	{
		queue_route_reinstall(this, ifname);
	}
	else
	{
		free(ifname);
	}

	if (roam)
	{
		fire_roam_event(this, TRUE);
	}
}

static void repopulate_iface(private_kernel_pfroute_net_t *this,
							 iface_entry_t *iface)
{
	struct ifaddrs *ifap, *ifa;
	addr_entry_t *addr;

	while (iface->addrs->remove_last(iface->addrs, (void**)&addr) == SUCCESS)
	{
		addr_map_entry_remove(addr, iface, this);
		addr_entry_destroy(addr);
	}

	if (getifaddrs(&ifap) == 0)
	{
		for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr && streq(ifa->ifa_name, iface->ifname))
			{
				switch (ifa->ifa_addr->sa_family)
				{
					case AF_INET:
					case AF_INET6:
						INIT(addr,
							.ip = host_create_from_sockaddr(ifa->ifa_addr),
						);
						iface->addrs->insert_last(iface->addrs, addr);
						addr_map_entry_add(this, addr, iface);
						break;
					default:
						break;
				}
			}
		}
		freeifaddrs(ifap);
	}
}

static void process_link(private_kernel_pfroute_net_t *this,
						 struct if_msghdr *msg)
{
	enumerator_t *enumerator;
	iface_entry_t *iface;
	bool roam = FALSE, found = FALSE, update_routes = FALSE;

	this->lock->write_lock(this->lock);
	enumerator = this->ifaces->create_enumerator(this->ifaces);
	while (enumerator->enumerate(enumerator, &iface))
	{
		if (iface->ifindex == msg->ifm_index)
		{
			if (iface->usable)
			{
				if (!(iface->flags & IFF_UP) && (msg->ifm_flags & IFF_UP))
				{
					roam = update_routes = TRUE;
					DBG1(DBG_KNL, "interface %s activated", iface->ifname);
				}
				else if ((iface->flags & IFF_UP) && !(msg->ifm_flags & IFF_UP))
				{
					roam = TRUE;
					DBG1(DBG_KNL, "interface %s deactivated", iface->ifname);
				}
			}
			iface->flags = msg->ifm_flags;
			repopulate_iface(this, iface);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		INIT(iface,
			.ifindex = msg->ifm_index,
			.flags = msg->ifm_flags,
			.addrs = linked_list_create(),
		);
		if (if_indextoname(iface->ifindex, iface->ifname))
		{
			DBG1(DBG_KNL, "interface %s appeared", iface->ifname);
			iface->usable = hydra->kernel_interface->is_interface_usable(
										hydra->kernel_interface, iface->ifname);
			repopulate_iface(this, iface);
			this->ifaces->insert_last(this->ifaces, iface);
			if (iface->usable)
			{
				roam = update_routes = TRUE;
			}
		}
		else
		{
			free(iface);
		}
	}
	this->lock->unlock(this->lock);

	if (update_routes)
	{
		queue_route_reinstall(this, strdup(iface->ifname));
	}

	if (roam)
	{
		fire_roam_event(this, TRUE);
	}
}

static void process_route(private_kernel_pfroute_net_t *this,
						  struct rt_msghdr *msg)
{

}

static bool receive_events(private_kernel_pfroute_net_t *this, int fd,
						   watcher_event_t event)
{
	struct {
		union {
			struct rt_msghdr rtm;
			struct if_msghdr ifm;
			struct ifa_msghdr ifam;
		};
		char buf[sizeof(struct sockaddr_storage) * RTAX_MAX];
	} msg;
	int len, hdrlen;

	len = recv(this->socket, &msg, sizeof(msg), MSG_DONTWAIT);
	if (len < 0)
	{
		switch (errno)
		{
			case EINTR:
			case EAGAIN:
				return TRUE;
			default:
				DBG1(DBG_KNL, "unable to receive from PF_ROUTE event socket");
				sleep(1);
				return TRUE;
		}
	}

	if (len < offsetof(struct rt_msghdr, rtm_flags) || len < msg.rtm.rtm_msglen)
	{
		DBG1(DBG_KNL, "received invalid PF_ROUTE message");
		return TRUE;
	}
	if (msg.rtm.rtm_version != RTM_VERSION)
	{
		DBG1(DBG_KNL, "received PF_ROUTE message with unsupported version: %d",
			 msg.rtm.rtm_version);
		return TRUE;
	}
	switch (msg.rtm.rtm_type)
	{
		case RTM_NEWADDR:
		case RTM_DELADDR:
			hdrlen = sizeof(msg.ifam);
			break;
		case RTM_IFINFO:
			hdrlen = sizeof(msg.ifm);
			break;
		case RTM_ADD:
		case RTM_DELETE:
		case RTM_GET:
			hdrlen = sizeof(msg.rtm);
			break;
		default:
			return TRUE;
	}
	if (msg.rtm.rtm_msglen < hdrlen)
	{
		DBG1(DBG_KNL, "ignoring short PF_ROUTE message");
		return TRUE;
	}
	switch (msg.rtm.rtm_type)
	{
		case RTM_NEWADDR:
		case RTM_DELADDR:
			process_addr(this, &msg.ifam);
			break;
		case RTM_IFINFO:
			process_link(this, &msg.ifm);
			break;
		case RTM_ADD:
		case RTM_DELETE:
			process_route(this, &msg.rtm);
			break;
		default:
			break;
	}

	this->mutex->lock(this->mutex);
	if (msg.rtm.rtm_pid == this->pid && msg.rtm.rtm_seq == this->waiting_seq)
	{
		
		this->reply = realloc(this->reply, msg.rtm.rtm_msglen);
		memcpy(this->reply, &msg, msg.rtm.rtm_msglen);
	}
	
	this->condvar->broadcast(this->condvar);
	this->mutex->unlock(this->mutex);

	return TRUE;
}


typedef struct {
	private_kernel_pfroute_net_t* this;
	
	kernel_address_type_t which;
} address_enumerator_t;

static void address_enumerator_destroy(address_enumerator_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

static bool filter_addresses(address_enumerator_t *data,
							 addr_entry_t** in, host_t** out)
{
	host_t *ip;
	if (!(data->which & ADDR_TYPE_VIRTUAL) && (*in)->virtual)
	{   
		return FALSE;
	}
	if (!(data->which & ADDR_TYPE_REGULAR) && !(*in)->virtual)
	{	
		return FALSE;
	}
	ip = (*in)->ip;
	if (ip->get_family(ip) == AF_INET6)
	{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ip->get_sockaddr(ip);
		if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr))
		{   
			return FALSE;
		}
	}
	*out = ip;
	return TRUE;
}

static enumerator_t *create_iface_enumerator(iface_entry_t *iface,
											 address_enumerator_t *data)
{
	return enumerator_create_filter(iface->addrs->create_enumerator(iface->addrs),
									(void*)filter_addresses, data, NULL);
}

static bool filter_interfaces(address_enumerator_t *data, iface_entry_t** in,
							  iface_entry_t** out)
{
	if (!(data->which & ADDR_TYPE_IGNORED) && !(*in)->usable)
	{	
		return FALSE;
	}
	if (!(data->which & ADDR_TYPE_LOOPBACK) && ((*in)->flags & IFF_LOOPBACK))
	{	
		return FALSE;
	}
	if (!(data->which & ADDR_TYPE_DOWN) && !((*in)->flags & IFF_UP))
	{	
		return FALSE;
	}
	*out = *in;
	return TRUE;
}

METHOD(kernel_net_t, create_address_enumerator, enumerator_t*,
	private_kernel_pfroute_net_t *this, kernel_address_type_t which)
{
	address_enumerator_t *data;

	INIT(data,
		.this = this,
		.which = which,
	);

	this->lock->read_lock(this->lock);
	return enumerator_create_nested(
				enumerator_create_filter(
					this->ifaces->create_enumerator(this->ifaces),
					(void*)filter_interfaces, data, NULL),
				(void*)create_iface_enumerator, data,
				(void*)address_enumerator_destroy);
}

METHOD(kernel_net_t, get_features, kernel_feature_t,
	private_kernel_pfroute_net_t *this)
{
	return KERNEL_REQUIRE_EXCLUDE_ROUTE;
}

METHOD(kernel_net_t, get_interface_name, bool,
	private_kernel_pfroute_net_t *this, host_t* ip, char **name)
{
	addr_map_entry_t *entry, lookup = {
		.ip = ip,
	};

	if (ip->is_anyaddr(ip))
	{
		return FALSE;
	}
	this->lock->read_lock(this->lock);
	
	entry = this->addrs->get_match(this->addrs, &lookup,
								  (void*)addr_map_entry_match_up_and_usable);
	if (entry)
	{
		if (name)
		{
			*name = strdup(entry->iface->ifname);
			DBG2(DBG_KNL, "%H is on interface %s", ip, *name);
		}
		this->lock->unlock(this->lock);
		return TRUE;
	}
	
	entry = this->addrs->get_match(this->addrs, &lookup,
								  (void*)addr_map_entry_match_virtual);
	if (entry)
	{
		if (name)
		{
			*name = strdup(entry->iface->ifname);
			DBG2(DBG_KNL, "virtual IP %H is on interface %s", ip, *name);
		}
		this->lock->unlock(this->lock);
		return TRUE;
	}
	
	entry = this->addrs->get_match(this->addrs, &lookup,
								  (void*)addr_map_entry_match_up);
	if (!entry)
	{	
		DBG2(DBG_KNL, "%H is not a local address or the interface is down", ip);
	}
	this->lock->unlock(this->lock);
	return FALSE;
}

METHOD(kernel_net_t, add_ip, status_t,
	private_kernel_pfroute_net_t *this, host_t *vip, int prefix,
	char *ifname)
{
	enumerator_t *ifaces, *addrs;
	iface_entry_t *iface;
	addr_entry_t *addr;
	tun_device_t *tun;
	bool timeout = FALSE;

	tun = tun_device_create(NULL);
	if (!tun)
	{
		return FAILED;
	}
	if (prefix == -1)
	{
		prefix = vip->get_address(vip).len * 8;
	}
	if (!tun->up(tun) || !tun->set_address(tun, vip, prefix))
	{
		tun->destroy(tun);
		return FAILED;
	}

	
	this->mutex->lock(this->mutex);
	while (!timeout && !get_interface_name(this, vip, NULL))
	{
		timeout = this->condvar->timed_wait(this->condvar, this->mutex,
											this->vip_wait);
	}
	this->mutex->unlock(this->mutex);
	if (timeout)
	{
		DBG1(DBG_KNL, "virtual IP %H did not appear on %s",
			 vip, tun->get_name(tun));
		tun->destroy(tun);
		return FAILED;
	}

	this->lock->write_lock(this->lock);
	this->tuns->insert_last(this->tuns, tun);

	ifaces = this->ifaces->create_enumerator(this->ifaces);
	while (ifaces->enumerate(ifaces, &iface))
	{
		if (streq(iface->ifname, tun->get_name(tun)))
		{
			addrs = iface->addrs->create_enumerator(iface->addrs);
			while (addrs->enumerate(addrs, &addr))
			{
				if (addr->ip->ip_equals(addr->ip, vip))
				{
					addr->virtual = TRUE;
				}
			}
			addrs->destroy(addrs);
			queue_route_reinstall(this, strdup(iface->ifname));
			break;
		}
	}
	ifaces->destroy(ifaces);
	hydra->kernel_interface->tun(hydra->kernel_interface, tun, TRUE);
	this->lock->unlock(this->lock);

	return SUCCESS;
}

METHOD(kernel_net_t, del_ip, status_t,
	private_kernel_pfroute_net_t *this, host_t *vip, int prefix,
	bool wait)
{
	enumerator_t *enumerator;
	tun_device_t *tun;
	host_t *addr;
	bool timeout = FALSE, found = FALSE;

	this->lock->write_lock(this->lock);
	enumerator = this->tuns->create_enumerator(this->tuns);
	while (enumerator->enumerate(enumerator, &tun))
	{
		addr = tun->get_address(tun, NULL);
		if (addr && addr->ip_equals(addr, vip))
		{
			this->tuns->remove_at(this->tuns, enumerator);
			hydra->kernel_interface->tun(hydra->kernel_interface, tun,
										 FALSE);
			tun->destroy(tun);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	if (!found)
	{
		return NOT_FOUND;
	}
	
	if (wait)
	{
		this->mutex->lock(this->mutex);
		while (!timeout && get_interface_name(this, vip, NULL))
		{
			timeout = this->condvar->timed_wait(this->condvar, this->mutex,
												this->vip_wait);
		}
		this->mutex->unlock(this->mutex);
		if (timeout)
		{
			DBG1(DBG_KNL, "virtual IP %H did not disappear from tun", vip);
			return FAILED;
		}
	}
	return SUCCESS;
}

static void add_rt_addr(struct rt_msghdr *hdr, int type, host_t *addr)
{
	if (addr)
	{
		int len;

		len = *addr->get_sockaddr_len(addr);
		memcpy((char*)hdr + hdr->rtm_msglen, addr->get_sockaddr(addr), len);
		hdr->rtm_msglen += SA_LEN(len);
		hdr->rtm_addrs |= type;
	}
}

static void add_rt_mask(struct rt_msghdr *hdr, int type, int family, int prefix)
{
	host_t *mask;

	mask = host_create_netmask(family, prefix);
	if (mask)
	{
		add_rt_addr(hdr, type, mask);
		mask->destroy(mask);
	}
}

static void add_rt_ifname(struct rt_msghdr *hdr, int type, char *name)
{
	struct sockaddr_dl sdl = {
		.sdl_len = sizeof(struct sockaddr_dl),
		.sdl_family = AF_LINK,
		.sdl_nlen = strlen(name),
	};

	if (strlen(name) <= sizeof(sdl.sdl_data))
	{
		memcpy(sdl.sdl_data, name, sdl.sdl_nlen);
		memcpy((char*)hdr + hdr->rtm_msglen, &sdl, sdl.sdl_len);
		hdr->rtm_msglen += SA_LEN(sdl.sdl_len);
		hdr->rtm_addrs |= type;
	}
}

static status_t manage_route(private_kernel_pfroute_net_t *this, int op,
							 chunk_t dst_net, u_int8_t prefixlen,
							 host_t *gateway, char *if_name)
{
	struct {
		struct rt_msghdr hdr;
		char buf[sizeof(struct sockaddr_storage) * RTAX_MAX];
	} msg = {
		.hdr = {
			.rtm_version = RTM_VERSION,
			.rtm_type = op,
			.rtm_flags = RTF_UP | RTF_STATIC,
			.rtm_pid = this->pid,
			.rtm_seq = ref_get(&this->seq),
		},
	};
	host_t *dst;
	int type;

	if (prefixlen == 0 && dst_net.len)
	{
		status_t status;
		chunk_t half;

		half = chunk_clonea(dst_net);
		half.ptr[0] |= 0x80;
		prefixlen = 1;
		status = manage_route(this, op, half, prefixlen, gateway, if_name);
		if (status != SUCCESS)
		{
			return status;
		}
	}

	dst = host_create_from_chunk(AF_UNSPEC, dst_net, 0);
	if (!dst)
	{
		return FAILED;
	}

	if ((dst->get_family(dst) == AF_INET && prefixlen == 32) ||
		(dst->get_family(dst) == AF_INET6 && prefixlen == 128))
	{
		msg.hdr.rtm_flags |= RTF_HOST | RTF_GATEWAY;
	}

	msg.hdr.rtm_msglen = sizeof(struct rt_msghdr);
	for (type = 0; type < RTAX_MAX; type++)
	{
		switch (type)
		{
			case RTAX_DST:
				add_rt_addr(&msg.hdr, RTA_DST, dst);
				break;
			case RTAX_NETMASK:
				if (!(msg.hdr.rtm_flags & RTF_HOST))
				{
					add_rt_mask(&msg.hdr, RTA_NETMASK,
								dst->get_family(dst), prefixlen);
				}
				break;
			case RTAX_IFP:
				if (if_name)
				{
					add_rt_ifname(&msg.hdr, RTA_IFP, if_name);
				}
				break;
			case RTAX_GATEWAY:
				if (gateway)
				{
					add_rt_addr(&msg.hdr, RTA_GATEWAY, gateway);
				}
				break;
			default:
				break;
		}
	}
	dst->destroy(dst);

	if (send(this->socket, &msg, msg.hdr.rtm_msglen, 0) != msg.hdr.rtm_msglen)
	{
		if (errno == EEXIST)
		{
			return ALREADY_DONE;
		}
		DBG1(DBG_KNL, "%s PF_ROUTE route failed: %s",
			 op == RTM_ADD ? "adding" : "deleting", strerror(errno));
		return FAILED;
	}
	return SUCCESS;
}

METHOD(kernel_net_t, add_route, status_t,
	private_kernel_pfroute_net_t *this, chunk_t dst_net, u_int8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	status_t status;
	route_entry_t *found, route = {
		.dst_net = dst_net,
		.prefixlen = prefixlen,
		.gateway = gateway,
		.if_name = if_name,
	};

	this->routes_lock->lock(this->routes_lock);
	found = this->routes->get(this->routes, &route);
	if (found)
	{
		this->routes_lock->unlock(this->routes_lock);
		return ALREADY_DONE;
	}
	status = manage_route(this, RTM_ADD, dst_net, prefixlen, gateway, if_name);
	if (status == SUCCESS)
	{
		found = route_entry_clone(&route);
		this->routes->put(this->routes, found, found);
	}
	this->routes_lock->unlock(this->routes_lock);
	return status;
}

METHOD(kernel_net_t, del_route, status_t,
	private_kernel_pfroute_net_t *this, chunk_t dst_net, u_int8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	status_t status;
	route_entry_t *found, route = {
		.dst_net = dst_net,
		.prefixlen = prefixlen,
		.gateway = gateway,
		.if_name = if_name,
	};

	this->routes_lock->lock(this->routes_lock);
	found = this->routes->get(this->routes, &route);
	if (!found)
	{
		this->routes_lock->unlock(this->routes_lock);
		return NOT_FOUND;
	}
	this->routes->remove(this->routes, found);
	route_entry_destroy(found);
	status = manage_route(this, RTM_DELETE, dst_net, prefixlen, gateway,
						  if_name);
	this->routes_lock->unlock(this->routes_lock);
	return status;
}

static host_t *get_route(private_kernel_pfroute_net_t *this, bool nexthop,
						 host_t *dest, host_t *src)
{
	struct {
		struct rt_msghdr hdr;
		char buf[sizeof(struct sockaddr_storage) * RTAX_MAX];
	} msg = {
		.hdr = {
			.rtm_version = RTM_VERSION,
			.rtm_type = RTM_GET,
			.rtm_pid = this->pid,
			.rtm_seq = ref_get(&this->seq),
		},
	};
	host_t *host = NULL;
	enumerator_t *enumerator;
	struct sockaddr *addr;
	bool failed = FALSE;
	int type;

retry:
	msg.hdr.rtm_msglen = sizeof(struct rt_msghdr);
	for (type = 0; type < RTAX_MAX; type++)
	{
		switch (type)
		{
			case RTAX_DST:
				add_rt_addr(&msg.hdr, RTA_DST, dest);
				break;
			case RTAX_IFA:
				add_rt_addr(&msg.hdr, RTA_IFA, src);
				break;
			case RTAX_IFP:
				if (!nexthop)
				{	
					add_rt_ifname(&msg.hdr, RTA_IFP, "");
				}
				break;
			default:
				break;
		}
	}
	this->mutex->lock(this->mutex);

	while (this->waiting_seq)
	{
		this->condvar->wait(this->condvar, this->mutex);
	}
	this->waiting_seq = msg.hdr.rtm_seq;
	if (send(this->socket, &msg, msg.hdr.rtm_msglen, 0) == msg.hdr.rtm_msglen)
	{
		while (TRUE)
		{
			if (this->condvar->timed_wait(this->condvar, this->mutex, 1000))
			{	
				break;
			}
			if (this->reply->rtm_msglen < sizeof(*this->reply) ||
				msg.hdr.rtm_seq != this->reply->rtm_seq)
			{
				continue;
			}
			enumerator = create_rtmsg_enumerator(this->reply);
			while (enumerator->enumerate(enumerator, &type, &addr))
			{
				if (nexthop)
				{
					if (type == RTAX_DST && this->reply->rtm_flags & RTF_HOST)
					{	
						host = host ?: host_create_from_sockaddr(addr);
					}
					if (type == RTAX_GATEWAY)
					{	
						host_t *gtw = host_create_from_sockaddr(addr);
						if (gtw)
						{
							DESTROY_IF(host);
							host = gtw;
						}
					}
				}
				else
				{
					if (type == RTAX_IFA)
					{
						host = host_create_from_sockaddr(addr);
					}
				}
			}
			enumerator->destroy(enumerator);
			break;
		}
	}
	else
	{
		failed = TRUE;
	}
	
	this->waiting_seq = 0;
	this->condvar->signal(this->condvar);
	this->mutex->unlock(this->mutex);

	if (failed)
	{
		if (src)
		{	
			src = NULL;
			msg.hdr.rtm_seq = ref_get(&this->seq);
			msg.hdr.rtm_addrs = 0;
			memset(msg.buf, 0, sizeof(msg.buf));
			goto retry;
		}
		DBG1(DBG_KNL, "PF_ROUTE lookup failed: %s", strerror(errno));
	}
	if (!host)
	{
		return NULL;
	}
	if (!nexthop)
	{	
		addr_entry_t *entry, lookup = {
			.ip = host,
		};

		this->lock->read_lock(this->lock);
		entry = this->addrs->get_match(this->addrs, &lookup,
									(void*)addr_map_entry_match_up_and_usable);
		this->lock->unlock(this->lock);
		if (!entry)
		{
			host->destroy(host);
			return NULL;
		}
	}
	DBG2(DBG_KNL, "using %H as %s to reach %H", host,
		 nexthop ? "nexthop" : "address", dest);
	return host;
}

METHOD(kernel_net_t, get_source_addr, host_t*,
	private_kernel_pfroute_net_t *this, host_t *dest, host_t *src)
{
	return get_route(this, FALSE, dest, src);
}

METHOD(kernel_net_t, get_nexthop, host_t*,
	private_kernel_pfroute_net_t *this, host_t *dest, host_t *src)
{
	return get_route(this, TRUE, dest, src);
}

static status_t init_address_list(private_kernel_pfroute_net_t *this)
{
	struct ifaddrs *ifap, *ifa;
	iface_entry_t *iface, *current;
	addr_entry_t *addr;
	enumerator_t *ifaces, *addrs;

	DBG2(DBG_KNL, "known interfaces and IP addresses:");

	if (getifaddrs(&ifap) < 0)
	{
		DBG1(DBG_KNL, "  failed to get interfaces!");
		return FAILED;
	}

	for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
		{
			continue;
		}
		switch(ifa->ifa_addr->sa_family)
		{
			case AF_LINK:
			case AF_INET:
			case AF_INET6:
			{
				iface = NULL;
				ifaces = this->ifaces->create_enumerator(this->ifaces);
				while (ifaces->enumerate(ifaces, &current))
				{
					if (streq(current->ifname, ifa->ifa_name))
					{
						iface = current;
						break;
					}
				}
				ifaces->destroy(ifaces);

				if (!iface)
				{
					INIT(iface,
						.ifindex = if_nametoindex(ifa->ifa_name),
						.flags = ifa->ifa_flags,
						.addrs = linked_list_create(),
						.usable = hydra->kernel_interface->is_interface_usable(
										hydra->kernel_interface, ifa->ifa_name),
					);
					memcpy(iface->ifname, ifa->ifa_name, IFNAMSIZ);
					this->ifaces->insert_last(this->ifaces, iface);
				}

				if (ifa->ifa_addr->sa_family != AF_LINK)
				{
					INIT(addr,
						.ip = host_create_from_sockaddr(ifa->ifa_addr),
					);
					iface->addrs->insert_last(iface->addrs, addr);
					addr_map_entry_add(this, addr, iface);
				}
			}
		}
	}
	freeifaddrs(ifap);

	ifaces = this->ifaces->create_enumerator(this->ifaces);
	while (ifaces->enumerate(ifaces, &iface))
	{
		if (iface->usable && iface->flags & IFF_UP)
		{
			DBG2(DBG_KNL, "  %s", iface->ifname);
			addrs = iface->addrs->create_enumerator(iface->addrs);
			while (addrs->enumerate(addrs, (void**)&addr))
			{
				DBG2(DBG_KNL, "    %H", addr->ip);
			}
			addrs->destroy(addrs);
		}
	}
	ifaces->destroy(ifaces);

	return SUCCESS;
}

METHOD(kernel_net_t, destroy, void,
	private_kernel_pfroute_net_t *this)
{
	enumerator_t *enumerator;
	route_entry_t *route;
	addr_entry_t *addr;

	enumerator = this->routes->create_enumerator(this->routes);
	while (enumerator->enumerate(enumerator, NULL, (void**)&route))
	{
		manage_route(this, RTM_DELETE, route->dst_net, route->prefixlen,
					 route->gateway, route->if_name);
		route_entry_destroy(route);
	}
	enumerator->destroy(enumerator);
	this->routes->destroy(this->routes);
	this->routes_lock->destroy(this->routes_lock);

	if (this->socket != -1)
	{
		lib->watcher->remove(lib->watcher, this->socket);
		close(this->socket);
	}

	net_changes_clear(this);
	this->net_changes->destroy(this->net_changes);
	this->net_changes_lock->destroy(this->net_changes_lock);

	enumerator = this->addrs->create_enumerator(this->addrs);
	while (enumerator->enumerate(enumerator, NULL, (void**)&addr))
	{
		free(addr);
	}
	enumerator->destroy(enumerator);
	this->addrs->destroy(this->addrs);
	this->ifaces->destroy_function(this->ifaces, (void*)iface_entry_destroy);
	this->tuns->destroy(this->tuns);
	this->lock->destroy(this->lock);
	this->mutex->destroy(this->mutex);
	this->condvar->destroy(this->condvar);
	this->roam_lock->destroy(this->roam_lock);
	free(this->reply);
	free(this);
}

kernel_pfroute_net_t *kernel_pfroute_net_create()
{
	private_kernel_pfroute_net_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_features = _get_features,
				.get_interface = _get_interface_name,
				.create_address_enumerator = _create_address_enumerator,
				.get_source_addr = _get_source_addr,
				.get_nexthop = _get_nexthop,
				.add_ip = _add_ip,
				.del_ip = _del_ip,
				.add_route = _add_route,
				.del_route = _del_route,
				.destroy = _destroy,
			},
		},
		.pid = getpid(),
		.ifaces = linked_list_create(),
		.addrs = hashtable_create(
								(hashtable_hash_t)addr_map_entry_hash,
								(hashtable_equals_t)addr_map_entry_equals, 16),
		.routes = hashtable_create((hashtable_hash_t)route_entry_hash,
								   (hashtable_equals_t)route_entry_equals, 16),
		.net_changes = hashtable_create(
								   (hashtable_hash_t)net_change_hash,
								   (hashtable_equals_t)net_change_equals, 16),
		.tuns = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.routes_lock = mutex_create(MUTEX_TYPE_DEFAULT),
		.net_changes_lock = mutex_create(MUTEX_TYPE_DEFAULT),
		.roam_lock = spinlock_create(),
		.vip_wait = lib->settings->get_int(lib->settings,
						"%s.plugins.kernel-pfroute.vip_wait", 1000, lib->ns),
	);
	timerclear(&this->last_route_reinstall);
	timerclear(&this->next_roam);

	
	this->socket = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
	if (this->socket == -1)
	{
		DBG1(DBG_KNL, "unable to create PF_ROUTE socket");
		destroy(this);
		return NULL;
	}

	if (streq(lib->ns, "starter"))
	{
		
		if (shutdown(this->socket, SHUT_RD) != 0)
		{
			DBG1(DBG_KNL, "closing read end of PF_ROUTE socket failed: %s",
				 strerror(errno));
		}
	}
	else
	{
		lib->watcher->add(lib->watcher, this->socket, WATCHER_READ,
						  (watcher_cb_t)receive_events, this);
	}
	if (init_address_list(this) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to get interface list");
		destroy(this);
		return NULL;
	}

	return &this->public;
}
