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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "pfkeyv2.h"
#include <linux/udp.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "kernel_klips_ipsec.h"

#include <hydra.h>
#include <utils/debug.h>
#include <collections/linked_list.h>
#include <threading/thread.h>
#include <threading/mutex.h>
#include <processing/jobs/callback_job.h>

#define SPI_TIMEOUT 30

#define PFKEY_BUFFER_SIZE 2048

#define PFKEY_ALIGNMENT 8
#define PFKEY_ALIGN(len) (((len) + PFKEY_ALIGNMENT - 1) & ~(PFKEY_ALIGNMENT - 1))
#define PFKEY_LEN(len) ((PFKEY_ALIGN(len) / PFKEY_ALIGNMENT))
#define PFKEY_USER_LEN(len) ((len) * PFKEY_ALIGNMENT)

#define PFKEY_EXT_ADD(msg, ext) ((msg)->sadb_msg_len += ((struct sadb_ext*)ext)->sadb_ext_len)
#define PFKEY_EXT_ADD_NEXT(msg) ((struct sadb_ext*)(((char*)(msg)) + PFKEY_USER_LEN((msg)->sadb_msg_len)))
#define PFKEY_EXT_COPY(msg, ext) (PFKEY_EXT_ADD(msg, memcpy(PFKEY_EXT_ADD_NEXT(msg), ext, PFKEY_USER_LEN(((struct sadb_ext*)ext)->sadb_ext_len))))
#define PFKEY_EXT_NEXT(ext) ((struct sadb_ext*)(((char*)(ext)) + PFKEY_USER_LEN(((struct sadb_ext*)ext)->sadb_ext_len)))
#define PFKEY_EXT_NEXT_LEN(ext,len) ((len) -= (ext)->sadb_ext_len, PFKEY_EXT_NEXT(ext))
#define PFKEY_EXT_OK(ext,len) ((len) >= PFKEY_LEN(sizeof(struct sadb_ext)) && \
				(ext)->sadb_ext_len >= PFKEY_LEN(sizeof(struct sadb_ext)) && \
				(ext)->sadb_ext_len <= (len))

#define SPI_PASS 256
#define SPI_DROP 257
#define SPI_REJECT 258
#define SPI_HOLD 259
#define SPI_TRAP 260
#define SPI_TRAPSUBNET 261

#define IPSEC_DEV_PREFIX "ipsec"
#define DEFAULT_IPSEC_DEV_COUNT 4
#define IS_IPSEC_DEV(name) (strpfx((name), IPSEC_DEV_PREFIX))

struct ipsectunnelconf
{
	__u32	cf_cmd;
	union
	{
		char	cfu_name[12];
	} cf_u;
#define cf_name cf_u.cfu_name
};

#define IPSEC_SET_DEV (SIOCDEVPRIVATE)
#define IPSEC_DEL_DEV (SIOCDEVPRIVATE + 1)
#define IPSEC_CLR_DEV (SIOCDEVPRIVATE + 2)

typedef struct private_kernel_klips_ipsec_t private_kernel_klips_ipsec_t;

struct private_kernel_klips_ipsec_t
{
	kernel_klips_ipsec_t public;

	mutex_t *mutex;

	linked_list_t *policies;

	linked_list_t *allocated_spis;

	linked_list_t *installed_sas;

	bool install_routes;

	linked_list_t *ipsec_devices;

	mutex_t *mutex_pfkey;

	int socket;

	int socket_events;

	int seq;

};


typedef struct ipsec_dev_t ipsec_dev_t;

struct ipsec_dev_t {
	
	char name[IFNAMSIZ];

	
	char phys_name[IFNAMSIZ];

	
	u_int refcount;
};

static inline bool ipsec_dev_match_byname(ipsec_dev_t *current, char *name)
{
	return name && streq(current->name, name);
}

static inline bool ipsec_dev_match_byphys(ipsec_dev_t *current, char *name)
{
	return name && streq(current->phys_name, name);
}

static inline bool ipsec_dev_match_free(ipsec_dev_t *current)
{
	return current->refcount == 0;
}

static status_t find_ipsec_dev(private_kernel_klips_ipsec_t *this, char *name,
							   ipsec_dev_t **dev)
{
	linked_list_match_t match = (linked_list_match_t)(IS_IPSEC_DEV(name) ?
								ipsec_dev_match_byname : ipsec_dev_match_byphys);
	return this->ipsec_devices->find_first(this->ipsec_devices, match,
												(void**)dev, name);
}

static status_t attach_ipsec_dev(char* name, char *phys_name)
{
	int sock;
	struct ifreq req;
	struct ipsectunnelconf *itc = (struct ipsectunnelconf*)&req.ifr_data;
	short phys_flags;
	int mtu;

	DBG2(DBG_KNL, "attaching virtual interface %s to %s", name, phys_name);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
	{
		return FAILED;
	}

	strncpy(req.ifr_name, phys_name, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFFLAGS, &req) < 0)
	{
		close(sock);
		return FAILED;
	}
	phys_flags = req.ifr_flags;

	strncpy(req.ifr_name, name, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFFLAGS, &req) < 0)
	{
		close(sock);
		return FAILED;
	}

	if (req.ifr_flags & IFF_UP)
	{
		
		ioctl(sock, IPSEC_DEL_DEV, &req);
	}

	
	strncpy(req.ifr_name, name, IFNAMSIZ);
	strncpy(itc->cf_name, phys_name, sizeof(itc->cf_name));
	ioctl(sock, IPSEC_SET_DEV, &req);

	
	strncpy(req.ifr_name, phys_name, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFADDR, &req) == 0)
	{
		strncpy(req.ifr_name, name, IFNAMSIZ);
		ioctl(sock, SIOCSIFADDR, &req);
	}

	
	strncpy(req.ifr_name, phys_name, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFNETMASK, &req) == 0)
	{
		strncpy(req.ifr_name, name, IFNAMSIZ);
		ioctl(sock, SIOCSIFNETMASK, &req);
	}

	
	strncpy(req.ifr_name, name, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFFLAGS, &req) == 0)
	{
		if (phys_flags & IFF_POINTOPOINT)
		{
			req.ifr_flags |= IFF_POINTOPOINT;
			req.ifr_flags &= ~IFF_BROADCAST;
			ioctl(sock, SIOCSIFFLAGS, &req);

			strncpy(req.ifr_name, phys_name, IFNAMSIZ);
			if (ioctl(sock, SIOCGIFDSTADDR, &req) == 0)
			{
				strncpy(req.ifr_name, name, IFNAMSIZ);
				ioctl(sock, SIOCSIFDSTADDR, &req);
			}
		}
		else if (phys_flags & IFF_BROADCAST)
		{
			req.ifr_flags &= ~IFF_POINTOPOINT;
			req.ifr_flags |= IFF_BROADCAST;
			ioctl(sock, SIOCSIFFLAGS, &req);

			strncpy(req.ifr_name, phys_name, IFNAMSIZ);
			if (ioctl(sock, SIOCGIFBRDADDR, &req)==0)
			{
				strncpy(req.ifr_name, name, IFNAMSIZ);
				ioctl(sock, SIOCSIFBRDADDR, &req);
			}
		}
		else
		{
			req.ifr_flags &= ~IFF_POINTOPOINT;
			req.ifr_flags &= ~IFF_BROADCAST;
			ioctl(sock, SIOCSIFFLAGS, &req);
		}
	}

	mtu = lib->settings->get_int(lib->settings,
								 "%s.plugins.kernel-klips.ipsec_dev_mtu", 0,
								 lib->ns);
	if (mtu <= 0)
	{
		strncpy(req.ifr_name, phys_name, IFNAMSIZ);
		ioctl(sock, SIOCGIFMTU, &req);
		mtu = req.ifr_mtu - 81;
	}

	
	strncpy(req.ifr_name, name, IFNAMSIZ);
	req.ifr_mtu = mtu;
	ioctl(sock, SIOCSIFMTU, &req);

	
	if (ioctl(sock, SIOCGIFFLAGS, &req) == 0)
	{
		req.ifr_flags |= IFF_UP;
		ioctl(sock, SIOCSIFFLAGS, &req);
	}

	close(sock);
	return SUCCESS;
}

