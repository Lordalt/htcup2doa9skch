/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Thomas Kallenberg
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


#ifndef UCI_CONFIG_H_
#define UCI_CONFIG_H_

#include "uci_parser.h"

#include <config/backend.h>

typedef struct uci_config_t uci_config_t;

struct uci_config_t {

	backend_t backend;

	void (*destroy)(uci_config_t *this);
};

uci_config_t *uci_config_create(uci_parser_t *parser);

#endif 
