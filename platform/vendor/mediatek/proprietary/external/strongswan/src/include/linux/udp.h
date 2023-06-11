/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the UDP protocol.
 *
 * Version:	@(#)udp.h	1.0.2	04/28/93
 *
 * Author:	Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_UDP_H
#define _LINUX_UDP_H

#include <linux/types.h>

struct udphdr {
	__u16	source;
	__u16	dest;
	__u16	len;
	__u16	check;
};

#define UDP_CORK	1	
#define UDP_ENCAP	100	

#define UDP_ENCAP_ESPINUDP_NON_IKE	1 
#define UDP_ENCAP_ESPINUDP	2 

#ifdef __KERNEL__
#include <linux/config.h>
#include <linux/types.h>

#include <net/inet_sock.h>

struct udp_sock {
	
	struct inet_sock inet;
	int		 pending;	
	unsigned int	 corkflag;	
	__u16		 encap_type;	
	__u16		 len;		
};

static __inline__ struct udp_sock *udp_sk(const struct sock *sk)
{
	return (struct udp_sock *)sk;
}

#endif

#endif	
