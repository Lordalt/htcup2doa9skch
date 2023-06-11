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

#include <encoding/payloads/unknown_payload.h>

typedef struct private_set_ike_spi_t private_set_ike_spi_t;

struct private_set_ike_spi_t {

	hook_t hook;

	bool req;

	int id;

	u_int64_t spii;

	u_int64_t spir;
};

METHOD(listener_t, message, bool,
	private_set_ike_spi_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (!incoming && plain &&
		message->get_request(message) == this->req &&
		message->get_message_id(message) == this->id)
	{
		ike_sa_id_t *id;

		DBG1(DBG_CFG, "setting IKE SPIs to: 0x%llx/0x%llx",
			 this->spii, this->spir);

		id = message->get_ike_sa_id(message);
		id->set_initiator_spi(id, this->spii);
		id->set_responder_spi(id, this->spir);
	}
	return TRUE;
}

METHOD(hook_t, destroy, void,
	private_set_ike_spi_t *this)
{
	free(this);
}

hook_t *set_ike_spi_hook_create(char *name)
{
	private_set_ike_spi_t *this;

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
		.spii = strtoull(conftest->test->get_str(conftest->test,
										"hooks.%s.spii", "0", name), NULL, 16),
		.spir = strtoull(conftest->test->get_str(conftest->test,
										"hooks.%s.spir", "0", name), NULL, 16),
	);

	return &this->hook;
}
