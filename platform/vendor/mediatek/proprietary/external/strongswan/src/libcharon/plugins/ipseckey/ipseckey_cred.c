/*
 * Copyright (C) 2013 Tobias Brunner
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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

#include "ipseckey_cred.h"
#include "ipseckey.h"

#include <bio/bio_reader.h>

typedef struct private_ipseckey_cred_t private_ipseckey_cred_t;

struct private_ipseckey_cred_t {

	ipseckey_cred_t public;

	resolver_t *res;
};

typedef struct {
	
	enumerator_t public;
	
	enumerator_t *inner;
	
	resolver_response_t *response;
	
	time_t notBefore;
	
	time_t notAfter;
	
	identification_t *identity;
} cert_enumerator_t;

METHOD(enumerator_t, cert_enumerator_enumerate, bool,
	cert_enumerator_t *this, certificate_t **cert)
{
	ipseckey_t *cur_ipseckey;
	public_key_t *public;
	rr_t *cur_rr;
	chunk_t key;

	
	while (this->inner->enumerate(this->inner, &cur_rr))
	{
		cur_ipseckey = ipseckey_create_frm_rr(cur_rr);

		if (!cur_ipseckey)
		{
			DBG1(DBG_CFG, "  failed to parse IPSECKEY, skipping");
			continue;
		}

		if (cur_ipseckey->get_algorithm(cur_ipseckey) != IPSECKEY_ALGORITHM_RSA)
		{
			DBG1(DBG_CFG, "  unsupported IPSECKEY algorithm, skipping");
			cur_ipseckey->destroy(cur_ipseckey);
			continue;
		}

		key = cur_ipseckey->get_public_key(cur_ipseckey);
		public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
									BUILD_BLOB_DNSKEY, key,
									BUILD_END);
		if (!public)
		{
			DBG1(DBG_CFG, "  failed to create public key from IPSECKEY");
			cur_ipseckey->destroy(cur_ipseckey);
			continue;
		}

		*cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
								   CERT_TRUSTED_PUBKEY,
								   BUILD_PUBLIC_KEY, public,
								   BUILD_SUBJECT, this->identity,
								   BUILD_NOT_BEFORE_TIME, this->notBefore,
								   BUILD_NOT_AFTER_TIME, this->notAfter,
								   BUILD_END);
		if (*cert == NULL)
		{
			DBG1(DBG_CFG, "  failed to create certificate from IPSECKEY");
			cur_ipseckey->destroy(cur_ipseckey);
			public->destroy(public);
			continue;
		}
		cur_ipseckey->destroy(cur_ipseckey);
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, cert_enumerator_destroy, void,
	cert_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	this->response->destroy(this->response);
	free(this);
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_ipseckey_cred_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	resolver_response_t *response;
	enumerator_t *rrsig_enum;
	cert_enumerator_t *e;
	rr_set_t *rrset;
	rr_t *rrsig;
	bio_reader_t *reader;
	u_int32_t nBefore, nAfter;
	chunk_t ignore;
	char *fqdn;

	if (!id || id->get_type(id) != ID_FQDN)
	{
		return enumerator_create_empty();
	}

	
	if (asprintf(&fqdn, "%Y", id) <= 0)
	{
		DBG1(DBG_CFG, "failed to determine FQDN to retrieve IPSECKEY RRs");
		return enumerator_create_empty();
	}
	DBG1(DBG_CFG, "performing a DNS query for IPSECKEY RRs of '%s'", fqdn);
	response = this->res->query(this->res, fqdn, RR_CLASS_IN, RR_TYPE_IPSECKEY);
	if (!response)
	{
		DBG1(DBG_CFG, "  query for IPSECKEY RRs failed");
		free(fqdn);
		return enumerator_create_empty();
	}
	free(fqdn);

	if (!response->has_data(response) ||
		!response->query_name_exist(response))
	{
		DBG1(DBG_CFG, "  unable to retrieve IPSECKEY RRs from the DNS");
		response->destroy(response);
		return enumerator_create_empty();
	}

	if (response->get_security_state(response) != SECURE)
	{
		DBG1(DBG_CFG, "  DNSSEC state of IPSECKEY RRs is not secure");
		response->destroy(response);
		return enumerator_create_empty();
	}

	rrset = response->get_rr_set(response);
	rrsig_enum = rrset->create_rrsig_enumerator(rrset);
	if (!rrsig_enum || !rrsig_enum->enumerate(rrsig_enum, &rrsig))
	{
		DBG1(DBG_CFG, "  unable to determine the validity period of "
			 "IPSECKEY RRs because no RRSIGs are present");
		DESTROY_IF(rrsig_enum);
		response->destroy(response);
		return enumerator_create_empty();
	}
	rrsig_enum->destroy(rrsig_enum);

	
	reader = bio_reader_create(rrsig->get_rdata(rrsig));
	if (!reader->read_data(reader, 8, &ignore) ||
		!reader->read_uint32(reader, &nAfter) ||
		!reader->read_uint32(reader, &nBefore))
	{
		DBG1(DBG_CFG, "  unable to determine the validity period of RRSIG RRs");
		reader->destroy(reader);
		response->destroy(response);
		return enumerator_create_empty();
	}
	reader->destroy(reader);

	INIT(e,
		.public = {
			.enumerate = (void*)_cert_enumerator_enumerate,
			.destroy = _cert_enumerator_destroy,
		},
		.inner = rrset->create_rr_enumerator(rrset),
		.response = response,
		.notBefore = nBefore,
		.notAfter = nAfter,
		.identity = id,
	);
	return &e->public;
}

METHOD(ipseckey_cred_t, destroy, void,
	private_ipseckey_cred_t *this)
{
	this->res->destroy(this->res);
	free(this);
}

ipseckey_cred_t *ipseckey_cred_create(resolver_t *res)
{
	private_ipseckey_cred_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
		.res = res,
	);

	return &this->public;
}
