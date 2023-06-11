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


#ifndef ERROR_NOTIFY_LISTENER_H_
#define ERROR_NOTIFY_LISTENER_H_

typedef struct error_notify_listener_t error_notify_listener_t;

#include <bus/listeners/listener.h>

#include "error_notify_socket.h"

struct error_notify_listener_t {

	listener_t listener;

	void (*destroy)(error_notify_listener_t *this);
};

error_notify_listener_t *error_notify_listener_create(error_notify_socket_t *s);

#endif 
