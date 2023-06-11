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


#ifndef KERNEL_HANDLER_H_
#define KERNEL_HANDLER_H_

typedef struct kernel_handler_t kernel_handler_t;

#include <kernel_wrap/kernel_listener.h>

struct kernel_handler_t {

	kernel_listener_t listener;

	void (*destroy)(kernel_handler_t *this);

};

kernel_handler_t *kernel_handler_create();

#endif 
