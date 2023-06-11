/*
 * Copyright (C) 2009 Tobias Brunner
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


#ifndef KERNEL_PFROUTE_NET_H_
#define KERNEL_PFROUTE_NET_H_

#include <kernel/kernel_net.h>

typedef struct kernel_pfroute_net_t kernel_pfroute_net_t;

struct kernel_pfroute_net_t {

	kernel_net_t interface;
};

kernel_pfroute_net_t *kernel_pfroute_net_create();

#endif 
