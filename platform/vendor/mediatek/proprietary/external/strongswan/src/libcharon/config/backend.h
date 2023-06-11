/*
 * Copyright (C) 2007-2008 Martin Willi
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


#ifndef BACKEND_H_
#define BACKEND_H_

typedef struct backend_t backend_t;

#include <library.h>
#include <config/ike_cfg.h>
#include <config/peer_cfg.h>
#include <collections/linked_list.h>

struct backend_t {

	enumerator_t* (*create_ike_cfg_enumerator)(backend_t *this,
											   host_t *me, host_t *other);
	enumerator_t* (*create_peer_cfg_enumerator)(backend_t *this,
												identification_t *me,
												identification_t *other);
	peer_cfg_t *(*get_peer_cfg_by_name)(backend_t *this, char *name);
};

#endif 
