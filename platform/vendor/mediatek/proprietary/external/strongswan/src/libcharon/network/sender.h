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


#ifndef SENDER_H_
#define SENDER_H_

typedef struct sender_t sender_t;

#include <library.h>
#include <networking/packet.h>

struct sender_t {

	void (*send) (sender_t *this, packet_t *packet);

	void (*send_no_marker) (sender_t *this, packet_t *packet);

	void (*flush)(sender_t *this);

	void (*destroy) (sender_t *this);
};

sender_t * sender_create(void);

#endif 
