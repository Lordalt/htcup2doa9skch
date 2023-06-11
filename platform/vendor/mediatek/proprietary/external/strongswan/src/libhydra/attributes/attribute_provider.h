/*
 * Copyright (C) 2008 Martin Willi
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


#ifndef ATTRIBUTE_PROVIDER_H_
#define ATTRIBUTE_PROVIDER_H_

#include <networking/host.h>
#include <utils/identification.h>
#include <collections/linked_list.h>

typedef struct attribute_provider_t attribute_provider_t;

struct attribute_provider_t {

	host_t* (*acquire_address)(attribute_provider_t *this,
							   linked_list_t *pools, identification_t *id,
							   host_t *requested);
	bool (*release_address)(attribute_provider_t *this,
							linked_list_t *pools, host_t *address,
							identification_t *id);

	enumerator_t* (*create_attribute_enumerator)(attribute_provider_t *this,
									linked_list_t *pools, identification_t *id,
									linked_list_t *vips);
};

#endif 
