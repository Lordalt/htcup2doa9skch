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


#ifndef TKM_ID_MANAGER_H_
#define TKM_ID_MANAGER_H_

#include <library.h>

typedef struct tkm_id_manager_t tkm_id_manager_t;
typedef enum tkm_context_kind_t tkm_context_kind_t;

enum tkm_context_kind_t {
	
	TKM_CTX_NONCE,
	
	TKM_CTX_DH,
	
	TKM_CTX_CC,
	
	TKM_CTX_ISA,
	
	TKM_CTX_AE,
	
	TKM_CTX_ESA,

	
	TKM_CTX_MAX,
};

extern enum_name_t *tkm_context_kind_names;

typedef uint64_t tkm_limits_t[TKM_CTX_MAX];

struct tkm_id_manager_t {

	int (*acquire_id)(tkm_id_manager_t * const this,
					  const tkm_context_kind_t kind);

	bool (*release_id)(tkm_id_manager_t * const this,
					   const tkm_context_kind_t kind,
					   const int id);

	void (*destroy)(tkm_id_manager_t *this);

};

tkm_id_manager_t *tkm_id_manager_create(const tkm_limits_t limits);

#endif 
