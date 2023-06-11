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


#ifndef EAP_AKA_3GPP2_FUNCTIONS_H_
#define EAP_AKA_3GPP2_FUNCTIONS_H_

#include <simaka_manager.h>

#define AKA_SQN_LEN		 6
#define AKA_K_LEN		16
#define AKA_MAC_LEN 	 8
#define AKA_AK_LEN		 6
#define AKA_AMF_LEN		 2
#define AKA_FMK_LEN		 4

typedef struct eap_aka_3gpp2_functions_t eap_aka_3gpp2_functions_t;

struct eap_aka_3gpp2_functions_t {

	bool (*f1)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char sqn[AKA_SQN_LEN],
				u_char amf[AKA_AMF_LEN], u_char mac[AKA_MAC_LEN]);

	bool (*f1star)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char sqn[AKA_SQN_LEN],
				u_char amf[AKA_AMF_LEN], u_char macs[AKA_MAC_LEN]);

	bool (*f2)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char res[AKA_RES_MAX]);
	bool (*f3)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char ck[AKA_CK_LEN]);
	bool (*f4)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char ik[AKA_IK_LEN]);
	bool (*f5)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char ak[AKA_AK_LEN]);
	bool (*f5star)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char aks[AKA_AK_LEN]);

	void (*destroy)(eap_aka_3gpp2_functions_t *this);
};

eap_aka_3gpp2_functions_t *eap_aka_3gpp2_functions_create();

#endif 
