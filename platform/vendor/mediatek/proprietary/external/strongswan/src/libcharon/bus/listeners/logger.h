/*
 * Copyright (C) 2012 Tobias Brunner
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


#ifndef LOGGER_H_
#define LOGGER_H_

typedef struct logger_t logger_t;

#include <bus/bus.h>

struct logger_t {

	void (*log)(logger_t *this, debug_t group, level_t level, int thread,
				ike_sa_t *ike_sa, const char *message);

	void (*vlog)(logger_t *this, debug_t group, level_t level, int thread,
				 ike_sa_t *ike_sa, const char *fmt, va_list args);

	level_t (*get_level)(logger_t *this, debug_t group);
};

#endif 
