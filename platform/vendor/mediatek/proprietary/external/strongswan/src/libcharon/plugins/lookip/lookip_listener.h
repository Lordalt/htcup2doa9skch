/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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


#ifndef LOOKIP_LISTENER_H_
#define LOOKIP_LISTENER_H_

#include <bus/listeners/listener.h>

typedef struct lookip_listener_t lookip_listener_t;

typedef bool (*lookip_callback_t)(void *user, bool up, host_t *vip,
								  host_t *other, identification_t *id,
								  char *name, u_int unique_id);

struct lookip_listener_t {

	listener_t listener;

	int (*lookup)(lookip_listener_t *this, host_t *vip,
				  lookip_callback_t cb, void *user);

	void (*add_listener)(lookip_listener_t *this,
						 lookip_callback_t cb, void *user);

	void (*remove_listener)(lookip_listener_t *this, void *user);

	void (*destroy)(lookip_listener_t *this);
};

lookip_listener_t *lookip_listener_create();

#endif 
