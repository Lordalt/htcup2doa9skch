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


#ifndef FAST_CONTROLLER_H_
#define FAST_CONTROLLER_H_

#include "fast_request.h"
#include "fast_context.h"

typedef struct fast_controller_t fast_controller_t;

typedef fast_controller_t *(*fast_controller_constructor_t)(
										fast_context_t* context, void *param);

struct fast_controller_t {

	char* (*get_name)(fast_controller_t *this);

	void (*handle)(fast_controller_t *this, fast_request_t *request,
				   char *p1, char *p2, char *p3, char *p4, char *p5);

	void (*destroy) (fast_controller_t *this);
};

#endif 
