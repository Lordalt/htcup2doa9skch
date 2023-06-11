/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
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


#ifndef SYS_LOGGER_H_
#define SYS_LOGGER_H_

#include <bus/listeners/logger.h>

typedef struct sys_logger_t sys_logger_t;

struct sys_logger_t {

	logger_t logger;

	void (*set_level) (sys_logger_t *this, debug_t group, level_t level);

	void (*set_options) (sys_logger_t *this, bool ike_name);

	void (*destroy) (sys_logger_t *this);
};

sys_logger_t *sys_logger_create(int facility);

#endif 
