/*
 * Copyright (C) 2012 Reto Guadagnini
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


#ifndef IPSECKEY_CRED_H_
#define IPSECKEY_CRED_H_

#include <credentials/credential_set.h>
#include <resolver/resolver.h>

typedef struct ipseckey_cred_t ipseckey_cred_t;

struct ipseckey_cred_t {

	credential_set_t set;

	void (*destroy)(ipseckey_cred_t *this);
};

ipseckey_cred_t *ipseckey_cred_create(resolver_t *res);

#endif 
