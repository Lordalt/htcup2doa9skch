/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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


#ifndef HA_ATTRIBUTE_H_
#define HA_ATTRIBUTE_H_

#include "ha_kernel.h"
#include "ha_segments.h"

#include <attributes/attribute_provider.h>

typedef struct ha_attribute_t ha_attribute_t;

struct ha_attribute_t {

	attribute_provider_t provider;

	void (*reserve)(ha_attribute_t *this, char *name, host_t *address);

	void (*destroy)(ha_attribute_t *this);
};

ha_attribute_t *ha_attribute_create(ha_kernel_t *kernel, ha_segments_t *segments);

#endif 
