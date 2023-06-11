/*
 * Copyright (C) 2009 Martin Willi
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


#ifndef HA_KERNEL_H_
#define HA_KERNEL_H_

typedef struct ha_kernel_t ha_kernel_t;

#include "ha_segments.h"

struct ha_kernel_t {

	u_int (*get_segment)(ha_kernel_t *this, host_t *host);

	u_int (*get_segment_spi)(ha_kernel_t *this, host_t *host, u_int32_t spi);

	u_int (*get_segment_int)(ha_kernel_t *this, int n);

	void (*activate)(ha_kernel_t *this, u_int segment);

	void (*deactivate)(ha_kernel_t *this, u_int segment);

	void (*destroy)(ha_kernel_t *this);
};

ha_kernel_t *ha_kernel_create(u_int count);

#endif 
