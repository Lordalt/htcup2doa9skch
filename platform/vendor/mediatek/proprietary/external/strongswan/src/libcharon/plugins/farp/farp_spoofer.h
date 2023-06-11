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


#ifndef FARP_SPOOFER_H_
#define FARP_SPOOFER_H_

#include "farp_listener.h"

typedef struct farp_spoofer_t farp_spoofer_t;

struct farp_spoofer_t {

	void (*destroy)(farp_spoofer_t *this);
};

farp_spoofer_t *farp_spoofer_create(farp_listener_t *listener);

#endif 