static status_t detach_ipsec_dev(char* name, char *phys_name)
{
	int sock;
	struct ifreq req;

	DBG2(DBG_KNL, "detaching virtual interface %s from %s", name,
			strlen(phys_name) ? phys_name : "any physical interface");

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
	{
		return FAILED;
	}

	strncpy(req.ifr_name, name, IFNAMSIZ);
	if (ioctl(sock, SIOCGIFFLAGS, &req) < 0)
	{
		close(sock);
		return FAILED;
	}

	
	if (req.ifr_flags & IFF_UP)
	{
		req.ifr_flags &= ~IFF_UP;
		ioctl(sock, SIOCSIFFLAGS, &req);
	}

	
	memset(&req.ifr_addr, 0, sizeof(req.ifr_addr));
	req.ifr_addr.sa_family = AF_INET;
	ioctl(sock, SIOCSIFADDR, &req);

	
	ioctl(sock, IPSEC_DEL_DEV, &req);

	close(sock);
	return SUCCESS;
}

static void ipsec_dev_destroy(ipsec_dev_t *this)
{
	detach_ipsec_dev(this->name, this->phys_name);
	free(this);
}


typedef struct route_entry_t route_entry_t;

struct route_entry_t {
	
	char *if_name;

	
	host_t *src_ip;

	
	host_t *gateway;

	
	chunk_t dst_net;

	
	u_int8_t prefixlen;
};

static void route_entry_destroy(route_entry_t *this)
{
	free(this->if_name);
	this->src_ip->destroy(this->src_ip);
	this->gateway->destroy(this->gateway);
	chunk_free(&this->dst_net);
	free(this);
}

typedef struct policy_entry_t policy_entry_t;

struct policy_entry_t {

	
	u_int32_t reqid;

	
	u_int8_t direction;

	
	struct {
		
		host_t *net;
		
		u_int8_t mask;
		
		u_int8_t proto;
	} src, dst;

	
	route_entry_t *route;

	
	u_int activecount;

	
	u_int trapcount;
};

static host_t *mask2host(int family, u_int8_t mask)
{
	static const u_char bitmask[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };
	chunk_t chunk = chunk_alloca(family == AF_INET ? 4 : 16);
	int bytes = mask / 8, bits = mask % 8;
	memset(chunk.ptr, 0xFF, bytes);
	memset(chunk.ptr + bytes, 0, chunk.len - bytes);
	if (bits)
	{
		chunk.ptr[bytes] =  bitmask[bits];
	}
	return host_create_from_chunk(family, chunk, 0);
}

static bool is_host_in_net(host_t *host, host_t *net, u_int8_t mask)
{
	static const u_char bitmask[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };
	chunk_t host_chunk, net_chunk;
	int bytes = mask / 8, bits = mask % 8;

	host_chunk = host->get_address(host);
	net_chunk = net->get_address(net);

	if (host_chunk.len != net_chunk.len)
	{
		return FALSE;
	}

	if (memeq(host_chunk.ptr, net_chunk.ptr, bytes))
	{
		return (bits == 0) ||
			   (host_chunk.ptr[bytes] & bitmask[bits]) ==
				   (net_chunk.ptr[bytes] & bitmask[bits]);
	}

	return FALSE;
}

static policy_entry_t *create_policy_entry(traffic_selector_t *src_ts,
		traffic_selector_t *dst_ts, policy_dir_t dir)
{
	policy_entry_t *policy = malloc_thing(policy_entry_t);
	policy->reqid = 0;
	policy->direction = dir;
	policy->route = NULL;
	policy->activecount = 0;
	policy->trapcount = 0;

	src_ts->to_subnet(src_ts, &policy->src.net, &policy->src.mask);
	dst_ts->to_subnet(dst_ts, &policy->dst.net, &policy->dst.mask);

	
	policy->src.proto = max(src_ts->get_protocol(src_ts), dst_ts->get_protocol(dst_ts));
	policy->src.proto = policy->src.proto ? policy->src.proto : 0;
	policy->dst.proto = policy->src.proto;

	return policy;
}

static void policy_entry_destroy(policy_entry_t *this)
{
	DESTROY_IF(this->src.net);
	DESTROY_IF(this->dst.net);
	if (this->route)
	{
		route_entry_destroy(this->route);
	}
	free(this);
}

static inline bool policy_entry_equals(policy_entry_t *current, policy_entry_t *policy)
{
	return current->direction == policy->direction &&
		   current->src.proto == policy->src.proto &&
		   current->dst.proto == policy->dst.proto &&
		   current->src.mask == policy->src.mask &&
		   current->dst.mask == policy->dst.mask &&
		   current->src.net->equals(current->src.net, policy->src.net) &&
		   current->dst.net->equals(current->dst.net, policy->dst.net);
}

static inline bool policy_entry_match_byaddrs(policy_entry_t *current, host_t *src,
		host_t *dst)
{
	return is_host_in_net(src, current->src.net, current->src.mask) &&
			is_host_in_net(dst, current->dst.net, current->dst.mask);
}

typedef struct sa_entry_t sa_entry_t;

struct sa_entry_t {

	
	u_int8_t protocol;

	
	u_int32_t reqid;

	
	u_int32_t spi;

	
	host_t *src;

	
	host_t *dst;

	
	bool encap;

	
	bool inbound;
};

static sa_entry_t *create_sa_entry(u_int8_t protocol, u_int32_t spi,
								   u_int32_t reqid, host_t *src, host_t *dst,
								   bool encap, bool inbound)
{
	sa_entry_t *sa = malloc_thing(sa_entry_t);
	sa->protocol = protocol;
	sa->reqid = reqid;
	sa->spi = spi;
	sa->src = src ? src->clone(src) : NULL;
	sa->dst = dst ? dst->clone(dst) : NULL;
	sa->encap = encap;
	sa->inbound = inbound;
	return sa;
}

static void sa_entry_destroy(sa_entry_t *this)
{
	DESTROY_IF(this->src);
	DESTROY_IF(this->dst);
	free(this);
}

static inline bool sa_entry_match_encapbysrc(sa_entry_t *current, u_int32_t *spi,
		host_t *src)
{
	return current->encap && current->inbound &&
		   current->spi == *spi && src->ip_equals(src, current->src);
}

static inline bool sa_entry_match_bydst(sa_entry_t *current, u_int8_t *protocol,
		u_int32_t *spi, host_t *dst)
{
	return current->protocol == *protocol && current->spi == *spi && dst->ip_equals(dst, current->dst);
}

static inline bool sa_entry_match_byid(sa_entry_t *current, u_int8_t *protocol,
		u_int32_t *spi, u_int32_t *reqid)
{
	return current->protocol == *protocol && current->spi == *spi && current->reqid == *reqid;
}

typedef struct pfkey_msg_t pfkey_msg_t;

struct pfkey_msg_t
{
	struct sadb_msg *msg;


	union {
		struct sadb_ext *ext[SADB_EXT_MAX + 1];
		struct {
			struct sadb_ext *reserved;				
			struct sadb_sa *sa;						
			struct sadb_lifetime *lft_current;		
			struct sadb_lifetime *lft_hard;			
			struct sadb_lifetime *lft_soft;			
			struct sadb_address *src;				
			struct sadb_address *dst;				
			struct sadb_address *proxy;				
			struct sadb_key *key_auth;				
			struct sadb_key *key_encr;				
			struct sadb_ident *id_src;				
			struct sadb_ident *id_dst;				
			struct sadb_sens *sensitivity;			
			struct sadb_prop *proposal;				
			struct sadb_supported *supported_auth;	
			struct sadb_supported *supported_encr;	
			struct sadb_spirange *spirange;			
			struct sadb_x_kmprivate *x_kmprivate;	
			struct sadb_ext *x_policy;				
			struct sadb_ext *x_sa2;					
			struct sadb_address *x_dst2;			
			struct sadb_address *x_src_flow;		
			struct sadb_address *x_dst_flow;		
			struct sadb_address *x_src_mask;		
			struct sadb_address *x_dst_mask;		
			struct sadb_x_debug *x_debug;			
			struct sadb_protocol *x_protocol;		
			struct sadb_x_nat_t_type *x_natt_type;	
			struct sadb_x_nat_t_port *x_natt_sport;	
			struct sadb_x_nat_t_port *x_natt_dport;	
			struct sadb_address *x_natt_oa;			
		} __attribute__((__packed__));
	};
};

static u_int8_t proto2satype(u_int8_t proto)
{
	switch (proto)
	{
		case IPPROTO_ESP:
			return SADB_SATYPE_ESP;
		case IPPROTO_AH:
			return SADB_SATYPE_AH;
		case IPPROTO_COMP:
			return SADB_X_SATYPE_COMP;
		default:
			return proto;
	}
}

static u_int8_t satype2proto(u_int8_t satype)
{
	switch (satype)
	{
		case SADB_SATYPE_ESP:
			return IPPROTO_ESP;
		case SADB_SATYPE_AH:
			return IPPROTO_AH;
		case SADB_X_SATYPE_COMP:
			return IPPROTO_COMP;
		default:
			return satype;
	}
}

