/*
 * Copyright (C) 2013 Ruslan Marchenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef DNSCERT_H_
#define DNSCERT_H_

typedef struct dnscert_t dnscert_t;
typedef enum dnscert_algorithm_t dnscert_algorithm_t;
typedef enum dnscert_type_t dnscert_type_t;

#include <library.h>

enum dnscert_type_t {
	
	DNSCERT_TYPE_RESERVED = 0,
	
	DNSCERT_TYPE_PKIX = 1,
	
	DNSCERT_TYPE_SKPI = 2,
	
	DNSCERT_TYPE_PGP = 3,
	
	DNSCERT_TYPE_IPKIX = 4,
	
	DNSCERT_TYPE_ISKPI = 5,
	
	DNSCERT_TYPE_IPGP = 6,
	
	DNSCERT_TYPE_ACPKIX = 7,
	
	DNSCERT_TYPE_IACKPIX = 8
};

enum dnscert_algorithm_t {
	
	DNSCERT_ALGORITHM_UNDEFINED = 0,
	
	DNSCERT_ALGORITHM_RSAMD5 = 1,
	
	DNSCERT_ALGORITHM_DH = 2,
	
	DNSCERT_ALGORITHM_DSASHA = 3,
	
	DNSCERT_ALGORITHM_RSRVD4 = 4,
	
	DNSCERT_ALGORITHM_RSASHA = 5,
	
	DNSCERT_ALGORITHM_DSANSEC3 = 6,
	
	DNSCERT_ALGORITHM_RSANSEC3 = 7,
	
	DNSCERT_ALGORITHM_RSASHA256 = 8,
	
	DNSCERT_ALGORITHM_RSRVD9 = 9,
	
	DNSCERT_ALGORITHM_RSASHA512 = 10,
};

struct dnscert_t {

	dnscert_type_t (*get_cert_type)(dnscert_t *this);

	u_int16_t (*get_key_tag)(dnscert_t *this);

	dnscert_algorithm_t (*get_algorithm)(dnscert_t *this);

	chunk_t (*get_certificate)(dnscert_t *this);

	void (*destroy) (dnscert_t *this);
};

dnscert_t *dnscert_create_frm_rr(rr_t *rr);

#endif 
