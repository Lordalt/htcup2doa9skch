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


#ifndef ADOPT_CHILDREN_JOB_H_
#define ADOPT_CHILDREN_JOB_H_

#include <library.h>
#include <processing/jobs/job.h>
#include <sa/ike_sa_id.h>

typedef struct adopt_children_job_t adopt_children_job_t;

struct adopt_children_job_t {

	job_t job_interface;
};

adopt_children_job_t *adopt_children_job_create(ike_sa_id_t *id);

#endif 
