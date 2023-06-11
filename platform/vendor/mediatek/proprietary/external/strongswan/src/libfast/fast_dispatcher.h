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


#ifndef FAST_DISPATCHER_H_
#define FAST_DISPATCHER_H_

#include "fast_controller.h"
#include "fast_filter.h"

typedef struct fast_dispatcher_t fast_dispatcher_t;

struct fast_dispatcher_t {

	void (*add_controller)(fast_dispatcher_t *this,
						   fast_controller_constructor_t constructor,
						   void *param);

	void (*add_filter)(fast_dispatcher_t *this,
					   fast_filter_constructor_t constructor, void *param);

	void (*run)(fast_dispatcher_t *this, int threads);

	void (*waitsignal)(fast_dispatcher_t *this);

	void (*destroy) (fast_dispatcher_t *this);
};

fast_dispatcher_t *fast_dispatcher_create(char *socket, bool debug, int timeout,
							fast_context_constructor_t constructor, void *param);

#endif 
