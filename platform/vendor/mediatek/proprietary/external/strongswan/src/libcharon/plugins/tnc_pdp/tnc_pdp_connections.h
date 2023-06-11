/*
 * Copyright (C) 2012 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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


#ifndef TNC_PDP_CONNECTIONS_H_
#define TNC_PDP_CONNECTIONS_H_

typedef struct tnc_pdp_connections_t tnc_pdp_connections_t;

#include <library.h>
#include <sa/ike_sa.h>
#include <sa/eap/eap_method.h>

struct tnc_pdp_connections_t {

	void (*add)(tnc_pdp_connections_t *this, chunk_t nas_id, chunk_t user_name,
				identification_t *peer, eap_method_t *method);

	void (*remove)(tnc_pdp_connections_t *this, chunk_t nas_id,
				   chunk_t user_name);

	eap_method_t* (*get_state)(tnc_pdp_connections_t *this, chunk_t nas_id,
							   chunk_t user_name, ike_sa_t **ike_sa);

	void (*unlock)(tnc_pdp_connections_t *this);

	void (*destroy)(tnc_pdp_connections_t *this);
};

tnc_pdp_connections_t* tnc_pdp_connections_create(void);

#endif 
