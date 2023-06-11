/*
 * Copyright (C) 2012 Tobias Brunner
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


#ifndef STROKE_CRED_H_
#define STROKE_CRED_H_

#include <stdio.h>

#include <stroke_msg.h>
#include <credentials/credential_set.h>
#include <credentials/certificates/certificate.h>
#include <collections/linked_list.h>

typedef struct stroke_cred_t stroke_cred_t;

struct stroke_cred_t {

	credential_set_t set;

	void (*reread)(stroke_cred_t *this, stroke_msg_t *msg, FILE *prompt);

	certificate_t* (*load_ca)(stroke_cred_t *this, char *filename);

	certificate_t* (*load_peer)(stroke_cred_t *this, char *filename);

	certificate_t* (*load_pubkey)(stroke_cred_t *this, char *filename,
								  identification_t *identity);

	void (*add_shared)(stroke_cred_t *this, shared_key_t *shared,
					   linked_list_t *owners);

	void (*cachecrl)(stroke_cred_t *this, bool enabled);

	void (*destroy)(stroke_cred_t *this);
};

stroke_cred_t *stroke_cred_create();

#endif 
