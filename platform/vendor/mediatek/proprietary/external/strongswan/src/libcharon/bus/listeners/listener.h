/*
 * Copyright (C) 2009 Martin Willi
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


#ifndef LISTENER_H_
#define LISTENER_H_

typedef struct listener_t listener_t;

#include <bus/bus.h>

struct listener_t {

	bool (*alert)(listener_t *this, ike_sa_t *ike_sa,
				  alert_t alert, va_list args);

	bool (*ike_state_change)(listener_t *this, ike_sa_t *ike_sa,
							 ike_sa_state_t state);

	bool (*child_state_change)(listener_t *this, ike_sa_t *ike_sa,
							   child_sa_t *child_sa, child_sa_state_t state);

	bool (*message)(listener_t *this, ike_sa_t *ike_sa, message_t *message,
					bool incoming, bool plain);

	bool (*ike_keys)(listener_t *this, ike_sa_t *ike_sa, diffie_hellman_t *dh,
					 chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r,
					 ike_sa_t *rekey, shared_key_t *shared);

	bool (*child_keys)(listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
					   bool initiator, diffie_hellman_t *dh,
					   chunk_t nonce_i, chunk_t nonce_r);

	bool (*ike_updown)(listener_t *this, ike_sa_t *ike_sa, bool up);

	bool (*ike_rekey)(listener_t *this, ike_sa_t *old, ike_sa_t *new);

	bool (*ike_reestablish)(listener_t *this, ike_sa_t *old, ike_sa_t *new);

	bool (*child_updown)(listener_t *this, ike_sa_t *ike_sa,
						 child_sa_t *child_sa, bool up);

	bool (*child_rekey)(listener_t *this, ike_sa_t *ike_sa,
						child_sa_t *old, child_sa_t *new);

	bool (*authorize)(listener_t *this, ike_sa_t *ike_sa,
					  bool final, bool *success);

	bool (*narrow)(listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
				narrow_hook_t type, linked_list_t *local, linked_list_t *remote);

	bool (*assign_vips)(listener_t *this, ike_sa_t *ike_sa, bool assign);

};

#endif 
