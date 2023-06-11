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


#ifndef CERTEXPIRE_LISTENER_H_
#define CERTEXPIRE_LISTENER_H_

#include <bus/listeners/listener.h>

#include "certexpire_export.h"

typedef struct certexpire_listener_t certexpire_listener_t;

struct certexpire_listener_t {

	listener_t listener;

	void (*destroy)(certexpire_listener_t *this);
};

certexpire_listener_t *certexpire_listener_create(certexpire_export_t *export);

#endif 
