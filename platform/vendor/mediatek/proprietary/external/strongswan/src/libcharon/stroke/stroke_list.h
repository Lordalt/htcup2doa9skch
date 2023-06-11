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


#ifndef STROKE_LIST_H_
#define STROKE_LIST_H_

#include "stroke_attribute.h"

#include <stroke_msg.h>
#include <library.h>

typedef struct stroke_list_t stroke_list_t;

struct stroke_list_t {

	void (*list)(stroke_list_t *this, stroke_msg_t *msg, FILE *out);

	void (*status)(stroke_list_t *this, stroke_msg_t *msg, FILE *out,
				   bool all, bool wait);

	void (*leases)(stroke_list_t *this, stroke_msg_t *msg, FILE *out);

	void (*destroy)(stroke_list_t *this);
	
	void (*trigger_dpd)(stroke_list_t *this, stroke_msg_t *msg, FILE *out);
};

stroke_list_t *stroke_list_create(stroke_attribute_t *attribute);

#endif 
