/*
 * Copyright (C) 2013 Tobias Brunner
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


#ifndef KERNEL_LIBIPSEC_ROUTER_H_
#define KERNEL_LIBIPSEC_ROUTER_H_

#include <kernel/kernel_listener.h>

typedef struct kernel_libipsec_router_t kernel_libipsec_router_t;

struct kernel_libipsec_router_t {

	kernel_listener_t listener;

	char *(*get_tun_name)(kernel_libipsec_router_t *this, host_t *vip);

	void (*destroy)(kernel_libipsec_router_t *this);
};

extern kernel_libipsec_router_t *router;

kernel_libipsec_router_t *kernel_libipsec_router_create();

#endif 
