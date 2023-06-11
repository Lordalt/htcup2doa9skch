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


#ifndef SQL_LOGGER_H_
#define SQL_LOGGER_H_

#include <bus/bus.h>
#include <database/database.h>

typedef struct sql_logger_t sql_logger_t;

struct sql_logger_t {

	logger_t logger;

	void (*destroy)(sql_logger_t *this);
};

sql_logger_t *sql_logger_create(database_t *db);

#endif 
