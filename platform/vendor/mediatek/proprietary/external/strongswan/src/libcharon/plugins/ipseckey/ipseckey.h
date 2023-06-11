/*
 * Copyright (C) 2012 Reto Guadagnini
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


#ifndef IPSECKEY_H_
#define IPSECKEY_H_

typedef struct ipseckey_t ipseckey_t;
typedef enum ipseckey_algorithm_t ipseckey_algorithm_t;
typedef enum ipseckey_gw_type_t ipseckey_gw_type_t;

#include <library.h>

enum ipseckey_gw_type_t {
	
	IPSECKEY_GW_TP_NOT_PRESENT = 0,
	
	IPSECKEY_GW_TP_IPV4 = 1,
	
	IPSECKEY_GW_TP_IPV6 = 2,
	
	IPSECKEY_GW_TP_WR_ENC_DNAME = 3,
};

enum ipseckey_algorithm_t {
	
	IPSECKEY_ALGORITHM_NONE = 0,
	
	IPSECKEY_ALGORITHM_DSA = 1,
	
	IPSECKEY_ALGORITHM_RSA = 2,
};

struct ipseckey_t {

	u_int8_t (*get_precedence)(ipseckey_t *this);

	ipseckey_gw_type_t (*get_gateway_type)(ipseckey_t *this);

	ipseckey_algorithm_t (*get_algorithm)(ipseckey_t *this);

	chunk_t (*get_gateway)(ipseckey_t *this);

	chunk_t (*get_public_key)(ipseckey_t *this);

	void (*destroy) (ipseckey_t *this);
};

ipseckey_t *ipseckey_create_frm_rr(rr_t *rr);

#endif 
