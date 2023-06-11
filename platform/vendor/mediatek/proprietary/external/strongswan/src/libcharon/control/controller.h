/*
 * Copyright (C) 2007 Martin Willi
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


#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <bus/bus.h>

typedef bool (*controller_cb_t)(void* param, debug_t group, level_t level,
								ike_sa_t* ike_sa, const char *message);

bool controller_cb_empty(void *param, debug_t group, level_t level,
						 ike_sa_t *ike_sa, const char *message);

typedef struct controller_t controller_t;

struct controller_t {

	enumerator_t* (*create_ike_sa_enumerator)(controller_t *this, bool wait);

	status_t (*initiate)(controller_t *this,
						 peer_cfg_t *peer_cfg, child_cfg_t *child_cfg,
						 controller_cb_t callback, void *param, u_int timeout);

	status_t (*terminate_ike)(controller_t *this, u_int32_t unique_id,
							  controller_cb_t callback, void *param,
							  u_int timeout);

	status_t (*terminate_child)(controller_t *this, u_int32_t reqid,
								controller_cb_t callback, void *param,
								u_int timeout);

	void (*destroy) (controller_t *this);
};

controller_t *controller_create();

#endif 
