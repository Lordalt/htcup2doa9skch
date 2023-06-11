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


#ifndef LOAD_TESTER_DIFFIE_HELLMAN_H_
#define LOAD_TESTER_DIFFIE_HELLMAN_H_

#include <crypto/diffie_hellman.h>

typedef struct load_tester_diffie_hellman_t load_tester_diffie_hellman_t;

struct load_tester_diffie_hellman_t {

	diffie_hellman_t dh;
};

load_tester_diffie_hellman_t *load_tester_diffie_hellman_create(
												diffie_hellman_group_t group);

#endif 
