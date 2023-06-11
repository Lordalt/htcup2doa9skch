/*
 * Copyright (C) 2009 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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

#ifndef COWFS_H
#define COWFS_H

#include <library.h>

typedef struct cowfs_t cowfs_t;

struct cowfs_t {

	bool (*add_overlay)(cowfs_t *this, char *path);

	bool (*del_overlay)(cowfs_t *this, char *path);

	bool (*pop_overlay)(cowfs_t *this);

	void (*destroy) (cowfs_t *this);
};

cowfs_t *cowfs_create(char *master, char *host, char *mount);

#endif 

