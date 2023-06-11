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


#ifndef DUPLICHECK_NOTIFY_H_
#define DUPLICHECK_NOTIFY_H_

#include <utils/identification.h>

typedef struct duplicheck_notify_t duplicheck_notify_t;

struct duplicheck_notify_t {

	void (*send)(duplicheck_notify_t *this, identification_t *id);

	void (*destroy)(duplicheck_notify_t *this);
};

duplicheck_notify_t *duplicheck_notify_create();

#endif 
