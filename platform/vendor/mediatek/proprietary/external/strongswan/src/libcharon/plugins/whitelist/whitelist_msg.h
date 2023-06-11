/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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


#ifndef WHITELIST_MSG_H_
#define WHITELIST_MSG_H_

#define WHITELIST_SOCKET IPSEC_PIDDIR "/charon.wlst"

typedef struct whitelist_msg_t whitelist_msg_t;

enum {
	
	WHITELIST_ADD = 1,
	
	WHITELIST_REMOVE = 2,
	
	WHITELIST_LIST = 3,
	
	WHITELIST_END = 4,
	
	WHITELIST_FLUSH = 5,
	
	WHITELIST_ENABLE = 6,
	
	WHITELIST_DISABLE = 7,
};

struct whitelist_msg_t {
	
	int type;
	
	char id[128];
} __attribute__((packed));

#endif 
