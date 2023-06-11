/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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


#ifndef LOAD_TESTER_CONTROL_H_
#define LOAD_TESTER_CONTROL_H_

#define LOAD_TESTER_SOCKET IPSEC_PIDDIR "/charon.ldt"

typedef struct load_tester_control_t load_tester_control_t;

struct load_tester_control_t {

	void (*destroy)(load_tester_control_t *this);
};

load_tester_control_t *load_tester_control_create();

#endif 
