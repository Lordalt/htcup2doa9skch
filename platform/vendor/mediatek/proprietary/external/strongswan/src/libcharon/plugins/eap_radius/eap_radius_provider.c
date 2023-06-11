/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "eap_radius_provider.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <threading/mutex.h>

typedef struct private_eap_radius_provider_t private_eap_radius_provider_t;
typedef struct private_listener_t private_listener_t;

struct private_listener_t {

	listener_t public;

	hashtable_t *unclaimed;

	hashtable_t *claimed;

	mutex_t *mutex;
};

struct private_eap_radius_provider_t {

	eap_radius_provider_t public;

	private_listener_t listener;
};

static eap_radius_provider_t *singleton = NULL;

typedef struct {
	
	configuration_attribute_type_t type;
	
	chunk_t data;
} attr_t;

static void destroy_attr(attr_t *this)
{
	free(this->data.ptr);
	free(this);
}

typedef struct {
	
	uintptr_t id;
	
	linked_list_t *addrs;
	
	linked_list_t *attrs;
} entry_t;

static void destroy_entry(entry_t *this)
{
	this->addrs->destroy_offset(this->addrs, offsetof(host_t, destroy));
	this->attrs->destroy_function(this->attrs, (void*)destroy_attr);
	free(this);
}

static entry_t* get_or_create_entry(hashtable_t *hashtable, uintptr_t id)
{
	entry_t *entry;

	entry = hashtable->get(hashtable, (void*)id);
	if (!entry)
	{
		INIT(entry,
			.id = id,
			.addrs = linked_list_create(),
			.attrs = linked_list_create(),
		);
		hashtable->put(hashtable, (void*)id, entry);
	}
	return entry;
}

static void put_or_destroy_entry(hashtable_t *hashtable, entry_t *entry)
{
	if (entry->addrs->get_count(entry->addrs) > 0 ||
		entry->attrs->get_count(entry->attrs) > 0)
	{
		hashtable->put(hashtable, (void*)entry->id, entry);
	}
	else
	{
		destroy_entry(entry);
	}
}

static u_int hash(uintptr_t id)
{
	return id;
}

static bool equals(uintptr_t a, uintptr_t b)
{
	return a == b;
}

static void add_addr(private_eap_radius_provider_t *this,
					 hashtable_t *hashtable, uintptr_t id, host_t *host)
{
	entry_t *entry;

	entry = get_or_create_entry(hashtable, id);
	entry->addrs->insert_last(entry->addrs, host);
}

static host_t* remove_addr(private_eap_radius_provider_t *this,
						   hashtable_t *hashtable, uintptr_t id)
{
	entry_t *entry;
	host_t *addr = NULL;

	entry = hashtable->remove(hashtable, (void*)id);
	if (entry)
	{
		entry->addrs->remove_first(entry->addrs, (void**)&addr);
		put_or_destroy_entry(hashtable, entry);
	}
	return addr;
}

static void add_attr(private_eap_radius_provider_t *this,
					 hashtable_t *hashtable, uintptr_t id, attr_t *attr)
{
	entry_t *entry;

	entry = get_or_create_entry(hashtable, id);
	entry->attrs->insert_last(entry->attrs, attr);
}

static attr_t* remove_attr(private_eap_radius_provider_t *this,
						   hashtable_t *hashtable, uintptr_t id)
{
	entry_t *entry;
	attr_t *attr = NULL;

	entry = hashtable->remove(hashtable, (void*)id);
	if (entry)
	{
		entry->attrs->remove_first(entry->attrs, (void**)&attr);
		put_or_destroy_entry(hashtable, entry);
	}
	return attr;
}

static void release_unclaimed(private_listener_t *this, ike_sa_t *ike_sa)
{
	uintptr_t id;
	entry_t *entry;

	id = ike_sa->get_unique_id(ike_sa);
	this->mutex->lock(this->mutex);
	entry = this->unclaimed->remove(this->unclaimed, (void*)id);
	this->mutex->unlock(this->mutex);
	if (entry)
	{
		destroy_entry(entry);
	}
}

METHOD(listener_t, message_hook, bool,
	private_listener_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	if (plain && ike_sa->get_state(ike_sa) == IKE_ESTABLISHED &&
		!incoming && !message->get_request(message))
	{
		if ((ike_sa->get_version(ike_sa) == IKEV1 &&
			 message->get_exchange_type(message) == TRANSACTION) ||
			(ike_sa->get_version(ike_sa) == IKEV2 &&
			 message->get_exchange_type(message) == IKE_AUTH))
		{
			release_unclaimed(this, ike_sa);
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_listener_t *this, ike_sa_t *ike_sa, bool up)
{
	if (!up)
	{
		release_unclaimed(this, ike_sa);
	}
	return TRUE;
}

static void migrate_entry(hashtable_t *table, uintptr_t old, uintptr_t new)
{
	entry_t *entry;

	entry = table->remove(table, (void*)old);
	if (entry)
	{
		entry->id = new;
		entry = table->put(table, (void*)new, entry);
		if (entry)
		{	
			destroy_entry(entry);
		}
	}
}

METHOD(listener_t, ike_rekey, bool,
	private_listener_t *this, ike_sa_t *old, ike_sa_t *new)
{
	uintptr_t old_id, new_id;

	old_id = old->get_unique_id(old);
	new_id = new->get_unique_id(new);

	this->mutex->lock(this->mutex);

	migrate_entry(this->unclaimed, old_id, new_id);
	migrate_entry(this->claimed, old_id, new_id);

	this->mutex->unlock(this->mutex);

	return TRUE;
}

METHOD(attribute_provider_t, acquire_address, host_t*,
	private_eap_radius_provider_t *this, linked_list_t *pools,
	identification_t *id, host_t *requested)
{
	enumerator_t *enumerator;
	host_t *addr = NULL;
	ike_sa_t *ike_sa;
	uintptr_t sa;
	char *name;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return NULL;
	}
	sa = ike_sa->get_unique_id(ike_sa);

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		if (streq(name, "radius"))
		{
			this->listener.mutex->lock(this->listener.mutex);
			addr = remove_addr(this, this->listener.unclaimed, sa);
			if (addr)
			{
				add_addr(this, this->listener.claimed, sa, addr->clone(addr));
			}
			this->listener.mutex->unlock(this->listener.mutex);
			break;
		}
	}
	enumerator->destroy(enumerator);

	return addr;
}

