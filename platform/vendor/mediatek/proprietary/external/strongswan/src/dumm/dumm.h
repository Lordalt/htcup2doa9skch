/*
 * Copyright (C) 2008-2009 Tobias Brunner
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

#ifndef DUMM_H
#define DUMM_H

#include <signal.h>

#include <library.h>
#include <collections/enumerator.h>

#include "guest.h"
#include "bridge.h"

typedef struct dumm_t dumm_t;

struct dumm_t {

	guest_t* (*create_guest) (dumm_t *this, char *name, char *kernel,
							  char *master, char *args);

	enumerator_t* (*create_guest_enumerator) (dumm_t *this);

	void (*delete_guest) (dumm_t *this, guest_t *guest);

	bridge_t* (*create_bridge)(dumm_t *this, char *name);

	enumerator_t* (*create_bridge_enumerator)(dumm_t *this);

	void (*delete_bridge) (dumm_t *this, bridge_t *bridge);

	bool (*add_overlay)(dumm_t *this, char *dir);

	bool (*del_overlay)(dumm_t *this, char *dir);

	bool (*pop_overlay)(dumm_t *this);

	bool (*load_template)(dumm_t *this, char *name);

	enumerator_t* (*create_template_enumerator)(dumm_t *this);

	void (*destroy) (dumm_t *this);
};

dumm_t *dumm_create(char *dir);

#endif 

