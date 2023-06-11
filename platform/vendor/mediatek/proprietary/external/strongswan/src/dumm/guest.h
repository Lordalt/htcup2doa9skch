/*
 * Copyright (C) 2008-2009 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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

#ifndef GUEST_H
#define GUEST_H

#include <library.h>
#include <collections/enumerator.h>

typedef enum guest_state_t guest_state_t;
typedef struct guest_t guest_t;

#include "iface.h"

enum guest_state_t {
	
	GUEST_STOPPED,
	
	GUEST_STARTING,
	
	GUEST_RUNNING,
	
	GUEST_PAUSED,
	
	GUEST_STOPPING,
};

extern enum_name_t *guest_state_names;

typedef pid_t (*invoke_function_t)(void *data, guest_t *guest,
								   char *args[], int argc);

typedef void (*idle_function_t)(void);

struct guest_t {

	char* (*get_name) (guest_t *this);

	pid_t (*get_pid) (guest_t *this);

	guest_state_t (*get_state)(guest_t *this);

	bool (*start) (guest_t *this, invoke_function_t invoke, void *data,
				   idle_function_t idle);

	void (*stop) (guest_t *this, idle_function_t idle);

	iface_t* (*create_iface)(guest_t *this, char *name);

	void (*destroy_iface)(guest_t *this, iface_t *iface);

	enumerator_t* (*create_iface_enumerator)(guest_t *this);

	bool (*add_overlay)(guest_t *this, char *dir);

	bool (*del_overlay)(guest_t *this, char *dir);

	bool (*pop_overlay)(guest_t *this);

	int (*exec)(guest_t *this, void(*cb)(void*,char*,size_t), void *data,
				char *cmd, ...);

	int (*exec_str)(guest_t *this, void(*cb)(void*,char*), bool lines,
					void *data, char *cmd, ...);

	void (*sigchild)(guest_t *this);

	void (*destroy) (guest_t *this);
};

guest_t *guest_create(char *parent, char *name, char *kernel,
					  char *master, char *args);

guest_t *guest_load(char *parent, char *name);

#endif 

