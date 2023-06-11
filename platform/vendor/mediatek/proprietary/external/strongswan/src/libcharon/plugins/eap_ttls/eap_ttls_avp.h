/*
 * Copyright (C) 2010 Andreas Steffen
 * Copyright (C) 2010 HSR Hochschule fuer Technik Rapperswil
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


#ifndef EAP_TTLS_AVP_H_
#define EAP_TTLS_AVP_H_

typedef struct eap_ttls_avp_t eap_ttls_avp_t;

#include <library.h>

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

struct eap_ttls_avp_t {

	status_t (*process)(eap_ttls_avp_t *this, bio_reader_t *reader,
						chunk_t *data);

	void (*build)(eap_ttls_avp_t *this, bio_writer_t *writer, chunk_t data);

	void (*destroy)(eap_ttls_avp_t *this);
};

eap_ttls_avp_t *eap_ttls_avp_create(void);

#endif 
