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


#ifndef CERTEXPIRE_EXPORT_H_
#define CERTEXPIRE_EXPORT_H_

typedef struct certexpire_export_t certexpire_export_t;

#include <collections/linked_list.h>

struct certexpire_export_t {

	void (*add)(certexpire_export_t *this, linked_list_t *trustchain, bool local);

	void (*destroy)(certexpire_export_t *this);
};

certexpire_export_t *certexpire_export_create();

#endif 
