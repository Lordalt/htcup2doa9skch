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


#ifndef DHCP_TRANSACTION_H_
#define DHCP_TRANSACTION_H_

#include <networking/host.h>
#include <utils/identification.h>
#include <attributes/attributes.h>

typedef struct dhcp_transaction_t dhcp_transaction_t;

struct dhcp_transaction_t {

	u_int32_t (*get_id)(dhcp_transaction_t *this);

	identification_t* (*get_identity)(dhcp_transaction_t *this);

	void (*set_address)(dhcp_transaction_t *this, host_t *address);

	host_t* (*get_address)(dhcp_transaction_t *this);

	void (*set_server)(dhcp_transaction_t *this, host_t *server);

	host_t* (*get_server)(dhcp_transaction_t *this);

	void (*add_attribute)(dhcp_transaction_t *this,
						  configuration_attribute_type_t type, chunk_t data);

	enumerator_t* (*create_attribute_enumerator)(dhcp_transaction_t *this);

	void (*destroy)(dhcp_transaction_t *this);
};

dhcp_transaction_t *dhcp_transaction_create(u_int32_t id,
											identification_t *identity);

#endif 
