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


#ifndef FAST_SESSION_H_
#define FAST_SESSION_H_

#include "fast_request.h"
#include "fast_controller.h"
#include "fast_filter.h"

typedef struct fast_session_t fast_session_t;

struct fast_session_t {

	char* (*get_sid)(fast_session_t *this);

	void (*add_controller)(fast_session_t *this, fast_controller_t *controller);

	void (*add_filter)(fast_session_t *this, fast_filter_t *filter);

	void (*process)(fast_session_t *this, fast_request_t *request);

	void (*destroy) (fast_session_t *this);
};

fast_session_t *fast_session_create(fast_context_t *context);

#endif 
