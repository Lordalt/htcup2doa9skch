/*
 * Copyright (C) 2006-2012 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005 Jan Hutter
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


#ifndef DAEMON_H_
#define DAEMON_H_

typedef struct daemon_t daemon_t;

#include <network/sender.h>
#include <network/receiver.h>
#include <network/socket_manager.h>
#include <control/controller.h>
#include <bus/bus.h>
#include <sa/ike_sa_manager.h>
#include <sa/trap_manager.h>
#include <sa/shunt_manager.h>
#include <config/backend_manager.h>
#include <sa/eap/eap_manager.h>
#include <sa/xauth/xauth_manager.h>

#ifdef ME
#include <sa/ikev2/connect_manager.h>
#include <sa/ikev2/mediation_manager.h>
#endif 

#define DEFAULT_THREADS 16

#define IKEV2_UDP_PORT 500

#define IKEV2_NATT_PORT 4500

#ifndef CHARON_UDP_PORT
#define CHARON_UDP_PORT IKEV2_UDP_PORT
#endif

#ifndef CHARON_NATT_PORT
#define CHARON_NATT_PORT IKEV2_NATT_PORT
#endif

struct daemon_t {

	socket_manager_t *socket;

	ike_sa_manager_t *ike_sa_manager;

	trap_manager_t *traps;

	shunt_manager_t *shunts;

	backend_manager_t *backends;

	sender_t *sender;

	receiver_t *receiver;

	bus_t *bus;

	controller_t *controller;

	eap_manager_t *eap;

	xauth_manager_t *xauth;

#ifdef ME
	connect_manager_t *connect_manager;

	mediation_manager_t *mediation_manager;
#endif 

	bool (*initialize)(daemon_t *this, char *plugins);

	void (*start)(daemon_t *this);

	void (*load_loggers)(daemon_t *this, level_t levels[DBG_MAX],
						 bool to_stderr);

	void (*set_level)(daemon_t *this, debug_t group, level_t level);
};

extern daemon_t *charon;

bool libcharon_init();

void libcharon_deinit();

#endif 
