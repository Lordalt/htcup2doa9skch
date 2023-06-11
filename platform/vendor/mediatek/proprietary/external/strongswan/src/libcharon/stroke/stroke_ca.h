/*
 * Copyright (C) 2008 Tobias Brunner
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


#ifndef STROKE_CA_H_
#define STROKE_CA_H_

#include <stroke_msg.h>

#include "stroke_cred.h"

typedef struct stroke_ca_t stroke_ca_t;

struct stroke_ca_t {

	credential_set_t set;

	void (*add)(stroke_ca_t *this, stroke_msg_t *msg);

	void (*del)(stroke_ca_t *this, stroke_msg_t *msg);

	void (*list)(stroke_ca_t *this, stroke_msg_t *msg, FILE *out);

	void (*check_for_hash_and_url)(stroke_ca_t *this, certificate_t* cert);

	void (*destroy)(stroke_ca_t *this);
};

stroke_ca_t *stroke_ca_create(stroke_cred_t *cred);

#endif 
