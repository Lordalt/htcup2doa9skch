/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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


#ifndef TKM_TYPES_H_
#define TKM_TYPES_H_

#include <tkm/types.h>
#include <utils/chunk.h>

typedef struct esa_info_t esa_info_t;

struct esa_info_t {

	isa_id_type isa_id;

	esp_spi_type spi_r;

	chunk_t nonce_i;

	chunk_t nonce_r;

	bool is_encr_r;

	dh_id_type dh_id;

};

typedef struct isa_info_t isa_info_t;

struct isa_info_t {

	isa_id_type parent_isa_id;

	ae_id_type ae_id;

};

typedef struct sign_info_t sign_info_t;

struct sign_info_t {

	isa_id_type isa_id;

	chunk_t init_message;

};

#endif 
