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


#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

#include <bus/listeners/logger.h>

typedef struct file_logger_t file_logger_t;

struct file_logger_t {

	logger_t logger;

	void (*set_level) (file_logger_t *this, debug_t group, level_t level);

	void (*set_options) (file_logger_t *this, char *time_format, bool ike_name);

	void (*open) (file_logger_t *this, bool flush_line, bool append);

	void (*destroy) (file_logger_t *this);
};

file_logger_t *file_logger_create(char *filename);

#endif 
