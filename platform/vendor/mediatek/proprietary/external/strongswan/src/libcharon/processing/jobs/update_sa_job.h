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


#ifndef UPDATE_SA_JOB_H_
#define UPDATE_SA_JOB_H_

typedef struct update_sa_job_t update_sa_job_t;

#include <library.h>
#include <networking/host.h>
#include <processing/jobs/job.h>

struct update_sa_job_t {

	job_t job_interface;
};

update_sa_job_t *update_sa_job_create(u_int32_t reqid, host_t *new);

#endif 
