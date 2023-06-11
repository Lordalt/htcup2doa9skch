/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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


#ifndef LOOKIP_MSG_H_
#define LOOKIP_MSG_H_

#define LOOKIP_SOCKET IPSEC_PIDDIR "/charon.lkp"

typedef struct lookip_request_t lookip_request_t;
typedef struct lookip_response_t lookip_response_t;

enum {
	
	LOOKIP_DUMP = 1,
	
	LOOKIP_LOOKUP,
	
	LOOKIP_ENTRY,
	
	LOOKIP_NOT_FOUND,
	
	LOOKIP_REGISTER_UP,
	
	LOOKIP_REGISTER_DOWN,
	
	LOOKIP_NOTIFY_UP,
	
	LOOKIP_NOTIFY_DOWN,
	
	LOOKIP_END,
};

struct lookip_request_t {
	
	int type;
	
	char vip[40];
} __attribute__((packed));

struct lookip_response_t {
	
	int type;
	
	char vip[40];
	
	char ip[40];
	
	char id[256];
	
	char name[40];
	
	unsigned int unique_id;
} __attribute__((packed));

#endif 
