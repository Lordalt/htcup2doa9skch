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


#ifndef LOAD_TESTER_IPSEC_H_
#define LOAD_TESTER_IPSEC_H_

#include <kernel/kernel_ipsec.h>

typedef struct load_tester_ipsec_t load_tester_ipsec_t;

struct load_tester_ipsec_t {

	kernel_ipsec_t interface;
};

load_tester_ipsec_t *load_tester_ipsec_create();

#endif 
