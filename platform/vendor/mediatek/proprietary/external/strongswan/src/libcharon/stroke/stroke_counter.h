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


#ifndef STROKE_COUNTER_H_
#define STROKE_COUNTER_H_

#include <bus/listeners/listener.h>

typedef struct stroke_counter_t stroke_counter_t;
typedef enum stroke_counter_type_t stroke_counter_type_t;

enum stroke_counter_type_t {
	
	COUNTER_INIT_IKE_SA_REKEY,
	
	COUNTER_RESP_IKE_SA_REKEY,
	
	COUNTER_CHILD_SA_REKEY,
	
	COUNTER_IN_INVALID,
	
	COUNTER_IN_INVALID_IKE_SPI,
	
	COUNTER_IN_IKE_SA_INIT_REQ,
	
	COUNTER_IN_IKE_SA_INIT_RSP,
	
	COUNTER_OUT_IKE_SA_INIT_REQ,
	
	COUNTER_OUT_IKE_SA_INIT_RES,
	
	COUNTER_IN_IKE_AUTH_REQ,
	
	COUNTER_IN_IKE_AUTH_RSP,
	
	COUNTER_OUT_IKE_AUTH_REQ,
	
	COUNTER_OUT_IKE_AUTH_RSP,
	
	COUNTER_IN_CREATE_CHILD_SA_REQ,
	
	COUNTER_IN_CREATE_CHILD_SA_RSP,
	
	COUNTER_OUT_CREATE_CHILD_SA_REQ,
	
	COUNTER_OUT_CREATE_CHILD_SA_RSP,
	
	COUNTER_IN_INFORMATIONAL_REQ,
	
	COUNTER_IN_INFORMATIONAL_RSP,
	
	COUNTER_OUT_INFORMATIONAL_REQ,
	
	COUNTER_OUT_INFORMATIONAL_RSP,
	
	COUNTER_MAX
};

struct stroke_counter_t {

	listener_t listener;

	void (*print)(stroke_counter_t *this, FILE *out, char *name);

	void (*reset)(stroke_counter_t *this, char *name);

	void (*destroy)(stroke_counter_t *this);
};

stroke_counter_t *stroke_counter_create();

#endif 
