/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "hook.h"

typedef struct private_unsort_message_t private_unsort_message_t;

struct private_unsort_message_t {

	hook_t hook;

	bool req;

	int id;

	char *order;
};

METHOD(listener_t, message, bool,
	private_unsort_message_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		enumerator_t *enumerator, *order;
		linked_list_t *list;
		payload_type_t type;
		payload_t *payload;
		char *name;

		list = linked_list_create();
		enumerator = message->create_payload_enumerator(message);
		while (enumerator->enumerate(enumerator, &payload))
		{
			message->remove_payload_at(message, enumerator);
			list->insert_last(list, payload);
		}
		enumerator->destroy(enumerator);

		order = enumerator_create_token(this->order, ", ", " ");
		while (order->enumerate(order, &name))
		{
			type = enum_from_name(payload_type_short_names, name);
			if (type != -1)
			{
				enumerator = list->create_enumerator(list);
				while (enumerator->enumerate(enumerator, &payload))
				{
					if (payload->get_type(payload) == type)
					{
						list->remove_at(list, enumerator);
						message->add_payload(message, payload);
					}
				}
				enumerator->destroy(enumerator);
			}
			else
			{
				DBG1(DBG_CFG, "unknown payload to sort: '%s', skipped", name);
			}
		}
		order->destroy(order);

		while (list->remove_first(list, (void**)&payload) == SUCCESS)
		{
			message->add_payload(message, payload);
		}
		list->destroy(list);

		message->disable_sort(message);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_unsort_message_t *this)
{
	free(this);
}

hook_t *unsort_message_hook_create(char *name)
{
	private_unsort_message_t *this;

	INIT(this,
		.hook = {
			.listener = {
				.message = _message,
			},
			.destroy = _destroy,
		},
		.req = conftest->test->get_bool(conftest->test,
										"hooks.%s.request", TRUE, name),
		.id = conftest->test->get_int(conftest->test,
										"hooks.%s.id", 0, name),
		.order = conftest->test->get_str(conftest->test,
										"hooks.%s.order", "", name),
	);

	return &this->hook;
}
