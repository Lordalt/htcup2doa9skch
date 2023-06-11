/*
 * Copyright (C) 2010-2011 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
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


#ifndef ANDROID_DNS_HANDLER_H_
#define ANDROID_DNS_HANDLER_H_

#include <attributes/attribute_handler.h>

typedef struct android_dns_handler_t android_dns_handler_t;

struct android_dns_handler_t {

	attribute_handler_t handler;

	void (*destroy)(android_dns_handler_t *this);
};

android_dns_handler_t *android_dns_handler_create();

#endif 
