/*
 * Copyright (C) 2012-2013 Tobias Brunner
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


#ifndef KERNEL_LIBIPSEC_IPSEC_H_
#define KERNEL_LIBIPSEC_IPSEC_H_

#include <library.h>
#include <kernel/kernel_ipsec.h>

typedef struct kernel_libipsec_ipsec_t kernel_libipsec_ipsec_t;

struct kernel_libipsec_ipsec_t {

	kernel_ipsec_t interface;
};

kernel_libipsec_ipsec_t *kernel_libipsec_ipsec_create();

#endif 
