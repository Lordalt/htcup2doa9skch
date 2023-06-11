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


#ifndef CMD_CREDS_H_
#define CMD_CREDS_H_

#include <library.h>

#include "cmd_options.h"

typedef struct cmd_creds_t cmd_creds_t;

struct cmd_creds_t {

	bool (*handle)(cmd_creds_t *this, cmd_option_type_t opt, char *arg);

	void (*destroy)(cmd_creds_t *this);
};

cmd_creds_t *cmd_creds_create();

#endif 
