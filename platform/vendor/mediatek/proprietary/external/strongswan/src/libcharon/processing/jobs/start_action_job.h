/*
 * Copyright (C) 2011 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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


#ifndef START_ACTION_JOB_H_
#define START_ACTION_JOB_H_

typedef struct start_action_job_t start_action_job_t;

#include <library.h>
#include <processing/jobs/job.h>

struct start_action_job_t {
	job_t job_interface;
};

start_action_job_t *start_action_job_create(void);

#endif 