typedef struct kernel_algorithm_t kernel_algorithm_t;

struct kernel_algorithm_t {
	int ikev2;

	int kernel;
};

#define END_OF_LIST -1

static kernel_algorithm_t encryption_algs[] = {
	{ENCR_DES,					SADB_EALG_DESCBC			},
	{ENCR_3DES,					SADB_EALG_3DESCBC			},
	{ENCR_BLOWFISH,				SADB_EALG_BFCBC				},
	{ENCR_NULL,					SADB_EALG_NULL				},
	{ENCR_AES_CBC,				SADB_EALG_AESCBC			},
	{END_OF_LIST,				0							},
};

static kernel_algorithm_t integrity_algs[] = {
	{AUTH_HMAC_MD5_96,			SADB_AALG_MD5HMAC			},
	{AUTH_HMAC_SHA1_96,			SADB_AALG_SHA1HMAC			},
	{AUTH_HMAC_SHA2_256_128,	SADB_AALG_SHA256_HMAC		},
	{AUTH_HMAC_SHA2_384_192,	SADB_AALG_SHA384_HMAC		},
	{AUTH_HMAC_SHA2_512_256,	SADB_AALG_SHA512_HMAC		},
	{END_OF_LIST,				0,							},
};

#if 0
static kernel_algorithm_t compression_algs[] = {
	{IPCOMP_DEFLATE,			SADB_X_CALG_DEFLATE			},
	{IPCOMP_LZS,				SADB_X_CALG_LZS				},
	{END_OF_LIST,				0							},
};
#endif

static int lookup_algorithm(transform_type_t type, int ikev2)
{
	kernel_algorithm_t *list;
	int alg = 0;

	switch (type)
	{
		case ENCRYPTION_ALGORITHM:
			list = encryption_algs;
			break;
		case INTEGRITY_ALGORITHM:
			list = integrity_algs;
			break;
		default:
			return 0;
	}
	while (list->ikev2 != END_OF_LIST)
	{
		if (ikev2 == list->ikev2)
		{
			return list->kernel;
		}
		list++;
	}
	hydra->kernel_interface->lookup_algorithm(hydra->kernel_interface, ikev2,
											  type, &alg, NULL);
	return alg;
}

static void host2ext(host_t *host, struct sadb_address *ext)
{
	sockaddr_t *host_addr = host->get_sockaddr(host);
	socklen_t *len = host->get_sockaddr_len(host);
	memcpy((char*)(ext + 1), host_addr, *len);
	ext->sadb_address_len = PFKEY_LEN(sizeof(*ext) + *len);
}

static void add_addr_ext(struct sadb_msg *msg, host_t *host, u_int16_t type)
{
	struct sadb_address *addr = (struct sadb_address*)PFKEY_EXT_ADD_NEXT(msg);
	addr->sadb_address_exttype = type;
	host2ext(host, addr);
	PFKEY_EXT_ADD(msg, addr);
}

static void add_anyaddr_ext(struct sadb_msg *msg, int family, u_int8_t type)
{
	socklen_t len = (family == AF_INET) ? sizeof(struct sockaddr_in) :
										  sizeof(struct sockaddr_in6);
	struct sadb_address *addr = (struct sadb_address*)PFKEY_EXT_ADD_NEXT(msg);
	addr->sadb_address_exttype = type;
	sockaddr_t *saddr = (sockaddr_t*)(addr + 1);
	saddr->sa_family = family;
	addr->sadb_address_len = PFKEY_LEN(sizeof(*addr) + len);
	PFKEY_EXT_ADD(msg, addr);
}

static void add_encap_ext(struct sadb_msg *msg, host_t *src, host_t *dst,
							bool ports_only)
{
	struct sadb_x_nat_t_type* nat_type;
	struct sadb_x_nat_t_port* nat_port;

	if (!ports_only)
	{
		nat_type = (struct sadb_x_nat_t_type*)PFKEY_EXT_ADD_NEXT(msg);
		nat_type->sadb_x_nat_t_type_exttype = SADB_X_EXT_NAT_T_TYPE;
		nat_type->sadb_x_nat_t_type_len = PFKEY_LEN(sizeof(struct sadb_x_nat_t_type));
		nat_type->sadb_x_nat_t_type_type = UDP_ENCAP_ESPINUDP;
		PFKEY_EXT_ADD(msg, nat_type);
	}

	nat_port = (struct sadb_x_nat_t_port*)PFKEY_EXT_ADD_NEXT(msg);
	nat_port->sadb_x_nat_t_port_exttype = SADB_X_EXT_NAT_T_SPORT;
	nat_port->sadb_x_nat_t_port_len = PFKEY_LEN(sizeof(struct sadb_x_nat_t_port));
	nat_port->sadb_x_nat_t_port_port = src->get_port(src);
	PFKEY_EXT_ADD(msg, nat_port);

	nat_port = (struct sadb_x_nat_t_port*)PFKEY_EXT_ADD_NEXT(msg);
	nat_port->sadb_x_nat_t_port_exttype = SADB_X_EXT_NAT_T_DPORT;
	nat_port->sadb_x_nat_t_port_len = PFKEY_LEN(sizeof(struct sadb_x_nat_t_port));
	nat_port->sadb_x_nat_t_port_port = dst->get_port(dst);
	PFKEY_EXT_ADD(msg, nat_port);
}

static void build_addflow(struct sadb_msg *msg, u_int8_t satype, u_int32_t spi,
		host_t *src, host_t *dst, host_t *src_net, u_int8_t src_mask,
		host_t *dst_net, u_int8_t dst_mask, u_int8_t protocol, bool replace)
{
	struct sadb_sa *sa;
	struct sadb_protocol *proto;
	host_t *host;

	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_X_ADDFLOW;
	msg->sadb_msg_satype = satype;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_spi = spi;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_flags = replace ? SADB_X_SAFLAGS_REPLACEFLOW : 0;
	PFKEY_EXT_ADD(msg, sa);

	if (!src)
	{
		add_anyaddr_ext(msg, src_net->get_family(src_net), SADB_EXT_ADDRESS_SRC);
	}
	else
	{
		add_addr_ext(msg, src, SADB_EXT_ADDRESS_SRC);
	}

	if (!dst)
	{
		add_anyaddr_ext(msg, dst_net->get_family(dst_net), SADB_EXT_ADDRESS_DST);
	}
	else
	{
		add_addr_ext(msg, dst, SADB_EXT_ADDRESS_DST);
	}

	add_addr_ext(msg, src_net, SADB_X_EXT_ADDRESS_SRC_FLOW);
	add_addr_ext(msg, dst_net, SADB_X_EXT_ADDRESS_DST_FLOW);

	host = mask2host(src_net->get_family(src_net), src_mask);
	add_addr_ext(msg, host, SADB_X_EXT_ADDRESS_SRC_MASK);
	host->destroy(host);

	host = mask2host(dst_net->get_family(dst_net), dst_mask);
	add_addr_ext(msg, host, SADB_X_EXT_ADDRESS_DST_MASK);
	host->destroy(host);

	proto = (struct sadb_protocol*)PFKEY_EXT_ADD_NEXT(msg);
	proto->sadb_protocol_exttype = SADB_X_EXT_PROTOCOL;
	proto->sadb_protocol_len = PFKEY_LEN(sizeof(struct sadb_protocol));
	proto->sadb_protocol_proto = protocol;
	PFKEY_EXT_ADD(msg, proto);
}

static void build_delflow(struct sadb_msg *msg, u_int8_t satype,
		host_t *src_net, u_int8_t src_mask, host_t *dst_net, u_int8_t dst_mask,
		u_int8_t protocol)
{
	struct sadb_protocol *proto;
	host_t *host;

	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_X_DELFLOW;
	msg->sadb_msg_satype = satype;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	add_addr_ext(msg, src_net, SADB_X_EXT_ADDRESS_SRC_FLOW);
	add_addr_ext(msg, dst_net, SADB_X_EXT_ADDRESS_DST_FLOW);

	host = mask2host(src_net->get_family(src_net),
					 src_mask);
	add_addr_ext(msg, host, SADB_X_EXT_ADDRESS_SRC_MASK);
	host->destroy(host);

	host = mask2host(dst_net->get_family(dst_net),
					 dst_mask);
	add_addr_ext(msg, host, SADB_X_EXT_ADDRESS_DST_MASK);
	host->destroy(host);

	proto = (struct sadb_protocol*)PFKEY_EXT_ADD_NEXT(msg);
	proto->sadb_protocol_exttype = SADB_X_EXT_PROTOCOL;
	proto->sadb_protocol_len = PFKEY_LEN(sizeof(struct sadb_protocol));
	proto->sadb_protocol_proto = protocol;
	PFKEY_EXT_ADD(msg, proto);
}

