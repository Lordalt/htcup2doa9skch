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


#ifndef STROKE_CONTROL_H_
#define STROKE_CONTROL_H_

#include <stroke_msg.h>
#include <library.h>
#include <stdio.h>

typedef struct stroke_control_t stroke_control_t;

struct stroke_control_t {

	void (*initiate)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	void (*terminate)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	void (*terminate_srcip)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	void (*rekey)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	void (*purge_ike)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	void (*route)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	void (*unroute)(stroke_control_t *this, stroke_msg_t *msg, FILE *out);

	void (*destroy)(stroke_control_t *this);
};

stroke_control_t *stroke_control_create();

#endif 
