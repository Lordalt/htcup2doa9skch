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


#ifndef SQL_CONFIG_H_
#define SQL_CONFIG_H_

#include <config/backend.h>
#include <database/database.h>

typedef struct sql_config_t sql_config_t;

struct sql_config_t {

	backend_t backend;

	void (*destroy)(sql_config_t *this);
};

sql_config_t *sql_config_create(database_t *db);

#endif 
