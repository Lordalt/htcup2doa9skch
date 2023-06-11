/*
 * Copyright (C) 2005-2006 Martin Willi
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


#ifndef PARSER_H_
#define PARSER_H_

typedef struct parser_t parser_t;

#include <library.h>
#include <encoding/payloads/encodings.h>
#include <encoding/payloads/payload.h>

struct parser_t {

	status_t (*parse_payload) (parser_t *this, payload_type_t payload_type, payload_t **payload);

	int (*get_remaining_byte_count) (parser_t *this);

	void (*reset_context) (parser_t *this);

	void (*destroy) (parser_t *this);
};

parser_t *parser_create(chunk_t data);

#endif 
