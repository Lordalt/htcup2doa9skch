/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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


#ifndef TKM_PRIVATE_KEY_H_
#define TKM_PRIVATE_KEY_H_

#include <credentials/keys/private_key.h>

typedef struct tkm_private_key_t tkm_private_key_t;

struct tkm_private_key_t {

	private_key_t key;
};

tkm_private_key_t *tkm_private_key_init(identification_t * const id);

#endif 
