/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
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


#ifndef CMD_OPTION_H_
#define CMD_OPTION_H_

typedef struct cmd_option_t cmd_option_t;
typedef enum cmd_option_type_t cmd_option_type_t;

enum cmd_option_type_t {
	CMD_OPT_HELP,
	CMD_OPT_VERSION,
	CMD_OPT_DEBUG,
	CMD_OPT_HOST,
	CMD_OPT_IDENTITY,
	CMD_OPT_EAP_IDENTITY,
	CMD_OPT_XAUTH_USER,
	CMD_OPT_REMOTE_IDENTITY,
	CMD_OPT_CERT,
	CMD_OPT_RSA,
	CMD_OPT_PKCS12,
	CMD_OPT_AGENT,
	CMD_OPT_LOCAL_TS,
	CMD_OPT_REMOTE_TS,
	CMD_OPT_IKE_PROPOSAL,
	CMD_OPT_AH_PROPOSAL,
	CMD_OPT_ESP_PROPOSAL,
	CMD_OPT_PROFILE,

	CMD_OPT_COUNT
};

struct cmd_option_t {
	
	cmd_option_type_t id;
	
	const char *name;
	
	int has_arg;
	
	const char *arg;
	
	const char *desc;
	
	const char *lines[12];
};

extern cmd_option_t cmd_options[CMD_OPT_COUNT];

#endif 
