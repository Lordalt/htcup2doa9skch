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

#include "eap_radius_xauth.h"
#include "eap_radius_plugin.h"
#include "eap_radius.h"
#include "eap_radius_forward.h"

#include <daemon.h>
#include <radius_client.h>
#include <collections/array.h>


typedef struct private_eap_radius_xauth_t private_eap_radius_xauth_t;
typedef struct xauth_round_t xauth_round_t;

struct xauth_round_t {
	
	configuration_attribute_type_t type;
	
	char *message;
};

struct private_eap_radius_xauth_t {

	eap_radius_xauth_t public;

	identification_t *server;

	identification_t *peer;

	radius_client_t *client;

	array_t *rounds;

	xauth_round_t round;

	chunk_t pass;
};

static bool build_round(private_eap_radius_xauth_t *this, cp_payload_t *cp)
{
	if (!array_remove(this->rounds, ARRAY_HEAD, &this->round))
	{
		return FALSE;
	}
	cp->add_attribute(cp, configuration_attribute_create_chunk(
					CONFIGURATION_ATTRIBUTE_V1, this->round.type, chunk_empty));

	if (this->round.message && strlen(this->round.message))
	{
		cp->add_attribute(cp, configuration_attribute_create_chunk(
					CONFIGURATION_ATTRIBUTE_V1, XAUTH_MESSAGE,
					chunk_from_str(this->round.message)));
	}
	return TRUE;
}

METHOD(xauth_method_t, initiate, status_t,
	private_eap_radius_xauth_t *this, cp_payload_t **out)
{
	cp_payload_t *cp;

	cp = cp_payload_create_type(CONFIGURATION_V1, CFG_REQUEST);
	
	cp->add_attribute(cp, configuration_attribute_create_chunk(
				CONFIGURATION_ATTRIBUTE_V1, XAUTH_USER_NAME, chunk_empty));

	if (build_round(this, cp))
	{
		*out = cp;
		return NEED_MORE;
	}
	cp->destroy(cp);
	return FAILED;
}

static status_t verify_radius(private_eap_radius_xauth_t *this)
{
	radius_message_t *request, *response;
	status_t status = FAILED;

	request = radius_message_create(RMC_ACCESS_REQUEST);
	request->add(request, RAT_USER_NAME, this->peer->get_encoding(this->peer));
	request->add(request, RAT_USER_PASSWORD, this->pass);

	eap_radius_build_attributes(request);
	eap_radius_forward_from_ike(request);

	response = this->client->request(this->client, request);
	if (response)
	{
		eap_radius_forward_to_ike(response);
		switch (response->get_code(response))
		{
			case RMC_ACCESS_ACCEPT:
				eap_radius_process_attributes(response);
				status = SUCCESS;
				break;
			case RMC_ACCESS_CHALLENGE:
				DBG1(DBG_IKE, "RADIUS Access-Challenge not supported");
				
			case RMC_ACCESS_REJECT:
			default:
				DBG1(DBG_IKE, "RADIUS authentication of '%Y' failed",
					 this->peer);
				break;
		}
		response->destroy(response);
	}
	else
	{
		eap_radius_handle_timeout(NULL);
	}
	request->destroy(request);
	return status;
}

METHOD(xauth_method_t, process, status_t,
	private_eap_radius_xauth_t *this, cp_payload_t *in, cp_payload_t **out)
{
	configuration_attribute_t *attr;
	enumerator_t *enumerator;
	identification_t *id;
	cp_payload_t *cp;
	chunk_t user = chunk_empty, pass = chunk_empty;

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &attr))
	{
		if (attr->get_type(attr) == XAUTH_USER_NAME)
		{
			user = attr->get_chunk(attr);
		}
		else if (attr->get_type(attr) == this->round.type)
		{
			pass = attr->get_chunk(attr);
			pass.len = strnlen(pass.ptr, pass.len);
		}
	}
	enumerator->destroy(enumerator);

	if (!pass.ptr)
	{
		DBG1(DBG_IKE, "peer did not respond to our XAuth %N request",
			 configuration_attribute_type_names, this->round.type);
		return FAILED;
	}
	this->pass = chunk_cat("mc", this->pass, pass);
	if (user.len)
	{
		id = identification_create_from_data(user);
		if (!id)
		{
			DBG1(DBG_IKE, "failed to parse provided XAuth username");
			return FAILED;
		}
		this->peer->destroy(this->peer);
		this->peer = id;
	}

	if (array_count(this->rounds) == 0)
	{
		return verify_radius(this);
	}
	cp = cp_payload_create_type(CONFIGURATION_V1, CFG_REQUEST);
	if (build_round(this, cp))
	{
		*out = cp;
		return NEED_MORE;
	}
	cp->destroy(cp);
	return FAILED;
}

METHOD(xauth_method_t, get_identity, identification_t*,
	private_eap_radius_xauth_t *this)
{
	return this->peer;
}

static bool parse_rounds(private_eap_radius_xauth_t *this, char *profile)
{
	struct {
		char *str;
		configuration_attribute_type_t type;
	} map[] = {
		{ "password",		XAUTH_USER_PASSWORD,	},
		{ "passcode",		XAUTH_PASSCODE,			},
		{ "nextpin",		XAUTH_NEXT_PIN,			},
		{ "answer",			XAUTH_ANSWER,			},
	};
	enumerator_t *enumerator;
	char *type, *message;
	xauth_round_t round;
	int i;

	if (!profile || strlen(profile) == 0)
	{
		
		round.type = XAUTH_USER_PASSWORD;
		round.message = NULL;
		array_insert(this->rounds, ARRAY_TAIL, &round);
		return TRUE;
	}

	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
							"%s.plugins.eap-radius.xauth.%s", lib->ns, profile);
	while (enumerator->enumerate(enumerator, &type, &message))
	{
		bool invalid = TRUE;

		for (i = 0; i < countof(map); i++)
		{
			if (strcaseeq(map[i].str, type))
			{
				round.type = map[i].type;
				round.message = message;
				array_insert(this->rounds, ARRAY_TAIL, &round);
				invalid = FALSE;
				break;
			}
		}
		if (invalid)
		{
			DBG1(DBG_CFG, "invalid XAuth round type: '%s'", type);
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);

	if (array_count(this->rounds) == 0)
	{
		DBG1(DBG_CFG, "XAuth configuration profile '%s' invalid", profile);
		return FALSE;
	}
	return TRUE;
}

METHOD(xauth_method_t, destroy, void,
	private_eap_radius_xauth_t *this)
{
	DESTROY_IF(this->client);
	chunk_clear(&this->pass);
	array_destroy(this->rounds);
	this->server->destroy(this->server);
	this->peer->destroy(this->peer);
	free(this);
}

eap_radius_xauth_t *eap_radius_xauth_create_server(identification_t *server,
												   identification_t *peer,
												   char *profile)
{
	private_eap_radius_xauth_t *this;

	INIT(this,
		.public = {
			.xauth_method = {
				.initiate = _initiate,
				.process = _process,
				.get_identity = _get_identity,
				.destroy = _destroy,
			},
		},
		.server = server->clone(server),
		.peer = peer->clone(peer),
		.client = eap_radius_create_client(),
		.rounds = array_create(sizeof(xauth_round_t), 0),
	);

	if (!parse_rounds(this, profile))
	{
		destroy(this);
		return NULL;
	}
	if (!this->client)
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
