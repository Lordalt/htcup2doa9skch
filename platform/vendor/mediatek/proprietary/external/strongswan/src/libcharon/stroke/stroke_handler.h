/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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


#ifndef STROKE_HANDLER_H_
#define STROKE_HANDLER_H_

#include <stroke_msg.h>
#include <attributes/attribute_handler.h>

typedef struct stroke_handler_t stroke_handler_t;

struct stroke_handler_t {

	attribute_handler_t handler;

	void (*add_attributes)(stroke_handler_t *this, stroke_msg_t *msg);

	void (*del_attributes)(stroke_handler_t *this, stroke_msg_t *msg);

	void (*destroy)(stroke_handler_t *this);
};

stroke_handler_t *stroke_handler_create();

#endif 
