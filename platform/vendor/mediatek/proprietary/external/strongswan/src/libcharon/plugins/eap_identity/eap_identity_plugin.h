/*
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


#ifndef EAP_IDENTITY_PLUGIN_H_
#define EAP_IDENTITY_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_identity_plugin_t eap_identity_plugin_t;

struct eap_identity_plugin_t {

	plugin_t plugin;
};

#endif 
