/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "duplicheck_plugin.h"

#include "duplicheck_notify.h"
#include "duplicheck_listener.h"

#include <daemon.h>

typedef struct private_duplicheck_plugin_t private_duplicheck_plugin_t;

struct private_duplicheck_plugin_t {

	duplicheck_plugin_t public;

	duplicheck_listener_t *listener;

	duplicheck_notify_t *notify;
};

METHOD(plugin_t, get_name, char*,
	private_duplicheck_plugin_t *this)
{
	return "duplicheck";
}

static bool plugin_cb(private_duplicheck_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		charon->bus->add_listener(charon->bus, &this->listener->listener);
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_duplicheck_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "duplicheck"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_duplicheck_plugin_t *this)
{
	this->notify->destroy(this->notify);
	this->listener->destroy(this->listener);
	free(this);
}

plugin_t *duplicheck_plugin_create()
{
	private_duplicheck_plugin_t *this;

	if (!lib->settings->get_bool(lib->settings,
								 "%s.plugins.duplicheck.enable", TRUE, lib->ns))
	{
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
		.notify = duplicheck_notify_create(),
	);

	if (!this->notify)
	{
		free(this);
		return NULL;
	}
	this->listener = duplicheck_listener_create(this->notify);

	return &this->public.plugin;
}
