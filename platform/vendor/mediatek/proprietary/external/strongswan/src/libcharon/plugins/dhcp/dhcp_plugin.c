/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
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

#include "dhcp_plugin.h"

#include <hydra.h>
#include <daemon.h>
#include <plugins/plugin_feature.h>

#include "dhcp_socket.h"
#include "dhcp_provider.h"

typedef struct private_dhcp_plugin_t private_dhcp_plugin_t;

struct private_dhcp_plugin_t {

	dhcp_plugin_t public;

	dhcp_socket_t *socket;

	dhcp_provider_t *provider;
};

METHOD(plugin_t, get_name, char*,
	private_dhcp_plugin_t *this)
{
	return "dhcp";
}

static bool plugin_cb(private_dhcp_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		this->socket = dhcp_socket_create();

		if (!this->socket)
		{
			return FALSE;
		}
		this->provider = dhcp_provider_create(this->socket);
		hydra->attributes->add_provider(hydra->attributes,
										&this->provider->provider);
	}
	else
	{
		hydra->attributes->remove_provider(hydra->attributes,
										   &this->provider->provider);
		this->provider->destroy(this->provider);
		this->socket->destroy(this->socket);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_dhcp_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "dhcp"),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_dhcp_plugin_t *this)
{
	free(this);
}

plugin_t *dhcp_plugin_create()
{
	private_dhcp_plugin_t *this;

	if (!lib->caps->check(lib->caps, CAP_NET_BIND_SERVICE))
	{	
		DBG1(DBG_NET, "dhcp plugin requires CAP_NET_BIND_SERVICE capability");
		return NULL;
	}
	else if (!lib->caps->keep(lib->caps, CAP_NET_RAW))
	{	
		DBG1(DBG_NET, "dhcp plugin requires CAP_NET_RAW capability");
		return NULL;
	}

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
