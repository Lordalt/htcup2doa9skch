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


#ifndef HYDRA_H_
#define HYDRA_H_

typedef struct hydra_t hydra_t;

#include <attributes/attribute_manager.h>
#include <kernel_wrap/kernel_interface.h>

#include <library.h>

struct hydra_t {

	attribute_manager_t *attributes;

	kernel_interface_t *kernel_interface;
};

extern hydra_t *hydra;

bool libhydra_init();

void libhydra_deinit();

#endif 
