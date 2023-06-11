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


#ifndef FAST_SMTP_H_
#define FAST_SMTP_H_

typedef struct fast_smtp_t fast_smtp_t;

#include <library.h>

struct fast_smtp_t {

	bool (*send_mail)(fast_smtp_t *this, char *from, char *to,
					  char *subject, char *fmt, ...);

	void (*destroy)(fast_smtp_t *this);
};

fast_smtp_t *fast_smtp_create();

#endif 
