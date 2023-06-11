/*
 * Copyright (C) 2007-2008 Martin Willi
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


#ifndef MEDSRV_CREDS_H_
#define MEDSRV_CREDS_H_

#include <credentials/credential_set.h>
#include <database/database.h>

typedef struct medsrv_creds_t medsrv_creds_t;

struct medsrv_creds_t {

	credential_set_t set;

	void (*destroy)(medsrv_creds_t *this);
};

medsrv_creds_t *medsrv_creds_create(database_t *database);

#endif 
