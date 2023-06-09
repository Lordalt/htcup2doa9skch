/*
 * Copyright (C) 2013 Tobias Brunner
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

#include <hydra.h>
#include <utils/debug.h>
#include <plugins/plugin_feature.h>

#include "attr_sql_plugin.h"
#include "sql_attribute.h"

typedef struct private_attr_sql_plugin_t private_attr_sql_plugin_t;

struct private_attr_sql_plugin_t {

	attr_sql_plugin_t public;

	database_t *db;

	sql_attribute_t *attribute;
};

METHOD(plugin_t, get_name, char*,
	private_attr_sql_plugin_t *this)
{
	return "attr-sql";
}

static bool open_database(private_attr_sql_plugin_t *this,
						  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		char *uri;

		uri = lib->settings->get_str(lib->settings,
								"%s.plugins.attr-sql.database", NULL, lib->ns);
		if (!uri)
		{
			DBG1(DBG_CFG, "attr-sql plugin: database URI not set");
			return FALSE;
		}

		this->db = lib->db->create(lib->db, uri);
		if (!this->db)
		{
			DBG1(DBG_CFG, "attr-sql plugin failed to connect to database");
			return FALSE;
		}
		this->attribute = sql_attribute_create(this->db);
		hydra->attributes->add_provider(hydra->attributes,
										&this->attribute->provider);
	}
	else
	{
		hydra->attributes->remove_provider(hydra->attributes,
										   &this->attribute->provider);
		this->attribute->destroy(this->attribute);
		this->db->destroy(this->db);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_attr_sql_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)open_database, NULL),
			PLUGIN_PROVIDE(CUSTOM, "attr-sql"),
				PLUGIN_DEPENDS(DATABASE, DB_ANY),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_attr_sql_plugin_t *this)
{
	free(this);
}

plugin_t *attr_sql_plugin_create()
{
	private_attr_sql_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);
	lib->settings->add_fallback(lib->settings, "%s.plugins.attr-sql",
								"libhydra.plugins.attr-sql", lib->ns);

	return &this->public.plugin;
}
