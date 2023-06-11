/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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


#ifndef DUPLICHECK_MSG_H_
#define DUPLICHECK_MSG_H_

#include <sys/types.h>

#define DUPLICHECK_SOCKET IPSEC_PIDDIR "/charon.dck"

typedef struct duplicheck_msg_t duplicheck_msg_t;

struct duplicheck_msg_t {
	
	u_int16_t len;
	
	char identity[];
} __attribute__((__packed__));

#endif 
