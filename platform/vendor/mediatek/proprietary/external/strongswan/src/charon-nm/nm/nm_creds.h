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


#ifndef NM_CREDS_H_
#define NM_CREDS_H_

#include <credentials/keys/private_key.h>
#include <credentials/credential_set.h>

typedef struct nm_creds_t nm_creds_t;

struct nm_creds_t {

	credential_set_t set;

	void (*add_certificate)(nm_creds_t *this, certificate_t *cert);

	void (*load_ca_dir)(nm_creds_t *this, char *dir);

	void (*set_username_password)(nm_creds_t *this, identification_t *id,
								  char *password);

	void (*set_key_password)(nm_creds_t *this, char *password);

	void (*set_pin)(nm_creds_t *this, chunk_t keyid, char *pin);

	void (*set_cert_and_key)(nm_creds_t *this, certificate_t *cert,
							 private_key_t *key);

	void (*clear)(nm_creds_t *this);

	void (*destroy)(nm_creds_t *this);
};

nm_creds_t *nm_creds_create();

#endif 
