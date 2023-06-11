/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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


#ifndef TKM_KEYMAT_H_
#define TKM_KEYMAT_H_

#include <sa/ikev2/keymat_v2.h>

typedef struct tkm_keymat_t tkm_keymat_t;

struct tkm_keymat_t {

	keymat_v2_t keymat_v2;

	isa_id_type (*get_isa_id)(tkm_keymat_t * const this);

	void (*set_auth_payload)(tkm_keymat_t *this, const chunk_t * const payload);

	chunk_t* (*get_auth_payload)(tkm_keymat_t * const this);

	chunk_t* (*get_peer_init_msg)(tkm_keymat_t * const this);

};

tkm_keymat_t *tkm_keymat_create(bool initiator);

#endif 