static status_t parse_pfkey_message(struct sadb_msg *msg, pfkey_msg_t *out)
{
	struct sadb_ext* ext;
	size_t len;

	memset(out, 0, sizeof(pfkey_msg_t));
	out->msg = msg;

	len = msg->sadb_msg_len;
	len -= PFKEY_LEN(sizeof(struct sadb_msg));

	ext = (struct sadb_ext*)(((char*)msg) + sizeof(struct sadb_msg));

	while (len >= PFKEY_LEN(sizeof(struct sadb_ext)))
	{
		if (ext->sadb_ext_len < PFKEY_LEN(sizeof(struct sadb_ext)) ||
			ext->sadb_ext_len > len)
		{
			DBG1(DBG_KNL, "length of PF_KEY extension (%d) is invalid", ext->sadb_ext_type);
			break;
		}

		if ((ext->sadb_ext_type > SADB_EXT_MAX) || (!ext->sadb_ext_type))
		{
			DBG1(DBG_KNL, "type of PF_KEY extension (%d) is invalid", ext->sadb_ext_type);
			break;
		}

		if (out->ext[ext->sadb_ext_type])
		{
			DBG1(DBG_KNL, "duplicate PF_KEY extension of type (%d)", ext->sadb_ext_type);
			break;
		}

		out->ext[ext->sadb_ext_type] = ext;
		ext = PFKEY_EXT_NEXT_LEN(ext, len);
	}

	if (len)
	{
		DBG1(DBG_KNL, "PF_KEY message length is invalid");
		return FAILED;
	}

	return SUCCESS;
}

static status_t pfkey_send_socket(private_kernel_klips_ipsec_t *this, int socket,
					struct sadb_msg *in, struct sadb_msg **out, size_t *out_len)
{
	unsigned char buf[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg;
	int in_len, len;

	this->mutex_pfkey->lock(this->mutex_pfkey);

	in->sadb_msg_seq = ++this->seq;
	in->sadb_msg_pid = getpid();

	in_len = PFKEY_USER_LEN(in->sadb_msg_len);

	while (TRUE)
	{
		len = send(socket, in, in_len, 0);

		if (len != in_len)
		{
			switch (errno)
			{
				case EINTR:
					
					continue;
				case EINVAL:
				case EEXIST:
				case ESRCH:
					
					break;
				default:
					this->mutex_pfkey->unlock(this->mutex_pfkey);
					DBG1(DBG_KNL, "error sending to PF_KEY socket: %s (%d)",
							strerror(errno), errno);
					return FAILED;
			}
		}
		break;
	}

	while (TRUE)
	{
		msg = (struct sadb_msg*)buf;

		len = recv(socket, buf, sizeof(buf), 0);

		if (len < 0)
		{
			if (errno == EINTR)
			{
				DBG1(DBG_KNL, "got interrupted");
				
				continue;
			}
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			DBG1(DBG_KNL, "error reading from PF_KEY socket: %s", strerror(errno));
			return FAILED;
		}
		if (len < sizeof(struct sadb_msg) ||
			msg->sadb_msg_len < PFKEY_LEN(sizeof(struct sadb_msg)))
		{
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			DBG1(DBG_KNL, "received corrupted PF_KEY message");
			return FAILED;
		}
		if (msg->sadb_msg_len > len / PFKEY_ALIGNMENT)
		{
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			DBG1(DBG_KNL, "buffer was too small to receive the complete PF_KEY message");
			return FAILED;
		}
		if (msg->sadb_msg_pid != in->sadb_msg_pid)
		{
			DBG2(DBG_KNL, "received PF_KEY message is not intended for us");
			continue;
		}
		if (msg->sadb_msg_seq != this->seq)
		{
			DBG1(DBG_KNL, "received PF_KEY message with invalid sequence number,"
					" was %d expected %d", msg->sadb_msg_seq, this->seq);
			if (msg->sadb_msg_seq < this->seq)
			{
				continue;
			}
			this->mutex_pfkey->unlock(this->mutex_pfkey);
			return FAILED;
		}
		if (msg->sadb_msg_type != in->sadb_msg_type)
		{
			DBG2(DBG_KNL, "received PF_KEY message of wrong type,"
					" was %d expected %d, ignoring",
					msg->sadb_msg_type, in->sadb_msg_type);
		}
		break;
	}

	*out_len = len;
	*out = (struct sadb_msg*)malloc(len);
	memcpy(*out, buf, len);

	this->mutex_pfkey->unlock(this->mutex_pfkey);

	return SUCCESS;
}

static status_t pfkey_send(private_kernel_klips_ipsec_t *this,
					struct sadb_msg *in, struct sadb_msg **out, size_t *out_len)
{
	return pfkey_send_socket(this, this->socket, in, out, out_len);
}

static status_t pfkey_send_ack(private_kernel_klips_ipsec_t *this, struct sadb_msg *in)
{
	struct sadb_msg *out;
	size_t len;

	if (pfkey_send(this, in, &out, &len) != SUCCESS)
	{
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "PF_KEY error: %s (%d)",
					   strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);
	return SUCCESS;
}

static status_t add_eroute(private_kernel_klips_ipsec_t *this, u_int8_t satype,
		u_int32_t spi, host_t *src, host_t *dst, host_t *src_net, u_int8_t src_mask,
		host_t *dst_net, u_int8_t dst_mask, u_int8_t protocol, bool replace)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg = (struct sadb_msg*)request;

	memset(&request, 0, sizeof(request));

	build_addflow(msg, satype, spi, src, dst, src_net, src_mask,
			dst_net, dst_mask, protocol, replace);

	return pfkey_send_ack(this, msg);
}

static status_t del_eroute(private_kernel_klips_ipsec_t *this, u_int8_t satype,
		host_t *src_net, u_int8_t src_mask, host_t *dst_net, u_int8_t dst_mask,
		u_int8_t protocol)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg = (struct sadb_msg*)request;

	memset(&request, 0, sizeof(request));

	build_delflow(msg, satype, src_net, src_mask, dst_net, dst_mask, protocol);

	return pfkey_send_ack(this, msg);
}

static void process_acquire(private_kernel_klips_ipsec_t *this, struct sadb_msg* msg)
{
	pfkey_msg_t response;
	host_t *src, *dst;
	u_int32_t reqid;
	u_int8_t proto;
	policy_entry_t *policy;

	switch (msg->sadb_msg_satype)
	{
		case SADB_SATYPE_UNSPEC:
		case SADB_SATYPE_ESP:
		case SADB_SATYPE_AH:
			break;
		default:
			
			return;
	}

	if (parse_pfkey_message(msg, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "parsing SADB_ACQUIRE from kernel failed");
		return;
	}

	src = host_create_from_sockaddr((sockaddr_t*)(response.src + 1));
	dst = host_create_from_sockaddr((sockaddr_t*)(response.dst + 1));
	proto = response.src->sadb_address_proto;
	if (!src || !dst || src->get_family(src) != dst->get_family(dst))
	{
		DBG1(DBG_KNL, "received an SADB_ACQUIRE with invalid hosts");
		return;
	}

	DBG2(DBG_KNL, "received an SADB_ACQUIRE for %H == %H : %d", src, dst, proto);
	this->mutex->lock(this->mutex);
	if (this->policies->find_first(this->policies,
			(linked_list_match_t)policy_entry_match_byaddrs,
				(void**)&policy, src, dst) != SUCCESS)
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_KNL, "received an SADB_ACQUIRE, but found no matching policy");
		return;
	}
	if ((reqid = policy->reqid) == 0)
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_KNL, "received an SADB_ACQUIRE, but policy is not routed anymore");
		return;
	}

	
	add_eroute(this, SADB_X_SATYPE_INT, htonl(SPI_HOLD), NULL, NULL,
			policy->src.net, policy->src.mask, policy->dst.net, policy->dst.mask,
			policy->src.proto, TRUE);

	
	del_eroute(this, SADB_X_SATYPE_INT, src, 32, dst, 32, proto);

	this->mutex->unlock(this->mutex);

	hydra->kernel_interface->acquire(hydra->kernel_interface, reqid, NULL,
									 NULL);
}

