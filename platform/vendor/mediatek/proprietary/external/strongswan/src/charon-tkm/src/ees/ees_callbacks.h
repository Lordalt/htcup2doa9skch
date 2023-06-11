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


#ifndef EES_CALLBACKS_H_
#define EES_CALLBACKS_H_

void charon_esa_acquire(result_type *res, const sp_id_type sp_id);

void charon_esa_expire(result_type *res, const sp_id_type sp_id,
					   const esp_spi_type spi_rem, const protocol_type protocol,
					   const expiry_flag_type hard);

#endif 
