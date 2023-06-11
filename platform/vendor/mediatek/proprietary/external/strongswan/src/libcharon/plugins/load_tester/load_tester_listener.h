/*
 * Copyright (C) 2008 Martin Willi
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


#ifndef LOAD_TESTER_LISTENER_H_
#define LOAD_TESTER_LISTENER_H_

#include <bus/bus.h>

#include "load_tester_config.h"

typedef struct load_tester_listener_t load_tester_listener_t;

struct load_tester_listener_t {

	listener_t listener;

	u_int (*get_established)(load_tester_listener_t *this);

	void (*destroy)(load_tester_listener_t *this);
};

load_tester_listener_t *load_tester_listener_create(u_int shutdown_on,
													load_tester_config_t *config);

#endif 