static void process_mapping(private_kernel_klips_ipsec_t *this, struct sadb_msg* msg)
{
	pfkey_msg_t response;
	u_int32_t spi, reqid;
	host_t *old_src, *new_src;

	DBG2(DBG_KNL, "received an SADB_X_NAT_T_NEW_MAPPING");

	if (parse_pfkey_message(msg, &response) != SUCCESS)
	{
		DBG1(DBG_KNL, "parsing SADB_X_NAT_T_NEW_MAPPING from kernel failed");
		return;
	}

	spi = response.sa->sadb_sa_spi;

	if (satype2proto(msg->sadb_msg_satype) == IPPROTO_ESP)
	{
		sa_entry_t *sa;
		sockaddr_t *addr = (sockaddr_t*)(response.src + 1);
		old_src = host_create_from_sockaddr(addr);

		this->mutex->lock(this->mutex);
		if (!old_src || this->installed_sas->find_first(this->installed_sas,
				(linked_list_match_t)sa_entry_match_encapbysrc,
					(void**)&sa, &spi, old_src) != SUCCESS)
		{
			this->mutex->unlock(this->mutex);
			DBG1(DBG_KNL, "received an SADB_X_NAT_T_NEW_MAPPING, but found no matching SA");
			return;
		}
		reqid = sa->reqid;
		this->mutex->unlock(this->mutex);

		addr = (sockaddr_t*)(response.dst + 1);
		switch (addr->sa_family)
		{
			case AF_INET:
			{
				struct sockaddr_in *sin = (struct sockaddr_in*)addr;
				sin->sin_port = htons(response.x_natt_dport->sadb_x_nat_t_port_port);
			}
			case AF_INET6:
			{
				struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)addr;
				sin6->sin6_port = htons(response.x_natt_dport->sadb_x_nat_t_port_port);
			}
			default:
				break;
		}
		new_src = host_create_from_sockaddr(addr);
		if (new_src)
		{
			hydra->kernel_interface->mapping(hydra->kernel_interface, reqid,
											 spi, new_src);
		}
	}
}

static job_requeue_t receive_events(private_kernel_klips_ipsec_t *this)
{
	unsigned char buf[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg = (struct sadb_msg*)buf;
	int len;
	bool oldstate;

	oldstate = thread_cancelability(TRUE);
	len = recv(this->socket_events, buf, sizeof(buf), 0);
	thread_cancelability(oldstate);

	if (len < 0)
	{
		switch (errno)
		{
			case EINTR:
				
				return JOB_REQUEUE_DIRECT;
			case EAGAIN:
				
				return JOB_REQUEUE_DIRECT;
			default:
				DBG1(DBG_KNL, "unable to receive from PF_KEY event socket");
				sleep(1);
				return JOB_REQUEUE_FAIR;
		}
	}

	if (len < sizeof(struct sadb_msg) ||
		msg->sadb_msg_len < PFKEY_LEN(sizeof(struct sadb_msg)))
	{
		DBG2(DBG_KNL, "received corrupted PF_KEY message");
		return JOB_REQUEUE_DIRECT;
	}
	if (msg->sadb_msg_pid != 0)
	{	
		return JOB_REQUEUE_DIRECT;
	}
	if (msg->sadb_msg_len > len / PFKEY_ALIGNMENT)
	{
		DBG1(DBG_KNL, "buffer was too small to receive the complete PF_KEY message");
		return JOB_REQUEUE_DIRECT;
	}

	switch (msg->sadb_msg_type)
	{
		case SADB_ACQUIRE:
			process_acquire(this, msg);
			break;
		case SADB_EXPIRE:
			break;
		case SADB_X_NAT_T_NEW_MAPPING:
			process_mapping(this, msg);
			break;
		default:
			break;
	}

	return JOB_REQUEUE_DIRECT;
}

typedef enum {
	
	EXPIRE_TYPE_SPI,
	
	EXPIRE_TYPE_SOFT,
	
	EXPIRE_TYPE_HARD
} expire_type_t;

typedef struct sa_expire_t sa_expire_t;

struct sa_expire_t {
	
	private_kernel_klips_ipsec_t *this;
	
	u_int32_t spi;
	
	u_int8_t protocol;
	
	u_int32_t reqid;
	
	expire_type_t type;
};

static job_requeue_t sa_expires(sa_expire_t *expire)
{
	private_kernel_klips_ipsec_t *this = expire->this;
	u_int8_t protocol = expire->protocol;
	u_int32_t spi = expire->spi, reqid = expire->reqid;
	bool hard = expire->type != EXPIRE_TYPE_SOFT;
	sa_entry_t *cached_sa;
	linked_list_t *list;

	list = expire->type == EXPIRE_TYPE_SPI ? this->allocated_spis : this->installed_sas;

	this->mutex->lock(this->mutex);
	if (list->find_first(list, (linked_list_match_t)sa_entry_match_byid,
			(void**)&cached_sa, &protocol, &spi, &reqid) != SUCCESS)
	{
		this->mutex->unlock(this->mutex);
		return JOB_REQUEUE_NONE;
	}
	else
	{
		list->remove(list, cached_sa, NULL);
		sa_entry_destroy(cached_sa);
	}
	this->mutex->unlock(this->mutex);

	hydra->kernel_interface->expire(hydra->kernel_interface, reqid, protocol,
									spi, hard);
	return JOB_REQUEUE_NONE;
}

static void schedule_expire(private_kernel_klips_ipsec_t *this,
							u_int8_t protocol, u_int32_t spi,
							u_int32_t reqid, expire_type_t type, u_int32_t time)
{
	callback_job_t *job;
	sa_expire_t *expire = malloc_thing(sa_expire_t);
	expire->this = this;
	expire->protocol = protocol;
	expire->spi = spi;
	expire->reqid = reqid;
	expire->type = type;
	job = callback_job_create((callback_job_cb_t)sa_expires, expire, free, NULL);
	lib->scheduler->schedule_job(lib->scheduler, (job_t*)job, time);
}

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_kernel_klips_ipsec_t *this, host_t *src, host_t *dst,
	u_int8_t protocol, u_int32_t reqid, u_int32_t *spi)
{
	rng_t *rng;
	u_int32_t spi_gen;

	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->get_bytes(rng, sizeof(spi_gen), (void*)&spi_gen))
	{
		DBG1(DBG_KNL, "allocating SPI failed");
		DESTROY_IF(rng);
		return FAILED;
	}
	rng->destroy(rng);

	
	spi_gen = 0xc0000000 | (spi_gen & 0x0FFFFFFF);

	*spi = htonl(spi_gen);

	this->mutex->lock(this->mutex);
	this->allocated_spis->insert_last(this->allocated_spis,
			create_sa_entry(protocol, *spi, reqid, NULL, NULL, FALSE, TRUE));
	this->mutex->unlock(this->mutex);
	schedule_expire(this, protocol, *spi, reqid, EXPIRE_TYPE_SPI, SPI_TIMEOUT);

	return SUCCESS;
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_klips_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t reqid, u_int16_t *cpi)
{
	return FAILED;
}

static status_t add_ipip_sa(private_kernel_klips_ipsec_t *this,
					   host_t *src, host_t *dst, u_int32_t spi, u_int32_t reqid)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_sa *sa;
	size_t len;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "adding pseudo IPIP SA with SPI %.8x and reqid {%d}", ntohl(spi), reqid);

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_ADD;
	msg->sadb_msg_satype = SADB_X_SATYPE_IPIP;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = spi;
	sa->sadb_sa_state = SADB_SASTATE_MATURE;
	PFKEY_EXT_ADD(msg, sa);

	add_addr_ext(msg, src, SADB_EXT_ADDRESS_SRC);
	add_addr_ext(msg, dst, SADB_EXT_ADDRESS_DST);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to add pseudo IPIP SA with SPI %.8x", ntohl(spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to add pseudo IPIP SA with SPI %.8x: %s (%d)",
				ntohl(spi), strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}

	free(out);
	return SUCCESS;
}

