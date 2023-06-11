/*
 * Copyright (C) 2007 Tobias Brunner
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


#ifndef MEDIATION_JOB_H_
#define MEDIATION_JOB_H_

typedef struct mediation_job_t mediation_job_t;

#include <library.h>
#include <processing/jobs/job.h>
#include <utils/identification.h>
#include <collections/linked_list.h>

struct mediation_job_t {
	job_t job_interface;
};

mediation_job_t *mediation_job_create(identification_t *peer_id,
		identification_t *requester, chunk_t connect_id, chunk_t connect_key,
		linked_list_t *endpoints, bool response);


mediation_job_t *mediation_callback_job_create(identification_t *requester,
		identification_t *peer_id);

#endif 
