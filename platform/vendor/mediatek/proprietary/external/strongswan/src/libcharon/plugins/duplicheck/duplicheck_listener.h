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


#ifndef DUPLICHECK_LISTENER_H_
#define DUPLICHECK_LISTENER_H_

#include "duplicheck_notify.h"

#include <bus/listeners/listener.h>

typedef struct duplicheck_listener_t duplicheck_listener_t;

struct duplicheck_listener_t {

	listener_t listener;

	void (*destroy)(duplicheck_listener_t *this);
};

duplicheck_listener_t *duplicheck_listener_create(duplicheck_notify_t *notify);

#endif 
