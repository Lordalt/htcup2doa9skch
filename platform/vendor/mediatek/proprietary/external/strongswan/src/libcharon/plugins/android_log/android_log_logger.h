/*
 * Copyright (C) 2010 Tobias Brunner
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


#ifndef ANDROID_LOG_LOGGER_H_
#define ANDROID_LOG_LOGGER_H_

#include <bus/bus.h>

typedef struct android_log_logger_t android_log_logger_t;

struct android_log_logger_t {

	logger_t logger;

	void (*destroy)(android_log_logger_t *this);

};

android_log_logger_t *android_log_logger_create();

#endif 
