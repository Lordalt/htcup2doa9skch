/*
 * Copyright (C) 2013 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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


#ifndef TNC_IFMAP_HTTP_H_
#define TNC_IFMAP_HTTP_H_

#include <library.h>
#include <tls_socket.h>

#include <libxml/parser.h>

typedef struct tnc_ifmap_http_t tnc_ifmap_http_t;

struct tnc_ifmap_http_t {

	status_t (*build)(tnc_ifmap_http_t *this, chunk_t *in, chunk_t *out);

	status_t (*process)(tnc_ifmap_http_t *this, chunk_t *in, chunk_t *out);

	void (*destroy)(tnc_ifmap_http_t *this);
};

tnc_ifmap_http_t *tnc_ifmap_http_create(char *uri, chunk_t user_pass);

#endif 
