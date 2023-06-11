/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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


#ifndef DELETE_IKE_SA_JOB_H_
#define DELETE_IKE_SA_JOB_H_

typedef struct delete_ike_sa_job_t delete_ike_sa_job_t;

#include <library.h>
#include <sa/ike_sa_id.h>
#include <processing/jobs/job.h>


struct delete_ike_sa_job_t {

	job_t job_interface;
};

delete_ike_sa_job_t *delete_ike_sa_job_create(ike_sa_id_t *ike_sa_id,
											  bool delete_if_established);

#endif 
