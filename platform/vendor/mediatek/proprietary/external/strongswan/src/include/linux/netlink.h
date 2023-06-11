#ifndef __LINUX_NETLINK_H
#define __LINUX_NETLINK_H

#include <linux/socket.h> 
#include <linux/types.h>

#define NETLINK_ROUTE		0	
#define NETLINK_W1		1	
#define NETLINK_USERSOCK	2	
#define NETLINK_FIREWALL	3	
#define NETLINK_INET_DIAG	4	
#define NETLINK_NFLOG		5	
#define NETLINK_XFRM		6	
#define NETLINK_SELINUX		7	
#define NETLINK_ISCSI		8	
#define NETLINK_AUDIT		9	
#define NETLINK_FIB_LOOKUP	10
#define NETLINK_CONNECTOR	11
#define NETLINK_NETFILTER	12	
#define NETLINK_IP6_FW		13
#define NETLINK_DNRTMSG		14	
#define NETLINK_KOBJECT_UEVENT	15	
#define NETLINK_GENERIC		16

#define MAX_LINKS 32

struct sockaddr_nl
{
	sa_family_t	nl_family;	
	unsigned short	nl_pad;		
	__u32		nl_pid;		
	__u32		nl_groups;	
};

struct nlmsghdr
{
	__u32		nlmsg_len;	
	__u16		nlmsg_type;	
	__u16		nlmsg_flags;	
	__u32		nlmsg_seq;	
	__u32		nlmsg_pid;	
};


#define NLM_F_REQUEST		1	
#define NLM_F_MULTI		2	
#define NLM_F_ACK		4	
#define NLM_F_ECHO		8	

#define NLM_F_ROOT	0x100	
#define NLM_F_MATCH	0x200	
#define NLM_F_ATOMIC	0x400	
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

#define NLM_F_REPLACE	0x100	
#define NLM_F_EXCL	0x200	
#define NLM_F_CREATE	0x400	
#define NLM_F_APPEND	0x800	


#define NLMSG_ALIGNTO	4
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_HDRLEN	 ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(len) ((len)+NLMSG_ALIGN(NLMSG_HDRLEN))
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len)	 ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
				  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP		0x1	
#define NLMSG_ERROR		0x2	
#define NLMSG_DONE		0x3	
#define NLMSG_OVERRUN		0x4	

#define NLMSG_MIN_TYPE		0x10	

struct nlmsgerr
{
	int		error;
	struct nlmsghdr msg;
};

#define NETLINK_ADD_MEMBERSHIP	1
#define NETLINK_DROP_MEMBERSHIP	2
#define NETLINK_PKTINFO		3

struct nl_pktinfo
{
	__u32	group;
};

#define NET_MAJOR 36		

enum {
	NETLINK_UNCONNECTED = 0,
	NETLINK_CONNECTED,
};


struct nlattr
{
	__u16           nla_len;
	__u16           nla_type;
};

#define NLA_ALIGNTO		4
#define NLA_ALIGN(len)		(((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN		((int) NLA_ALIGN(sizeof(struct nlattr)))

#ifdef __KERNEL__

#include <linux/capability.h>
#include <linux/skbuff.h>

struct netlink_skb_parms
{
	struct ucred		creds;		
	__u32			pid;
	__u32			dst_pid;
	__u32			dst_group;
	kernel_cap_t		eff_cap;
	__u32			loginuid;	
	__u32			sid;		
};

#define NETLINK_CB(skb)		(*(struct netlink_skb_parms*)&((skb)->cb))
#define NETLINK_CREDS(skb)	(&NETLINK_CB((skb)).creds)


extern struct sock *netlink_kernel_create(int unit, unsigned int groups, void (*input)(struct sock *sk, int len), struct module *module);
extern void netlink_ack(struct sk_buff *in_skb, struct nlmsghdr *nlh, int err);
extern int netlink_has_listeners(struct sock *sk, unsigned int group);
extern int netlink_unicast(struct sock *ssk, struct sk_buff *skb, __u32 pid, int nonblock);
extern int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, __u32 pid,
			     __u32 group, gfp_t allocation);
extern void netlink_set_err(struct sock *ssk, __u32 pid, __u32 group, int code);
extern int netlink_register_notifier(struct notifier_block *nb);
extern int netlink_unregister_notifier(struct notifier_block *nb);

struct sock *netlink_getsockbyfilp(struct file *filp);
int netlink_attachskb(struct sock *sk, struct sk_buff *skb, int nonblock,
		long timeo, struct sock *ssk);
void netlink_detachskb(struct sock *sk, struct sk_buff *skb);
int netlink_sendskb(struct sock *sk, struct sk_buff *skb, int protocol);

#define NLMSG_GOODORDER 0
#define NLMSG_GOODSIZE (SKB_MAX_ORDER(0, NLMSG_GOODORDER))


struct netlink_callback
{
	struct sk_buff	*skb;
	struct nlmsghdr	*nlh;
	int		(*dump)(struct sk_buff * skb, struct netlink_callback *cb);
	int		(*done)(struct netlink_callback *cb);
	int		family;
	long		args[5];
};

struct netlink_notify
{
	int pid;
	int protocol;
};

static __inline__ struct nlmsghdr *
__nlmsg_put(struct sk_buff *skb, __u32 pid, __u32 seq, int type, int len, int flags)
{
	struct nlmsghdr *nlh;
	int size = NLMSG_LENGTH(len);

	nlh = (struct nlmsghdr*)skb_put(skb, NLMSG_ALIGN(size));
	nlh->nlmsg_type = type;
	nlh->nlmsg_len = size;
	nlh->nlmsg_flags = flags;
	nlh->nlmsg_pid = pid;
	nlh->nlmsg_seq = seq;
	memset(NLMSG_DATA(nlh) + len, 0, NLMSG_ALIGN(size) - size);
	return nlh;
}

#define NLMSG_NEW(skb, pid, seq, type, len, flags) \
({	if (skb_tailroom(skb) < (int)NLMSG_SPACE(len)) \
		goto nlmsg_failure; \
	__nlmsg_put(skb, pid, seq, type, len, flags); })

#define NLMSG_PUT(skb, pid, seq, type, len) \
	NLMSG_NEW(skb, pid, seq, type, len, 0)

#define NLMSG_NEW_ANSWER(skb, cb, type, len, flags) \
	NLMSG_NEW(skb, NETLINK_CB((cb)->skb).pid, \
		  (cb)->nlh->nlmsg_seq, type, len, flags)

#define NLMSG_END(skb, nlh) \
({	(nlh)->nlmsg_len = (skb)->tail - (unsigned char *) (nlh); \
	(skb)->len; })

#define NLMSG_CANCEL(skb, nlh) \
({	skb_trim(skb, (unsigned char *) (nlh) - (skb)->data); \
	-1; })

extern int netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
			      struct nlmsghdr *nlh,
			      int (*dump)(struct sk_buff *skb, struct netlink_callback*),
			      int (*done)(struct netlink_callback*));


#define NL_NONROOT_RECV 0x1
#define NL_NONROOT_SEND 0x2
extern void netlink_set_nonroot(int protocol, unsigned flag);

#endif 

#endif	
