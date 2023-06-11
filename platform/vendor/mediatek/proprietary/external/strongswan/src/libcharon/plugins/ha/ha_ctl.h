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


#ifndef HA_CTL_H_
#define HA_CTL_H_

#include "ha_segments.h"
#include "ha_cache.h"

typedef struct ha_ctl_t ha_ctl_t;

struct ha_ctl_t {

	void (*destroy)(ha_ctl_t *this);
};

ha_ctl_t *ha_ctl_create(ha_segments_t *segments, ha_cache_t *cache);

#endif 
