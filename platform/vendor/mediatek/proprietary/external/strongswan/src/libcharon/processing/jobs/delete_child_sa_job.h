/*
 * Copyright (C) 2006 Martin Willi
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


#ifndef DELETE_CHILD_SA_JOB_H_
#define DELETE_CHILD_SA_JOB_H_

typedef struct delete_child_sa_job_t delete_child_sa_job_t;

#include <library.h>
#include <sa/ike_sa_id.h>
#include <processing/jobs/job.h>
#include <config/proposal.h>


struct delete_child_sa_job_t {
	job_t job_interface;
};

delete_child_sa_job_t *delete_child_sa_job_create(u_int32_t reqid,
							protocol_id_t protocol, u_int32_t spi, bool expired);

#endif 
