/*
 * Copyright (C) 2005-2009 Martin Willi
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


#ifndef GENERATOR_H_
#define GENERATOR_H_

typedef struct generator_t generator_t;

#include <library.h>
#include <encoding/payloads/encodings.h>
#include <encoding/payloads/payload.h>

struct generator_t {

	void (*generate_payload) (generator_t *this,payload_t *payload);

	chunk_t (*get_chunk) (generator_t *this, u_int32_t **lenpos);

	void (*destroy) (generator_t *this);
};

generator_t *generator_create(void);

generator_t *generator_create_no_dbg(void);


#endif 
