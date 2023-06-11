/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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


#ifndef WHITELIST_LISTENER_H_
#define WHITELIST_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct whitelist_listener_t whitelist_listener_t;

struct whitelist_listener_t {

	listener_t listener;

	void (*add)(whitelist_listener_t *this, identification_t *id);

	void (*remove)(whitelist_listener_t *this, identification_t *id);

	enumerator_t* (*create_enumerator)(whitelist_listener_t *this);

	void (*flush)(whitelist_listener_t *this, identification_t *id);

	void (*set_active)(whitelist_listener_t *this, bool enable);

	void (*destroy)(whitelist_listener_t *this);
};

whitelist_listener_t *whitelist_listener_create();

#endif 
