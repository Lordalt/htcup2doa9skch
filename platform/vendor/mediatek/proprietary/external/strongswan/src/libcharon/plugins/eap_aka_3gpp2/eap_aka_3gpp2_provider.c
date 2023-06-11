/*
 * Copyright (C) 2008-2009 Martin Willi
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

#include "eap_aka_3gpp2_provider.h"

#include <daemon.h>
#include <credentials/keys/shared_key.h>

typedef struct private_eap_aka_3gpp2_provider_t private_eap_aka_3gpp2_provider_t;

struct private_eap_aka_3gpp2_provider_t {

	eap_aka_3gpp2_provider_t public;

	eap_aka_3gpp2_functions_t *f;

	char sqn[AKA_SQN_LEN];
};

static char amf[AKA_AMF_LEN] = {0x00, 0x01};

bool eap_aka_3gpp2_get_k(identification_t *id, char k[AKA_K_LEN])
{
	shared_key_t *shared;
	chunk_t key;

	shared = lib->credmgr->get_shared(lib->credmgr, SHARED_EAP, id, NULL);
	if (shared == NULL)
	{
		return FALSE;
	}
	key = shared->get_key(shared);
	memset(k, '\0', AKA_K_LEN);
	memcpy(k, key.ptr, min(key.len, AKA_K_LEN));
	shared->destroy(shared);
	return TRUE;
}

void eap_aka_3gpp2_get_sqn(char sqn[AKA_SQN_LEN], int offset)
{
	timeval_t time;

	gettimeofday(&time, NULL);
	
	time.tv_sec = htonl(time.tv_sec + offset);
	
	time.tv_usec = htonl(time.tv_usec << 12);
	memcpy(sqn, (char*)&time.tv_sec + sizeof(time_t) - 4, 4);
	memcpy(sqn + 4, &time.tv_usec, 2);
}

METHOD(simaka_provider_t, get_quintuplet, bool,
	private_eap_aka_3gpp2_provider_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char xres[AKA_RES_MAX], int *xres_len,
	char ck[AKA_CK_LEN], char ik[AKA_IK_LEN], char autn[AKA_AUTN_LEN])
{
	rng_t *rng;
	char mac[AKA_MAC_LEN], ak[AKA_AK_LEN], k[AKA_K_LEN];

	
	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->get_bytes(rng, AKA_RAND_LEN, rand))
	{
		DBG1(DBG_IKE, "generating RAND for AKA failed");
		DESTROY_IF(rng);
		return FALSE;
	}
	rng->destroy(rng);

	if (!eap_aka_3gpp2_get_k(id, k))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to authenticate with AKA", id);
		return FALSE;
	}

	DBG3(DBG_IKE, "generated rand %b", rand, AKA_RAND_LEN);
	DBG3(DBG_IKE, "using K %b", k, AKA_K_LEN);

	
	if (!this->f->f1(this->f, k, rand, this->sqn, amf, mac) ||
		!this->f->f5(this->f, k, rand, ak) ||
		!this->f->f2(this->f, k, rand, xres))
	{
		return FALSE;
	}
	*xres_len = AKA_RES_MAX;
	
	memcpy(autn, this->sqn, AKA_SQN_LEN);
	memxor(autn, ak, AKA_AK_LEN);
	memcpy(autn + AKA_SQN_LEN, amf, AKA_AMF_LEN);
	memcpy(autn + AKA_SQN_LEN + AKA_AMF_LEN, mac, AKA_MAC_LEN);
	DBG3(DBG_IKE, "AUTN %b", autn, AKA_AUTN_LEN);
	
	if (!this->f->f3(this->f, k, rand, ck) ||
		!this->f->f4(this->f, k, rand, ik))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(simaka_provider_t, resync, bool,
	private_eap_aka_3gpp2_provider_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{
	char *sqn, *macs;
	char aks[AKA_AK_LEN], k[AKA_K_LEN], amf[AKA_AMF_LEN], xmacs[AKA_MAC_LEN];

	if (!eap_aka_3gpp2_get_k(id, k))
	{
		DBG1(DBG_IKE, "no EAP key found for %Y to authenticate with AKA", id);
		return FALSE;
	}

	
	sqn = auts;
	macs = auts + AKA_SQN_LEN;
	if (!this->f->f5star(this->f, k, rand, aks))
	{
		return FALSE;
	}
	memxor(sqn, aks, AKA_AK_LEN);

	
	memset(amf, 0, AKA_AMF_LEN);
	if (!this->f->f1star(this->f, k, rand, sqn, amf, xmacs))
	{
		return FALSE;
	}
	if (!memeq(macs, xmacs, AKA_MAC_LEN))
	{
		DBG1(DBG_IKE, "received MACS does not match XMACS");
		DBG3(DBG_IKE, "MACS %b XMACS %b",
			 macs, AKA_MAC_LEN, xmacs, AKA_MAC_LEN);
		return FALSE;
	}
	
	memcpy(this->sqn, sqn, AKA_SQN_LEN);
	chunk_increment(chunk_create(this->sqn, AKA_SQN_LEN));
	return TRUE;
}

METHOD(eap_aka_3gpp2_provider_t, destroy, void,
	private_eap_aka_3gpp2_provider_t *this)
{
	free(this);
}

eap_aka_3gpp2_provider_t *eap_aka_3gpp2_provider_create(
												eap_aka_3gpp2_functions_t *f)
{
	private_eap_aka_3gpp2_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.get_triplet = (void*)return_false,
				.get_quintuplet = _get_quintuplet,
				.resync = _resync,
				.is_pseudonym = (void*)return_null,
				.gen_pseudonym = (void*)return_null,
				.is_reauth = (void*)return_null,
				.gen_reauth = (void*)return_null,
			},
			.destroy = _destroy,
		},
		.f = f,
	);
	
	eap_aka_3gpp2_get_sqn(this->sqn, 180);

	return &this->public;
}

