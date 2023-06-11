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


#ifndef LOOKIP_SOCKET_H_
#define LOOKIP_SOCKET_H_

#include "lookip_listener.h"

typedef struct lookip_socket_t lookip_socket_t;

struct lookip_socket_t {

	void (*destroy)(lookip_socket_t *this);
};

lookip_socket_t *lookip_socket_create(lookip_listener_t *listener);

#endif 
