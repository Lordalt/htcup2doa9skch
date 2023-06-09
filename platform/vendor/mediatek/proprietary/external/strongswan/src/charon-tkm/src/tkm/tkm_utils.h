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


#ifndef TKM_UTILS_H_
#define TKM_UTILS_H_

#include <utils/chunk.h>
#include <tkm/types.h>

void sequence_to_chunk(const byte_t * const first, const uint32_t len,
					   chunk_t * const chunk);

void chunk_to_sequence(const chunk_t * const chunk, void *sequence,
					   const uint32_t typelen);

#endif 
