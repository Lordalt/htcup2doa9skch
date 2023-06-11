/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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


#ifndef TKM_LISTENER_H_
#define TKM_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct tkm_listener_t tkm_listener_t;

struct tkm_listener_t {

	listener_t listener;

	void (*destroy)(tkm_listener_t *this);
};

tkm_listener_t *tkm_listener_create();

#endif 
