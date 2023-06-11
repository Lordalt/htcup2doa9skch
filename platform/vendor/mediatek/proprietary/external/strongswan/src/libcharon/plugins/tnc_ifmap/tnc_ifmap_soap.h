/*
 * Copyright (C) 2011-2013 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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


#ifndef TNC_IFMAP_SOAP_H_
#define TNC_IFMAP_SOAP_H_

#include <library.h>
#include <networking/host.h>
#include <sa/ike_sa.h>

typedef struct tnc_ifmap_soap_t tnc_ifmap_soap_t;

struct tnc_ifmap_soap_t {

	bool (*newSession)(tnc_ifmap_soap_t *this);

	bool (*renewSession)(tnc_ifmap_soap_t *this);

	bool (*purgePublisher)(tnc_ifmap_soap_t *this);

	bool (*publish_ike_sa)(tnc_ifmap_soap_t *this, ike_sa_t *ike_sa, bool up);

	bool (*publish_device_ip)(tnc_ifmap_soap_t *this, host_t *host);

	bool (*publish_virtual_ips)(tnc_ifmap_soap_t *this, ike_sa_t *ike_sa,
								bool assign);

	bool (*publish_enforcement_report)(tnc_ifmap_soap_t *this, host_t *host,
									   char *action, char *reason);

	bool (*endSession)(tnc_ifmap_soap_t *this);

	char* (*get_session_id)(tnc_ifmap_soap_t *this);

	bool (*orphaned)(tnc_ifmap_soap_t *this);

	tnc_ifmap_soap_t* (*get_ref)(tnc_ifmap_soap_t *this);

	void (*destroy)(tnc_ifmap_soap_t *this);
};

tnc_ifmap_soap_t *tnc_ifmap_soap_create();

#endif 
