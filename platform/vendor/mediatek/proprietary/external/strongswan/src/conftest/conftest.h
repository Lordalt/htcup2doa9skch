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


#ifndef CONFTEST_H_
#define CONFTEST_H_

#include <library.h>
#include <hydra.h>
#include <daemon.h>
#include <credentials/sets/mem_cred.h>

#include "config.h"
#include "actions.h"

typedef struct conftest_t conftest_t;

struct conftest_t {

	settings_t *test;

	char *suite_dir;

	mem_cred_t *creds;

	config_t *config;

	linked_list_t *hooks;

	actions_t *actions;

	linked_list_t *loggers;
};

extern conftest_t *conftest;

#endif 
