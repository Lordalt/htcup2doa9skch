/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "resolve_handler.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <hydra.h>
#include <utils/debug.h>
#include <threading/mutex.h>

#define RESOLVCONF_EXEC "/sbin/resolvconf"

#define RESOLVCONF_PREFIX "lo.inet.ipsec."

#ifdef ANDROID
#include <cutils/properties.h>
#endif

typedef struct private_resolve_handler_t private_resolve_handler_t;

struct private_resolve_handler_t {

	resolve_handler_t public;

	char *file;

	bool use_resolvconf;

	char *iface_prefix;

	mutex_t *mutex;
};

static bool write_nameserver(private_resolve_handler_t *this,
							 identification_t *server, host_t *addr)
{
	FILE *in, *out;
	char buf[1024];
	size_t len;
	bool handled = FALSE;

	in = fopen(this->file, "r");
	
	unlink(this->file);
	out = fopen(this->file, "w");
	if (out)
	{
		fprintf(out, "nameserver %H   # by strongSwan, from %Y\n", addr,
				server);
		DBG1(DBG_IKE, "installing DNS server %H to %s", addr, this->file);
		handled = TRUE;

		
		if (in)
		{
			while ((len = fread(buf, 1, sizeof(buf), in)))
			{
				ignore_result(fwrite(buf, 1, len, out));
			}
		}
		fclose(out);
	}
	if (in)
	{
		fclose(in);
	}
	return handled;
}

static void remove_nameserver(private_resolve_handler_t *this,
							  identification_t *server, host_t *addr)
{
	FILE *in, *out;
	char line[1024], matcher[512];

	in = fopen(this->file, "r");
	if (in)
	{
		
		unlink(this->file);
		out = fopen(this->file, "w");
		if (out)
		{
			snprintf(matcher, sizeof(matcher),
					 "nameserver %H   # by strongSwan, from %Y\n",
					 addr, server);

			
			while (fgets(line, sizeof(line), in))
			{
				if (strpfx(line, matcher))
				{
					DBG1(DBG_IKE, "removing DNS server %H from %s",
						 addr, this->file);
				}
				else
				{
					fputs(line, out);
				}
			}
			fclose(out);
		}
		fclose(in);
	}
}

static bool invoke_resolvconf(private_resolve_handler_t *this,
							  identification_t *server, host_t *addr,
							  bool install)
{
	char cmd[128];
	bool success = TRUE;

	if (snprintf(cmd, sizeof(cmd), "%s %s %s%H", RESOLVCONF_EXEC,
				install ? "-a" : "-d", this->iface_prefix, addr) >= sizeof(cmd))
	{
		return FALSE;
	}

	if (install)
	{
		FILE *out;

		out = popen(cmd, "w");
		if (!out)
		{
			return FALSE;
		}
		DBG1(DBG_IKE, "installing DNS server %H via resolvconf", addr);
		fprintf(out, "nameserver %H\n", addr);
		success = !ferror(out);
		if (pclose(out))
		{
			return FALSE;
		}
	}
	else
	{
		ignore_result(system(cmd));
	}
	return success;
}

METHOD(attribute_handler_t, handle, bool,
	private_resolve_handler_t *this, identification_t *server,
	configuration_attribute_type_t type, chunk_t data)
{
	host_t *addr;
	bool handled;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			addr = host_create_from_chunk(AF_INET, data, 0);
			break;
		case INTERNAL_IP6_DNS:
			addr = host_create_from_chunk(AF_INET6, data, 0);
			break;
		default:
			return FALSE;
	}

	if (!addr || addr->is_anyaddr(addr))
	{
		DESTROY_IF(addr);
		return FALSE;
	}

#ifdef ANDROID
	handled = TRUE;
#else
	this->mutex->lock(this->mutex);
	if (this->use_resolvconf)
	{
 		handled = invoke_resolvconf(this, server, addr, TRUE);
	}
	else
	{
		handled = write_nameserver(this, server, addr);
	}
	this->mutex->unlock(this->mutex);
#endif
	addr->destroy(addr);

	if (!handled)
	{
		DBG1(DBG_IKE, "adding DNS server failed");
	}
	return handled;
}

METHOD(attribute_handler_t, release, void,
	private_resolve_handler_t *this, identification_t *server,
	configuration_attribute_type_t type, chunk_t data)
{
	host_t *addr;
	int family;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			family = AF_INET;
			break;
		case INTERNAL_IP6_DNS:
			family = AF_INET6;
			break;
		default:
			return;
	}
	addr = host_create_from_chunk(family, data, 0);

#ifndef ANDROID
	this->mutex->lock(this->mutex);
	if (this->use_resolvconf)
	{
		invoke_resolvconf(this, server, addr, FALSE);
	}
	else
	{
		remove_nameserver(this, server, addr);
	}
	this->mutex->unlock(this->mutex);
#endif

	addr->destroy(addr);
}

typedef struct {
	
	enumerator_t public;
	
	bool v4;
	
	bool v6;
} attribute_enumerator_t;

static bool attribute_enumerate(attribute_enumerator_t *this,
								configuration_attribute_type_t *type,
								chunk_t *data)
{
	if (this->v4)
	{
		*type = INTERNAL_IP4_DNS;
		*data = chunk_empty;
		this->v4 = FALSE;
		return TRUE;
	}
	if (this->v6)
	{
		*type = INTERNAL_IP6_DNS;
		*data = chunk_empty;
		this->v6 = FALSE;
		return TRUE;
	}
	return FALSE;
}

static bool has_host_family(linked_list_t *list, int family)
{
	enumerator_t *enumerator;
	host_t *host;
	bool found = FALSE;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &host))
	{
		if (host->get_family(host) == family)
		{
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return found;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t*,
	private_resolve_handler_t *this, identification_t *server,
	linked_list_t *vips)
{
	attribute_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = (void*)attribute_enumerate,
			.destroy = (void*)free,
		},
		.v4 = has_host_family(vips, AF_INET),
		.v6 = has_host_family(vips, AF_INET6),
	);
	return &enumerator->public;
}

METHOD(resolve_handler_t, destroy, void,
	private_resolve_handler_t *this)
{
	this->mutex->destroy(this->mutex);
	free(this);
}

resolve_handler_t *resolve_handler_create()
{
	private_resolve_handler_t *this;
	struct stat st;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = _release,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
#ifdef ANDROID
		.file = NULL,
#else
		.file = lib->settings->get_str(lib->settings, "%s.plugins.resolve.file", RESOLV_CONF, lib->ns),
#endif
	);

	if (stat(RESOLVCONF_EXEC, &st) == 0)
	{
		this->use_resolvconf = TRUE;
		this->iface_prefix = lib->settings->get_str(lib->settings,
								"%s.plugins.resolve.resolvconf.iface_prefix",
								RESOLVCONF_PREFIX, lib->ns);
	}

	return &this->public;
}

