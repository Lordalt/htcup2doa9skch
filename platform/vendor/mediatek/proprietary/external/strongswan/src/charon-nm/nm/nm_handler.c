/*
 * Copyright (C) 2009 Martin Willi
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

#include "nm_handler.h"

#include <daemon.h>

typedef struct private_nm_handler_t private_nm_handler_t;

struct private_nm_handler_t {

	nm_handler_t public;

	linked_list_t *dns;

	linked_list_t *nbns;
};

METHOD(attribute_handler_t, handle, bool,
	private_nm_handler_t *this, identification_t *server,
	configuration_attribute_type_t type, chunk_t data)
{
	linked_list_t *list;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			list = this->dns;
			break;
		case INTERNAL_IP4_NBNS:
			list = this->nbns;
			break;
		default:
			return FALSE;
	}
	if (data.len != 4)
	{
		return FALSE;
	}
	list->insert_last(list, chunk_clone(data).ptr);
	return TRUE;
}

static bool enumerate_nbns(enumerator_t *this,
						   configuration_attribute_type_t *type, chunk_t *data)
{
	*type = INTERNAL_IP4_NBNS;
	*data = chunk_empty;
	
	this->enumerate = (void*)return_false;
	return TRUE;
}

static bool enumerate_dns(enumerator_t *this,
						  configuration_attribute_type_t *type, chunk_t *data)
{
	*type = INTERNAL_IP4_DNS;
	*data = chunk_empty;
	
	this->enumerate = (void*)enumerate_nbns;
	return TRUE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t*,
	private_nm_handler_t *this, identification_t *server, linked_list_t *vips)
{
	if (vips->get_count(vips))
	{
		enumerator_t *enumerator;

		INIT(enumerator,
			
			.enumerate = (void*)enumerate_dns,
			.destroy = (void*)free,
		);
		return enumerator;
	}
	return enumerator_create_empty();
}

static bool filter_chunks(void* null, char **in, chunk_t *out)
{
	*out = chunk_create(*in, 4);
	return TRUE;
}

METHOD(nm_handler_t, create_enumerator, enumerator_t*,
	private_nm_handler_t *this, configuration_attribute_type_t type)
{
	linked_list_t *list;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			list = this->dns;
			break;
		case INTERNAL_IP4_NBNS:
			list = this->nbns;
			break;
		default:
			return enumerator_create_empty();
	}
	return enumerator_create_filter(list->create_enumerator(list),
						(void*)filter_chunks, NULL, NULL);
}

METHOD(nm_handler_t, reset, void,
	private_nm_handler_t *this)
{
	void *data;

	while (this->dns->remove_last(this->dns, (void**)&data) == SUCCESS)
	{
		free(data);
	}
	while (this->nbns->remove_last(this->nbns, (void**)&data) == SUCCESS)
	{
		free(data);
	}
}

METHOD(nm_handler_t, destroy, void,
	private_nm_handler_t *this)
{
	reset(this);
	this->dns->destroy(this->dns);
	this->nbns->destroy(this->nbns);
	free(this);
}

nm_handler_t *nm_handler_create()
{
	private_nm_handler_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = nop,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.create_enumerator = _create_enumerator,
			.reset = _reset,
			.destroy = _destroy,
		},
		.dns = linked_list_create(),
		.nbns = linked_list_create(),
	);

	return &this->public;
}

