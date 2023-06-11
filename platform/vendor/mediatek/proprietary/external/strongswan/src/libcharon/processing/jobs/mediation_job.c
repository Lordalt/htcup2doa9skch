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

#include "mediation_job.h"

#include <encoding/payloads/endpoint_notify.h>
#include <daemon.h>


typedef struct private_mediation_job_t private_mediation_job_t;

struct private_mediation_job_t {
	mediation_job_t public;

	identification_t *target;

	identification_t *source;

	chunk_t connect_id;

	chunk_t connect_key;

	linked_list_t *endpoints;

	bool callback;

	bool response;
};

METHOD(job_t, destroy, void,
	private_mediation_job_t *this)
{
	DESTROY_IF(this->target);
	DESTROY_IF(this->source);
	chunk_free(&this->connect_id);
	chunk_free(&this->connect_key);
	DESTROY_OFFSET_IF(this->endpoints, offsetof(endpoint_notify_t, destroy));
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_mediation_job_t *this)
{
	ike_sa_id_t *target_sa_id;

	target_sa_id = charon->mediation_manager->check(charon->mediation_manager, this->target);

	if (target_sa_id)
	{
		ike_sa_t *target_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
											  target_sa_id);
		if (target_sa)
		{
			if (this->callback)
			{
				
				if (target_sa->callback(target_sa, this->source) != SUCCESS)
				{
					DBG1(DBG_JOB, "callback for '%Y' to '%Y' failed",
							this->source, this->target);
					charon->ike_sa_manager->checkin(charon->ike_sa_manager, target_sa);
					return JOB_REQUEUE_NONE;
				}
			}
			else
			{
				
				if (target_sa->relay(target_sa, this->source, this->connect_id,
						this->connect_key, this->endpoints, this->response) != SUCCESS)
				{
					DBG1(DBG_JOB, "mediation between '%Y' and '%Y' failed",
							this->source, this->target);
					charon->ike_sa_manager->checkin(charon->ike_sa_manager, target_sa);
					
					return JOB_REQUEUE_NONE;
				}
			}

			charon->ike_sa_manager->checkin(charon->ike_sa_manager, target_sa);
		}
		else
		{
			DBG1(DBG_JOB, "mediation between '%Y' and '%Y' failed: "
					"SA not found", this->source, this->target);
		}
	}
	else
	{
		DBG1(DBG_JOB, "mediation between '%Y' and '%Y' failed: "
				"peer is not online anymore", this->source, this->target);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_mediation_job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

static private_mediation_job_t *mediation_job_create_empty()
{
	private_mediation_job_t *this;
	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
	);
	return this;
}

mediation_job_t *mediation_job_create(identification_t *peer_id,
		identification_t *requester, chunk_t connect_id, chunk_t connect_key,
		linked_list_t *endpoints, bool response)
{
	private_mediation_job_t *this = mediation_job_create_empty();

	this->target = peer_id->clone(peer_id);
	this->source = requester->clone(requester);
	this->connect_id = chunk_clone(connect_id);
	this->connect_key = chunk_clone(connect_key);
	this->endpoints = endpoints->clone_offset(endpoints, offsetof(endpoint_notify_t, clone));
	this->response = response;

	return &this->public;
}

mediation_job_t *mediation_callback_job_create(identification_t *requester,
		identification_t *peer_id)
{
	private_mediation_job_t *this = mediation_job_create_empty();

	this->target = requester->clone(requester);
	this->source = peer_id->clone(peer_id);
	this->callback = TRUE;

	return &this->public;
}
