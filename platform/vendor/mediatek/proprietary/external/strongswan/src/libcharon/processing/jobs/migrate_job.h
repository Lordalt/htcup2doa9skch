/*
 * Copyright (C) 2008 Andreas Steffen
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


#ifndef MIGRATE_JOB_H_
#define MIGRATE_JOB_H_

typedef struct migrate_job_t migrate_job_t;

#include <library.h>
#include <networking/host.h>
#include <selectors/traffic_selector.h>
#include <kernel_wrap/kernel_ipsec.h>
#include <processing/jobs/job.h>

struct migrate_job_t {
	job_t job_interface;
};

migrate_job_t *migrate_job_create(u_int32_t reqid,
						traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
						policy_dir_t dir, host_t *local, host_t *remote);

#endif 
