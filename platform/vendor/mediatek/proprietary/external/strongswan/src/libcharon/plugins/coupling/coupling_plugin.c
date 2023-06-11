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

#include "coupling_plugin.h"

#include "coupling_validator.h"

#include <daemon.h>

typedef struct private_coupling_plugin_t private_coupling_plugin_t;

struct private_coupling_plugin_t {

	coupling_plugin_t public;

	coupling_validator_t *validator;
};

METHOD(plugin_t, get_name, char*,
	private_coupling_plugin_t *this)
{
	return "coupling";
}

static bool plugin_cb(private_coupling_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		this->validator = coupling_validator_create();

		if (!this->validator)
		{
			return FALSE;
		}
		lib->credmgr->add_validator(lib->credmgr, &this->validator->validator);
	}
	else
	{
		lib->credmgr->remove_validator(lib->credmgr,
									   &this->validator->validator);
		this->validator->destroy(this->validator);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_coupling_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "coupling"),
				PLUGIN_SDEPEND(HASHER, HASH_SHA1),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_coupling_plugin_t *this)
{
	free(this);
}

plugin_t *coupling_plugin_create()
{
	private_coupling_plugin_t *this;

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