static status_t group_ipip_sa(private_kernel_klips_ipsec_t *this,
					   host_t *src, host_t *dst, u_int32_t spi,
					   u_int8_t protocol, u_int32_t reqid)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_sa *sa;
	struct sadb_x_satype *satype;
	size_t len;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "grouping SAs with SPI %.8x and reqid {%d}", ntohl(spi), reqid);

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_X_GRPSA;
	msg->sadb_msg_satype = SADB_X_SATYPE_IPIP;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = spi;
	sa->sadb_sa_state = SADB_SASTATE_MATURE;
	PFKEY_EXT_ADD(msg, sa);

	add_addr_ext(msg, dst, SADB_EXT_ADDRESS_DST);

	satype = (struct sadb_x_satype*)PFKEY_EXT_ADD_NEXT(msg);
	satype->sadb_x_satype_exttype = SADB_X_EXT_SATYPE2;
	satype->sadb_x_satype_len = PFKEY_LEN(sizeof(struct sadb_x_satype));
	satype->sadb_x_satype_satype = proto2satype(protocol);
	PFKEY_EXT_ADD(msg, satype);

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_X_EXT_SA2;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = spi;
	sa->sadb_sa_state = SADB_SASTATE_MATURE;
	PFKEY_EXT_ADD(msg, sa);

	add_addr_ext(msg, dst, SADB_X_EXT_ADDRESS_DST2);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to group SAs with SPI %.8x", ntohl(spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to group SAs with SPI %.8x: %s (%d)",
				ntohl(spi), strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}

	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_klips_ipsec_t *this, host_t *src, host_t *dst, u_int32_t spi,
	u_int8_t protocol, u_int32_t reqid, mark_t mark, u_int32_t tfc,
	lifetime_cfg_t *lifetime, u_int16_t enc_alg, chunk_t enc_key,
	u_int16_t int_alg, chunk_t int_key, ipsec_mode_t mode,
	u_int16_t ipcomp, u_int16_t cpi, bool initiator, bool encap, bool esn,
	bool inbound, traffic_selector_t *src_ts, traffic_selector_t *dst_ts)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_sa *sa;
	struct sadb_key *key;
	size_t len;

	if (inbound)
	{
		sa_entry_t *alloc_spi;
		this->mutex->lock(this->mutex);
		if (this->allocated_spis->find_first(this->allocated_spis,
				(linked_list_match_t)sa_entry_match_byid, (void**)&alloc_spi,
					&protocol, &spi, &reqid) != SUCCESS)
		{
			this->mutex->unlock(this->mutex);
			DBG1(DBG_KNL, "allocated SPI %.8x has already expired", ntohl(spi));
			return FAILED;
		}
		else
		{
			this->allocated_spis->remove(this->allocated_spis, alloc_spi, NULL);
			sa_entry_destroy(alloc_spi);
		}
		this->mutex->unlock(this->mutex);
	}

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "adding SAD entry with SPI %.8x and reqid {%d}", ntohl(spi), reqid);

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_ADD;
	msg->sadb_msg_satype = proto2satype(protocol);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = spi;
	sa->sadb_sa_state = SADB_SASTATE_MATURE;
	sa->sadb_sa_replay = (protocol == IPPROTO_COMP) ? 0 : 32;
	sa->sadb_sa_auth = lookup_algorithm(INTEGRITY_ALGORITHM, int_alg);
	sa->sadb_sa_encrypt = lookup_algorithm(ENCRYPTION_ALGORITHM, enc_alg);
	PFKEY_EXT_ADD(msg, sa);

	add_addr_ext(msg, src, SADB_EXT_ADDRESS_SRC);
	add_addr_ext(msg, dst, SADB_EXT_ADDRESS_DST);

	if (enc_alg != ENCR_UNDEFINED)
	{
		if (!sa->sadb_sa_encrypt)
		{
			DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
				 encryption_algorithm_names, enc_alg);
			return FAILED;
		}
		DBG2(DBG_KNL, "  using encryption algorithm %N with key size %d",
			 encryption_algorithm_names, enc_alg, enc_key.len * 8);

		key = (struct sadb_key*)PFKEY_EXT_ADD_NEXT(msg);
		key->sadb_key_exttype = SADB_EXT_KEY_ENCRYPT;
		key->sadb_key_bits = enc_key.len * 8;
		key->sadb_key_len = PFKEY_LEN(sizeof(struct sadb_key) + enc_key.len);
		memcpy(key + 1, enc_key.ptr, enc_key.len);

		PFKEY_EXT_ADD(msg, key);
	}

	if (int_alg != AUTH_UNDEFINED)
	{
		if (!sa->sadb_sa_auth)
		{
			DBG1(DBG_KNL, "algorithm %N not supported by kernel!",
					 integrity_algorithm_names, int_alg);
			return FAILED;
		}
		DBG2(DBG_KNL, "  using integrity algorithm %N with key size %d",
			 integrity_algorithm_names, int_alg, int_key.len * 8);

		key = (struct sadb_key*)PFKEY_EXT_ADD_NEXT(msg);
		key->sadb_key_exttype = SADB_EXT_KEY_AUTH;
		key->sadb_key_bits = int_key.len * 8;
		key->sadb_key_len = PFKEY_LEN(sizeof(struct sadb_key) + int_key.len);
		memcpy(key + 1, int_key.ptr, int_key.len);

		PFKEY_EXT_ADD(msg, key);
	}

	if (ipcomp != IPCOMP_NONE)
	{
		
	}

	if (encap)
	{
		add_encap_ext(msg, src, dst, FALSE);
	}

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x", ntohl(spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x: %s (%d)",
				ntohl(spi), strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);

	if (mode == MODE_TUNNEL)
	{
		if (add_ipip_sa(this, src, dst, spi, reqid) != SUCCESS ||
			group_ipip_sa(this, src, dst, spi, protocol, reqid) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to add SAD entry with SPI %.8x", ntohl(spi));
			return FAILED;
		}
	}

	this->mutex->lock(this->mutex);
	this->installed_sas->insert_last(this->installed_sas,
				create_sa_entry(protocol, spi, reqid, src, dst, encap, inbound));
	this->mutex->unlock(this->mutex);

	if (lifetime->time.rekey)
	{
		schedule_expire(this, protocol, spi, reqid, EXPIRE_TYPE_SOFT, lifetime->time.rekey);
	}

	if (lifetime->time.life)
	{
		schedule_expire(this, protocol, spi, reqid, EXPIRE_TYPE_HARD, lifetime->time.life);
	}

	return SUCCESS;
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_klips_ipsec_t *this, u_int32_t spi, u_int8_t protocol,
	u_int16_t cpi, host_t *src, host_t *dst, host_t *new_src, host_t *new_dst,
	bool encap, bool new_encap, mark_t mark)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_sa *sa;
	size_t len;

	if (!src->ip_equals(src, new_src) ||
		!dst->ip_equals(dst, new_dst))
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x: address changes"
				" are not supported", ntohl(spi));
		return NOT_SUPPORTED;
	}

	if (encap != new_encap)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x: change of UDP"
				" encapsulation is not supported", ntohl(spi));
		return NOT_SUPPORTED;
	}

	DBG2(DBG_KNL, "updating SAD entry with SPI %.8x from %#H..%#H to %#H..%#H",
		 ntohl(spi), src, dst, new_src, new_dst);

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_UPDATE;
	msg->sadb_msg_satype = proto2satype(protocol);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = spi;
	sa->sadb_sa_encrypt = SADB_EALG_AESCBC; 
	sa->sadb_sa_auth = SADB_AALG_SHA1HMAC; 
	sa->sadb_sa_state = SADB_SASTATE_MATURE;
	PFKEY_EXT_ADD(msg, sa);

	add_addr_ext(msg, src, SADB_EXT_ADDRESS_SRC);
	add_addr_ext(msg, dst, SADB_EXT_ADDRESS_DST);

	add_encap_ext(msg, new_src, new_dst, TRUE);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x", ntohl(spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to update SAD entry with SPI %.8x: %s (%d)",
				ntohl(spi), strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);

	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_klips_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, mark_t mark,
	u_int64_t *bytes, u_int64_t *packets, time_t *time)
{
	return NOT_SUPPORTED;  
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_klips_ipsec_t *this, host_t *src, host_t *dst,
	u_int32_t spi, u_int8_t protocol, u_int16_t cpi, mark_t mark)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	struct sadb_sa *sa;
	sa_entry_t *cached_sa;
	size_t len;

	memset(&request, 0, sizeof(request));

	DBG2(DBG_KNL, "deleting SAD entry with SPI %.8x", ntohl(spi));

	this->mutex->lock(this->mutex);
	if (this->installed_sas->find_first(this->installed_sas,
			(linked_list_match_t)sa_entry_match_bydst, (void**)&cached_sa,
			&protocol, &spi, dst) == SUCCESS)
	{
		this->installed_sas->remove(this->installed_sas, cached_sa, NULL);
		sa_entry_destroy(cached_sa);
	}
	this->mutex->unlock(this->mutex);

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_DELETE;
	msg->sadb_msg_satype = proto2satype(protocol);
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	sa = (struct sadb_sa*)PFKEY_EXT_ADD_NEXT(msg);
	sa->sadb_sa_exttype = SADB_EXT_SA;
	sa->sadb_sa_len = PFKEY_LEN(sizeof(struct sadb_sa));
	sa->sadb_sa_spi = spi;
	PFKEY_EXT_ADD(msg, sa);

	add_anyaddr_ext(msg, dst->get_family(dst), SADB_EXT_ADDRESS_SRC);
	add_addr_ext(msg, dst, SADB_EXT_ADDRESS_DST);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to delete SAD entry with SPI %.8x", ntohl(spi));
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to delete SAD entry with SPI %.8x: %s (%d)",
				ntohl(spi), strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}

	DBG2(DBG_KNL, "deleted SAD entry with SPI %.8x", ntohl(spi));
	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_klips_ipsec_t *this, host_t *src, host_t *dst,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
	policy_dir_t direction, policy_type_t type, ipsec_sa_cfg_t *sa,
	mark_t mark, policy_priority_t priority)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	policy_entry_t *policy, *found = NULL;
	u_int32_t spi;
	u_int8_t satype;
	size_t len;

	if (direction == POLICY_FWD)
	{
		
		return SUCCESS;
	}

	
	satype = (sa->mode == MODE_TUNNEL) ? SADB_X_SATYPE_IPIP
									   : proto2satype(sa->esp.use ? IPPROTO_ESP
																  : IPPROTO_AH);
	spi = sa->esp.use ? sa->esp.spi : sa->ah.spi;

	
	policy = create_policy_entry(src_ts, dst_ts, direction);

	
	this->mutex->lock(this->mutex);
	if (this->policies->find_first(this->policies,
			(linked_list_match_t)policy_entry_equals, (void**)&found, policy) == SUCCESS)
	{
		
		DBG2(DBG_KNL, "policy %R === %R %N already exists, increasing"
					  " refcount", src_ts, dst_ts,
					   policy_dir_names, direction);
		policy_entry_destroy(policy);
		policy = found;
	}
	else
	{
		
		this->policies->insert_first(this->policies, policy);
	}

	if (priority == POLICY_PRIORITY_ROUTED)
	{
		spi = htonl(SPI_TRAP);
		satype = SADB_X_SATYPE_INT;

		policy->reqid = sa->reqid;

		
		policy->trapcount++;

		if (policy->activecount)
		{
			this->mutex->unlock(this->mutex);
			return SUCCESS;
		}
	}
	else
	{
		
		policy->activecount++;
	}

	DBG2(DBG_KNL, "adding policy %R === %R %N", src_ts, dst_ts,
				   policy_dir_names, direction);

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;

	
	build_addflow(msg, satype, spi,
				  priority == POLICY_PRIORITY_ROUTED ? NULL : src,
				  priority == POLICY_PRIORITY_ROUTED ? NULL : dst,
				  policy->src.net, policy->src.mask, policy->dst.net,
				  policy->dst.mask, policy->src.proto, found != NULL);

	this->mutex->unlock(this->mutex);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to add policy %R === %R %N", src_ts, dst_ts,
					   policy_dir_names, direction);
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to add policy %R === %R %N: %s (%d)", src_ts, dst_ts,
					   policy_dir_names, direction,
					   strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);

	this->mutex->lock(this->mutex);

	
	if (this->policies->find_first(this->policies, NULL,
								  (void**)&policy) != SUCCESS)
	{
		this->mutex->unlock(this->mutex);
		DBG2(DBG_KNL, "the policy %R === %R %N is already gone, ignoring",
				src_ts, dst_ts, policy_dir_names, direction);
		return SUCCESS;
	}

	if (policy->route == NULL && direction == POLICY_OUT)
	{
		char *iface = NULL;
		ipsec_dev_t *dev;
		route_entry_t *route = malloc_thing(route_entry_t);
		route->src_ip = NULL;

		if (sa->mode != MODE_TRANSPORT && src->get_family(src) != AF_INET6 &&
			this->install_routes)
		{
			hydra->kernel_interface->get_address_by_ts(hydra->kernel_interface,
												src_ts, &route->src_ip, NULL);
		}

		if (!route->src_ip)
		{
			route->src_ip = host_create_any(src->get_family(src));
		}

		
		hydra->kernel_interface->get_interface(hydra->kernel_interface,
											   src, &iface);
		if (find_ipsec_dev(this, iface, &dev) == SUCCESS)
		{
			dev->refcount++;
		}
		else
		{
			if (this->ipsec_devices->find_first(this->ipsec_devices,
					(linked_list_match_t)ipsec_dev_match_free,
						(void**)&dev) == SUCCESS)
			{
				if (attach_ipsec_dev(dev->name, iface) == SUCCESS)
				{
					strncpy(dev->phys_name, iface, IFNAMSIZ);
					dev->refcount = 1;
				}
				else
				{
					DBG1(DBG_KNL, "failed to attach virtual interface %s"
							" to %s", dev->name, iface);
					this->mutex->unlock(this->mutex);
					free(iface);
					return FAILED;
				}
			}
			else
			{
				this->mutex->unlock(this->mutex);
				DBG1(DBG_KNL, "failed to attach a virtual interface to %s: no"
						" virtual interfaces left", iface);
				free(iface);
				return FAILED;
			}
		}
		free(iface);
		route->if_name = strdup(dev->name);

		
		route->gateway = hydra->kernel_interface->get_nexthop(
								hydra->kernel_interface, dst, route->src_ip);
		route->dst_net = chunk_clone(policy->dst.net->get_address(policy->dst.net));
		route->prefixlen = policy->dst.mask;

		switch (hydra->kernel_interface->add_route(hydra->kernel_interface,
				route->dst_net, route->prefixlen, route->gateway,
				route->src_ip, route->if_name))
		{
			default:
				DBG1(DBG_KNL, "unable to install route for policy %R === %R",
					 src_ts, dst_ts);
				
			case ALREADY_DONE:
				
				route_entry_destroy(route);
				break;
			case SUCCESS:
				
				policy->route = route;
				break;
		}
	}

	this->mutex->unlock(this->mutex);

	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_klips_ipsec_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, mark_t mark,
	time_t *use_time)
{
	#define IDLE_PREFIX "idle="
	static const char *path_eroute = "/proc/net/ipsec_eroute";
	static const char *path_spi = "/proc/net/ipsec_spi";
	FILE *file;
	char line[1024], src[INET6_ADDRSTRLEN + 9], dst[INET6_ADDRSTRLEN + 9];
	char *said = NULL, *pos;
	policy_entry_t *policy, *found = NULL;
	status_t status = FAILED;

	if (direction == POLICY_FWD)
	{
		
		return FAILED;
	}

	DBG2(DBG_KNL, "querying policy %R === %R %N", src_ts, dst_ts,
				   policy_dir_names, direction);

	
	policy = create_policy_entry(src_ts, dst_ts, direction);

	
	this->mutex->lock(this->mutex);
	if (this->policies->find_first(this->policies,
			(linked_list_match_t)policy_entry_equals, (void**)&found, policy) != SUCCESS)
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_KNL, "querying policy %R === %R %N failed, not found", src_ts,
					   dst_ts, policy_dir_names, direction);
		policy_entry_destroy(policy);
		return NOT_FOUND;
	}
	policy_entry_destroy(policy);
	policy = found;

	
	snprintf(src, sizeof(src), "%H/%d:%d", policy->src.net, policy->src.mask,
			policy->src.proto);
	src[sizeof(src) - 1] = '\0';
	snprintf(dst, sizeof(dst), "%H/%d:%d", policy->dst.net, policy->dst.mask,
			policy->dst.proto);
	dst[sizeof(dst) - 1] = '\0';

	this->mutex->unlock(this->mutex);

	
	file = fopen(path_eroute, "r");
	if (file == NULL)
	{
		DBG1(DBG_KNL, "unable to query policy %R === %R %N: %s (%d)", src_ts,
				dst_ts, policy_dir_names, direction, strerror(errno), errno);
		return FAILED;
	}

	while (fgets(line, sizeof(line), file))
	{
		enumerator_t *enumerator;
		char *token;
		int i = 0;

		enumerator = enumerator_create_token(line, " \t", " \t\n");
		while (enumerator->enumerate(enumerator, &token))
		{
			switch (i++)
			{
				case 0: 
					continue;
				case 1: 
					if (streq(token, src))
					{
						continue;
					}
					break;
				case 2: 
					continue;
				case 3: 
					if (streq(token, dst))
					{
						continue;
					}
					break;
				case 4: 
					continue;
				case 5: 
					said = strdup(token);
					break;
			}
			break;
		}
		enumerator->destroy(enumerator);

		if (i == 5)
		{
			
			break;
		}
	}
	fclose(file);

	if (said == NULL)
	{
		DBG1(DBG_KNL, "unable to query policy %R === %R %N: found no matching"
				" eroute", src_ts, dst_ts, policy_dir_names, direction);
		return FAILED;
	}

	pos = strrchr(said, ':');
	*pos = '\0';

	
	file = fopen(path_spi, "r");
	if (file == NULL)
	{
		DBG1(DBG_KNL, "unable to query policy %R === %R %N: %s (%d)", src_ts,
				dst_ts, policy_dir_names, direction, strerror(errno), errno);
		return FAILED;
	}

	while (fgets(line, sizeof(line), file))
	{
		if (strpfx(line, said))
		{
			
			u_int32_t idle_time;
			pos = strstr(line, IDLE_PREFIX);
			if (pos == NULL)
			{
				
				break;
			}
			if (sscanf(pos, IDLE_PREFIX"%u", &idle_time) <= 0)
			{
				
				break;
			}

			*use_time = time_monotonic(NULL) - idle_time;
			status = SUCCESS;
			break;
		}
	}
	fclose(file);
	free(said);

	return status;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_klips_ipsec_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, u_int32_t reqid,
	mark_t mark, policy_priority_t priority)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg = (struct sadb_msg*)request, *out;
	policy_entry_t *policy, *found = NULL;
	route_entry_t *route;
	size_t len;

	if (direction == POLICY_FWD)
	{
		
		return SUCCESS;
	}

	DBG2(DBG_KNL, "deleting policy %R === %R %N", src_ts, dst_ts,
				   policy_dir_names, direction);

	
	policy = create_policy_entry(src_ts, dst_ts, direction);

	
	this->mutex->lock(this->mutex);
	if (this->policies->find_first(this->policies,
			(linked_list_match_t)policy_entry_equals, (void**)&found, policy) != SUCCESS)
	{
		this->mutex->unlock(this->mutex);
		DBG1(DBG_KNL, "deleting policy %R === %R %N failed, not found", src_ts,
					   dst_ts, policy_dir_names, direction);
		policy_entry_destroy(policy);
		return NOT_FOUND;
	}
	policy_entry_destroy(policy);

	
	priority == POLICY_PRIORITY_ROUTED ? found->trapcount--
									   : found->activecount--;

	if (found->trapcount == 0)
	{
		found->reqid = 0;
	}

	if (found->activecount > 0)
	{
		
		this->mutex->unlock(this->mutex);
		DBG2(DBG_KNL, "policy still used by another CHILD_SA, not removed");
		return SUCCESS;
	}
	else if (found->activecount == 0 && found->trapcount > 0)
	{
		DBG2(DBG_KNL, "policy still routed by another CHILD_SA, not removed");
		memset(&request, 0, sizeof(request));
		build_addflow(msg, SADB_X_SATYPE_INT, htonl(SPI_TRAP), NULL, NULL,
				found->src.net, found->src.mask, found->dst.net,
				found->dst.mask, found->src.proto, TRUE);
		this->mutex->unlock(this->mutex);
		return pfkey_send_ack(this, msg);
	}

	
	this->policies->remove(this->policies, found, NULL);
	policy = found;

	this->mutex->unlock(this->mutex);

	memset(&request, 0, sizeof(request));

	build_delflow(msg, 0, policy->src.net, policy->src.mask, policy->dst.net,
			policy->dst.mask, policy->src.proto);

	route = policy->route;
	policy->route = NULL;
	policy_entry_destroy(policy);

	if (pfkey_send(this, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to delete policy %R === %R %N", src_ts, dst_ts,
					   policy_dir_names, direction);
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to delete policy %R === %R %N: %s (%d)", src_ts,
					   dst_ts, policy_dir_names, direction,
					   strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);

	if (route)
	{
		ipsec_dev_t *dev;

		if (hydra->kernel_interface->del_route(hydra->kernel_interface,
				route->dst_net, route->prefixlen, route->gateway,
				route->src_ip, route->if_name) != SUCCESS)
		{
			DBG1(DBG_KNL, "error uninstalling route installed with"
						  " policy %R === %R %N", src_ts, dst_ts,
						   policy_dir_names, direction);
		}

		this->mutex->lock(this->mutex);

		if (find_ipsec_dev(this, route->if_name, &dev) == SUCCESS)
		{
			if (--dev->refcount == 0)
			{
				if (detach_ipsec_dev(dev->name, dev->phys_name) != SUCCESS)
				{
					DBG1(DBG_KNL, "failed to detach virtual interface %s"
							" from %s", dev->name, dev->phys_name);
				}
				dev->phys_name[0] = '\0';
			}
		}

		this->mutex->unlock(this->mutex);

		route_entry_destroy(route);
	}

	return SUCCESS;
}

