/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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


#ifndef DHCP_SOCKET_H_
#define DHCP_SOCKET_H_

typedef struct dhcp_socket_t dhcp_socket_t;

#include "dhcp_transaction.h"

struct dhcp_socket_t {

	dhcp_transaction_t* (*enroll)(dhcp_socket_t *this,
								  identification_t *identity);

	void (*release)(dhcp_socket_t *this, dhcp_transaction_t *transaction);

	void (*destroy)(dhcp_socket_t *this);
};

dhcp_socket_t *dhcp_socket_create();

#endif 
