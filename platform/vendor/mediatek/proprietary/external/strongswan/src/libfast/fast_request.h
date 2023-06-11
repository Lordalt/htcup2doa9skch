/*
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


#ifndef FAST_REQUEST_H_
#define FAST_REQUEST_H_

#include <fcgiapp.h>
#include <library.h>

typedef struct fast_request_t fast_request_t;

struct fast_request_t {

	void (*add_cookie)(fast_request_t *this, char *name, char *value);

	char* (*get_cookie)(fast_request_t *this, char *name);

	char* (*get_path)(fast_request_t *this);

	char* (*get_base)(fast_request_t *this);

	char* (*get_host)(fast_request_t *this);

	char* (*get_user_agent)(fast_request_t *this);

	char* (*get_query_data)(fast_request_t *this, char *name);

	char* (*get_env_var)(fast_request_t *this, char *name);

	int (*read_data)(fast_request_t *this, char *buf, int len);

	void (*close_session)(fast_request_t *this);

	bool (*session_closed)(fast_request_t *this);

	void (*redirect)(fast_request_t *this, char *fmt, ...);

	char* (*get_referer)(fast_request_t *this);

	void (*to_referer)(fast_request_t *this);

	void (*set)(fast_request_t *this, char *key, char *value);

	void (*setf)(fast_request_t *this, char *format, ...);

	void (*render)(fast_request_t *this, char *template);

	int (*streamf)(fast_request_t *this, char *format, ...);

	void (*serve)(fast_request_t *this, char *headers, chunk_t chunk);

	bool (*sendfile)(fast_request_t *this, char *path, char *mime);

	fast_request_t* (*get_ref)(fast_request_t *this);

	void (*destroy) (fast_request_t *this);
};

fast_request_t *fast_request_create(int fd, bool debug);

#endif 