static void init_ipsec_devices(private_kernel_klips_ipsec_t *this)
{
	int i, count = lib->settings->get_int(lib->settings,
									"%s.plugins.kernel-klips.ipsec_dev_count",
									DEFAULT_IPSEC_DEV_COUNT, lib->ns);

	for (i = 0; i < count; ++i)
	{
		ipsec_dev_t *dev = malloc_thing(ipsec_dev_t);
		snprintf(dev->name, IFNAMSIZ, IPSEC_DEV_PREFIX"%d", i);
		dev->name[IFNAMSIZ - 1] = '\0';
		dev->phys_name[0] = '\0';
		dev->refcount = 0;
		this->ipsec_devices->insert_last(this->ipsec_devices, dev);

		
		detach_ipsec_dev(dev->name, dev->phys_name);
	}
}

static status_t register_pfkey_socket(private_kernel_klips_ipsec_t *this, u_int8_t satype)
{
	unsigned char request[PFKEY_BUFFER_SIZE];
	struct sadb_msg *msg, *out;
	size_t len;

	memset(&request, 0, sizeof(request));

	msg = (struct sadb_msg*)request;
	msg->sadb_msg_version = PF_KEY_V2;
	msg->sadb_msg_type = SADB_REGISTER;
	msg->sadb_msg_satype = satype;
	msg->sadb_msg_len = PFKEY_LEN(sizeof(struct sadb_msg));

	if (pfkey_send_socket(this, this->socket_events, msg, &out, &len) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to register PF_KEY socket");
		return FAILED;
	}
	else if (out->sadb_msg_errno)
	{
		DBG1(DBG_KNL, "unable to register PF_KEY socket: %s (%d)",
					   strerror(out->sadb_msg_errno), out->sadb_msg_errno);
		free(out);
		return FAILED;
	}
	free(out);
	return SUCCESS;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_klips_ipsec_t *this)
{
	if (this->socket > 0)
	{
		close(this->socket);
	}
	if (this->socket_events > 0)
	{
		close(this->socket_events);
	}
	this->mutex_pfkey->destroy(this->mutex_pfkey);
	this->mutex->destroy(this->mutex);
	this->ipsec_devices->destroy_function(this->ipsec_devices, (void*)ipsec_dev_destroy);
	this->installed_sas->destroy_function(this->installed_sas, (void*)sa_entry_destroy);
	this->allocated_spis->destroy_function(this->allocated_spis, (void*)sa_entry_destroy);
	this->policies->destroy_function(this->policies, (void*)policy_entry_destroy);
	free(this);
}

