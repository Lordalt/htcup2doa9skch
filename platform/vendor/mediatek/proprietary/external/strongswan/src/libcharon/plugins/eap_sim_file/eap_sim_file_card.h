/*
 * Copyright (C) 2008 Martin Willi
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


#ifndef EAP_SIM_FILE_CARD_H_
#define EAP_SIM_FILE_CARD_H_

#include "eap_sim_file_triplets.h"

#include <simaka_card.h>

typedef struct eap_sim_file_card_t eap_sim_file_card_t;

struct eap_sim_file_card_t {

	simaka_card_t card;

	void (*destroy)(eap_sim_file_card_t *this);
};

eap_sim_file_card_t *eap_sim_file_card_create(eap_sim_file_triplets_t *triplets);

#endif 
