/*
 * Copyright (C) 2010 Tobias Brunner
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


#ifndef MAEMO_SERVICE_H_
#define MAEMO_SERVICE_H_

#include <bus/listeners/listener.h>

typedef struct maemo_service_t maemo_service_t;

struct maemo_service_t {

	listener_t listener;

	void (*destroy)(maemo_service_t *this);
};

maemo_service_t *maemo_service_create();

#endif 