kernel_klips_ipsec_t *kernel_klips_ipsec_create()
{
	private_kernel_klips_ipsec_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_spi = _get_spi,
				.get_cpi = _get_cpi,
				.add_sa  = _add_sa,
				.update_sa = _update_sa,
				.query_sa = _query_sa,
				.del_sa = _del_sa,
				.flush_sas = (void*)return_failed,
				.add_policy = _add_policy,
				.query_policy = _query_policy,
				.del_policy = _del_policy,
				.flush_policies = (void*)return_failed,
				
				.bypass_socket = (void*)return_true,
				
				.enable_udp_decap = (void*)return_true,
				.destroy = _destroy,
			},
		},
		.policies = linked_list_create(),
		.allocated_spis = linked_list_create(),
		.installed_sas = linked_list_create(),
		.ipsec_devices = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.mutex_pfkey = mutex_create(MUTEX_TYPE_DEFAULT),
		.install_routes = lib->settings->get_bool(lib->settings,
												  "%s.install_routes", TRUE,
												  lib->ns),
	);

	
	init_ipsec_devices(this);

	
	this->socket = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
	if (this->socket <= 0)
	{
		DBG1(DBG_KNL, "unable to create PF_KEY socket");
		destroy(this);
		return NULL;
	}

	
	this->socket_events = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
	if (this->socket_events <= 0)
	{
		DBG1(DBG_KNL, "unable to create PF_KEY event socket");
		destroy(this);
		return NULL;
	}

	
	if (register_pfkey_socket(this, SADB_SATYPE_ESP) != SUCCESS ||
		register_pfkey_socket(this, SADB_SATYPE_AH) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to register PF_KEY event socket");
		destroy(this);
		return NULL;
	}

	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)receive_events,
			this, NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));

	return &this->public;
}