METHOD(attribute_provider_t, release_address, bool,
	private_eap_radius_provider_t *this, linked_list_t *pools, host_t *address,
	identification_t *id)
{
	enumerator_t *enumerator;
	host_t *found = NULL;
	ike_sa_t *ike_sa;
	uintptr_t sa;
	char *name;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return FALSE;
	}
	sa = ike_sa->get_unique_id(ike_sa);

	enumerator = pools->create_enumerator(pools);
	while (enumerator->enumerate(enumerator, &name))
	{
		if (streq(name, "radius"))
		{
			this->listener.mutex->lock(this->listener.mutex);
			found = remove_addr(this, this->listener.claimed, sa);
			this->listener.mutex->unlock(this->listener.mutex);
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (found)
	{
		found->destroy(found);
		return TRUE;
	}
	return FALSE;
}

typedef struct {
	
	enumerator_t public;
	
	linked_list_t *list;
	
	attr_t *current;
} attribute_enumerator_t;


METHOD(enumerator_t, attribute_enumerate, bool,
	attribute_enumerator_t *this, configuration_attribute_type_t *type,
	chunk_t *data)
{
	if (this->current)
	{
		destroy_attr(this->current);
		this->current = NULL;
	}
	if (this->list->remove_first(this->list, (void**)&this->current) == SUCCESS)
	{
		*type = this->current->type;
		*data = this->current->data;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, attribute_destroy, void,
	attribute_enumerator_t *this)
{
	if (this->current)
	{
		destroy_attr(this->current);
	}
	this->list->destroy_function(this->list, (void*)destroy_attr);
	free(this);
}

METHOD(attribute_provider_t, create_attribute_enumerator, enumerator_t*,
	private_eap_radius_provider_t *this, linked_list_t *pools,
	identification_t *id, linked_list_t *vips)
{
	attribute_enumerator_t *enumerator;
	attr_t *attr;
	ike_sa_t *ike_sa;
	uintptr_t sa;

	ike_sa = charon->bus->get_sa(charon->bus);
	if (!ike_sa)
	{
		return NULL;
	}
	sa = ike_sa->get_unique_id(ike_sa);

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_attribute_enumerate,
			.destroy = _attribute_destroy,
		},
		.list = linked_list_create(),
	);

	
	this->listener.mutex->lock(this->listener.mutex);
	while (TRUE)
	{
		attr = remove_attr(this, this->listener.unclaimed, sa);
		if (!attr)
		{
			break;
		}
		enumerator->list->insert_last(enumerator->list, attr);
	}
	this->listener.mutex->unlock(this->listener.mutex);

	return &enumerator->public;
}

METHOD(eap_radius_provider_t, add_framed_ip, void,
	private_eap_radius_provider_t *this, u_int32_t id, host_t *ip)
{
	this->listener.mutex->lock(this->listener.mutex);
	add_addr(this, this->listener.unclaimed, id, ip);
	this->listener.mutex->unlock(this->listener.mutex);
}

METHOD(eap_radius_provider_t, add_attribute, void,
	private_eap_radius_provider_t *this, u_int32_t id,
	configuration_attribute_type_t type, chunk_t data)
{
	attr_t *attr;

	INIT(attr,
		.type = type,
		.data = chunk_clone(data),
	);
	this->listener.mutex->lock(this->listener.mutex);
	add_attr(this, this->listener.unclaimed, id, attr);
	this->listener.mutex->unlock(this->listener.mutex);
}

METHOD(eap_radius_provider_t, destroy, void,
	private_eap_radius_provider_t *this)
{
	singleton = NULL;
	charon->bus->remove_listener(charon->bus, &this->listener.public);
	this->listener.mutex->destroy(this->listener.mutex);
	this->listener.claimed->destroy(this->listener.claimed);
	this->listener.unclaimed->destroy(this->listener.unclaimed);
	free(this);
}

eap_radius_provider_t *eap_radius_provider_create()
{
	if (!singleton)
	{
		private_eap_radius_provider_t *this;

		INIT(this,
			.public = {
				.provider = {
					.acquire_address = _acquire_address,
					.release_address = _release_address,
					.create_attribute_enumerator = _create_attribute_enumerator,
				},
				.add_framed_ip = _add_framed_ip,
				.add_attribute = _add_attribute,
				.destroy = _destroy,
			},
			.listener = {
				.public = {
					.ike_updown = _ike_updown,
					.ike_rekey = _ike_rekey,
					.message = _message_hook,
				},
				.claimed = hashtable_create((hashtable_hash_t)hash,
										(hashtable_equals_t)equals, 32),
				.unclaimed = hashtable_create((hashtable_hash_t)hash,
										(hashtable_equals_t)equals, 32),
				.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
			},
		);

		charon->bus->add_listener(charon->bus, &this->listener.public);

		singleton = &this->public;
	}
	return singleton;
}

eap_radius_provider_t *eap_radius_provider_get()
{
	return singleton;
}
