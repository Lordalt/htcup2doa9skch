/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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


#ifndef DELETE_PAYLOAD_H_
#define DELETE_PAYLOAD_H_

typedef struct delete_payload_t delete_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/proposal_substructure.h>

struct delete_payload_t {

	payload_t payload_interface;

	protocol_id_t (*get_protocol_id) (delete_payload_t *this);

	void (*add_spi) (delete_payload_t *this, u_int32_t spi);

	void (*set_ike_spi)(delete_payload_t *this, u_int64_t spi_i, u_int64_t spi_r);

	enumerator_t *(*create_spi_enumerator) (delete_payload_t *this);

	void (*destroy) (delete_payload_t *this);
};

delete_payload_t *delete_payload_create(payload_type_t type,
										protocol_id_t protocol_id);

#endif 
