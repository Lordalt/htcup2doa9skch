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


#ifndef SQL_CRED_H_
#define SQL_CRED_H_

#include <credentials/credential_set.h>
#include <database/database.h>

typedef struct sql_cred_t sql_cred_t;

struct sql_cred_t {

	credential_set_t set;

	void (*destroy)(sql_cred_t *this);
};

sql_cred_t *sql_cred_create(database_t *db);

#endif 
