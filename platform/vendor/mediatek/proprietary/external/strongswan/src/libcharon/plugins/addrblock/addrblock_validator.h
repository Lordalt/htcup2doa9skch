/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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


#ifndef ADDRBLOCK_VALIDATOR_H_
#define ADDRBLOCK_VALIDATOR_H_

#include <credentials/cert_validator.h>

typedef struct addrblock_validator_t addrblock_validator_t;

struct addrblock_validator_t {

	cert_validator_t validator;

	void (*destroy)(addrblock_validator_t *this);
};

addrblock_validator_t *addrblock_validator_create();

#endif 
