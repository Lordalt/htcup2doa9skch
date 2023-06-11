/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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


#ifndef RECEIVER_H_
#define RECEIVER_H_

typedef struct receiver_t receiver_t;

#include <library.h>
#include <networking/host.h>
#include <networking/packet.h>

typedef void (*receiver_esp_cb_t)(void *data, packet_t *packet);

struct receiver_t {

	void (*add_esp_cb)(receiver_t *this, receiver_esp_cb_t callback,
					   void *data);

	void (*del_esp_cb)(receiver_t *this, receiver_esp_cb_t callback);

	void (*destroy)(receiver_t *this);
};

receiver_t * receiver_create(void);

#endif 
