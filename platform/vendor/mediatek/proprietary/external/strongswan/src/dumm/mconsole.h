/*
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

#ifndef MCONSOLE_H
#define MCONSOLE_H

#include <library.h>

typedef struct mconsole_t mconsole_t;

struct mconsole_t {

	bool (*add_iface)(mconsole_t *this, char *guest, char *host);

	bool (*del_iface)(mconsole_t *this, char *guest);

	int (*exec)(mconsole_t *this, void(*cb)(void*,char*,size_t), void *data,
				char *cmd);

	void (*destroy) (mconsole_t *this);
};

mconsole_t *mconsole_create(char *notify, void(*idle)(void));

#endif 

