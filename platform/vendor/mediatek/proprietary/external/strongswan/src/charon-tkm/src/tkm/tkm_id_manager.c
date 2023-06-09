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

#include "tkm_id_manager.h"

#include <utils/debug.h>
#include <threading/rwlock.h>

#define TKM_LIMIT 100

ENUM_BEGIN(tkm_context_kind_names, TKM_CTX_NONCE, TKM_CTX_ESA,
	"NONCE_CONTEXT",
	"DH_CONTEXT",
	"CC_CONTEXT"
	"ISA_CONTEXT",
	"AE_CONTEXT",
	"ESA_CONTEXT");
ENUM_END(tkm_context_kind_names, TKM_CTX_ESA);

typedef struct private_tkm_id_manager_t private_tkm_id_manager_t;

struct private_tkm_id_manager_t {

	tkm_id_manager_t public;

	bool* ctxids[TKM_CTX_MAX];

	tkm_limits_t limits;

	rwlock_t *locks[TKM_CTX_MAX];

};

static bool is_valid_kind(const tkm_context_kind_t kind)
{
	return (int)kind >= 0 && kind < TKM_CTX_MAX;
};

METHOD(tkm_id_manager_t, acquire_id, int,
	private_tkm_id_manager_t * const this, const tkm_context_kind_t kind)
{
	int id = 0;
	uint64_t j;

	if (!is_valid_kind(kind))
	{
		DBG1(DBG_LIB, "tried to acquire id for invalid context kind '%d'",
			 kind);
		return 0;
	}

	this->locks[kind]->write_lock(this->locks[kind]);
	for (j = 0; j < this->limits[kind]; j++)
	{
		if (!this->ctxids[kind][j])
		{
			this->ctxids[kind][j] = true;
			id = j + 1;
			break;
		}
	}
	this->locks[kind]->unlock(this->locks[kind]);

	if (!id)
	{
		DBG1(DBG_LIB, "acquiring %N context id failed",	tkm_context_kind_names,
			 kind);
	}

	return id;
}

METHOD(tkm_id_manager_t, release_id, bool,
	private_tkm_id_manager_t * const this, const tkm_context_kind_t kind,
	const int id)
{
	const int idx = id - 1;

	if (!is_valid_kind(kind))
	{
		DBG1(DBG_LIB, "tried to release id %d for invalid context kind '%d'",
			 id, kind);
		return FALSE;
	}

	this->locks[kind]->write_lock(this->locks[kind]);
	this->ctxids[kind][idx] = false;
	this->locks[kind]->unlock(this->locks[kind]);

	return TRUE;
}


METHOD(tkm_id_manager_t, destroy, void,
	private_tkm_id_manager_t *this)
{
	int i;
	for (i = 0; i < TKM_CTX_MAX; i++)
	{
		free(this->ctxids[i]);
		this->locks[i]->destroy(this->locks[i]);
	}
	free(this);
}

tkm_id_manager_t *tkm_id_manager_create(const tkm_limits_t limits)
{
	private_tkm_id_manager_t *this;
	int i;

	INIT(this,
		.public = {
			.acquire_id = _acquire_id,
			.release_id = _release_id,
			.destroy = _destroy,
		},
	);

	for (i = 0; i < TKM_CTX_MAX; i++)
	{
		this->limits[i] = limits[i];
		this->ctxids[i] = calloc(limits[i], sizeof(bool));
		this->locks[i] = rwlock_create(RWLOCK_TYPE_DEFAULT);
		DBG2(DBG_LIB, "%N initialized, %llu slot(s)", tkm_context_kind_names, i,
			 limits[i]);
	}

	return &this->public;
}
