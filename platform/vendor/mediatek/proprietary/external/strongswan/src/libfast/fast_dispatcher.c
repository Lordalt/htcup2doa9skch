/*
 * Copyright (C) 2007 Martin Willi
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

#include "fast_dispatcher.h"

#include "fast_request.h"
#include "fast_session.h"

#include <fcgiapp.h>
#include <signal.h>
#include <unistd.h>

#include <utils/debug.h>
#include <threading/thread.h>
#include <threading/condvar.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>
#include <collections/hashtable.h>

#define CLEANUP_INTERVAL 30

typedef struct private_fast_dispatcher_t private_fast_dispatcher_t;

struct private_fast_dispatcher_t {

	fast_dispatcher_t public;

	int fd;

	thread_t **threads;

	int thread_count;

	mutex_t *mutex;

	hashtable_t *sessions;

	time_t timeout;

	time_t last_cleanup;

	bool debug;

	linked_list_t *controllers;

	linked_list_t *filters;

	fast_context_constructor_t context_constructor;

	void *param;
};

typedef struct {
	
	fast_controller_constructor_t constructor;
	
	void *param;
} controller_entry_t;

typedef struct {
	
	fast_filter_constructor_t constructor;
	
	void *param;
} filter_entry_t;

typedef struct {
	
	fast_session_t *session;
	
	condvar_t *cond;
	
	char *host;
	
	bool in_use;
	
	time_t used;
	
	bool closed;
} session_entry_t;

static fast_session_t* load_session(private_fast_dispatcher_t *this)
{
	enumerator_t *enumerator;
	controller_entry_t *centry;
	filter_entry_t *fentry;
	fast_session_t *session;
	fast_context_t *context = NULL;
	fast_controller_t *controller;
	fast_filter_t *filter;

	if (this->context_constructor)
	{
		context = this->context_constructor(this->param);
	}
	session = fast_session_create(context);
	if (!session)
	{
		return NULL;
	}

	enumerator = this->controllers->create_enumerator(this->controllers);
	while (enumerator->enumerate(enumerator, &centry))
	{
		controller = centry->constructor(context, centry->param);
		session->add_controller(session, controller);
	}
	enumerator->destroy(enumerator);

	enumerator = this->filters->create_enumerator(this->filters);
	while (enumerator->enumerate(enumerator, &fentry))
	{
		filter = fentry->constructor(context, fentry->param);
		session->add_filter(session, filter);
	}
	enumerator->destroy(enumerator);

	return session;
}

static session_entry_t *session_entry_create(private_fast_dispatcher_t *this,
											 char *host)
{
	session_entry_t *entry;
	fast_session_t *session;

	session = load_session(this);
	if (!session)
	{
		return NULL;
	}
	INIT(entry,
		.cond = condvar_create(CONDVAR_TYPE_DEFAULT),
		.session = session,
		.host = strdup(host),
		.used = time_monotonic(NULL),
	);
	return entry;
}

static void session_entry_destroy(session_entry_t *entry)
{
	entry->session->destroy(entry->session);
	entry->cond->destroy(entry->cond);
	free(entry->host);
	free(entry);
}

METHOD(fast_dispatcher_t, add_controller, void,
	private_fast_dispatcher_t *this, fast_controller_constructor_t constructor,
	void *param)
{
	controller_entry_t *entry;

	INIT(entry,
		.constructor = constructor,
		.param = param,
	);
	this->controllers->insert_last(this->controllers, entry);
}

METHOD(fast_dispatcher_t, add_filter, void,
	private_fast_dispatcher_t *this, fast_filter_constructor_t constructor,
	void *param)
{
	filter_entry_t *entry;

	INIT(entry,
		.constructor = constructor,
		.param = param,
	);
	this->filters->insert_last(this->filters, entry);
}

static u_int session_hash(char *sid)
{
	return chunk_hash(chunk_create(sid, strlen(sid)));
}

static bool session_equals(char *sid1, char *sid2)
{
	return streq(sid1, sid2);
}

static void cleanup_sessions(private_fast_dispatcher_t *this, time_t now)
{
	if (this->last_cleanup < now - CLEANUP_INTERVAL)
	{
		char *sid;
		session_entry_t *entry;
		enumerator_t *enumerator;
		linked_list_t *remove;

		this->last_cleanup = now;
		remove = linked_list_create();
		enumerator = this->sessions->create_enumerator(this->sessions);
		while (enumerator->enumerate(enumerator, &sid, &entry))
		{
			
			if (!entry->in_use &&
				(entry->used < now - this->timeout || entry->closed))
			{
				remove->insert_last(remove, sid);
			}
		}
		enumerator->destroy(enumerator);

		while (remove->remove_last(remove, (void**)&sid) == SUCCESS)
		{
			entry = this->sessions->remove(this->sessions, sid);
			if (entry)
			{
				session_entry_destroy(entry);
			}
		}
		remove->destroy(remove);
	}
}

static void dispatch(private_fast_dispatcher_t *this)
{
	thread_cancelability(FALSE);

	while (TRUE)
	{
		fast_request_t *request;
		session_entry_t *found = NULL;
		time_t now;
		char *sid;

		thread_cancelability(TRUE);
		request = fast_request_create(this->fd, this->debug);
		thread_cancelability(FALSE);

		if (request == NULL)
		{
			break;
		}
		now = time_monotonic(NULL);
		sid = request->get_cookie(request, "SID");

		this->mutex->lock(this->mutex);
		if (sid)
		{
			found = this->sessions->get(this->sessions, sid);
		}
		if (found && !streq(found->host, request->get_host(request)))
		{
			found = NULL;
		}
		if (found)
		{
			
			while (found->in_use)
			{
				found->cond->wait(found->cond, this->mutex);
			}
		}
		else
		{	
			found = session_entry_create(this, request->get_host(request));
			if (!found)
			{
				request->destroy(request);
				this->mutex->unlock(this->mutex);
				continue;
			}
			sid = found->session->get_sid(found->session);
			this->sessions->put(this->sessions, sid, found);
		}
		found->in_use = TRUE;
		this->mutex->unlock(this->mutex);

		
		found->session->process(found->session, request);
		found->used = time_monotonic(NULL);

		
		this->mutex->lock(this->mutex);
		found->in_use = FALSE;
		found->closed = request->session_closed(request);
		found->cond->signal(found->cond);
		cleanup_sessions(this, now);
		this->mutex->unlock(this->mutex);

		request->destroy(request);
	}
}

METHOD(fast_dispatcher_t, run, void,
	private_fast_dispatcher_t *this, int threads)
{
	this->thread_count = threads;
	this->threads = malloc(sizeof(thread_t*) * threads);
	while (threads)
	{
		this->threads[threads - 1] = thread_create((thread_main_t)dispatch,
												   this);
		if (this->threads[threads - 1])
		{
			threads--;
		}
	}
}

METHOD(fast_dispatcher_t, waitsignal, void,
	private_fast_dispatcher_t *this)
{
	sigset_t set;
	int sig;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGHUP);
	sigprocmask(SIG_BLOCK, &set, NULL);
	sigwait(&set, &sig);
}

METHOD(fast_dispatcher_t, destroy, void,
	private_fast_dispatcher_t *this)
{
	char *sid;
	session_entry_t *entry;
	enumerator_t *enumerator;

	FCGX_ShutdownPending();
	while (this->thread_count--)
	{
		thread_t *thread = this->threads[this->thread_count];
		thread->cancel(thread);
		thread->join(thread);
	}
	enumerator = this->sessions->create_enumerator(this->sessions);
	while (enumerator->enumerate(enumerator, &sid, &entry))
	{
		session_entry_destroy(entry);
	}
	enumerator->destroy(enumerator);
	this->sessions->destroy(this->sessions);
	this->controllers->destroy_function(this->controllers, free);
	this->filters->destroy_function(this->filters, free);
	this->mutex->destroy(this->mutex);
	free(this->threads);
	free(this);
}

fast_dispatcher_t *fast_dispatcher_create(char *socket, bool debug, int timeout,
							fast_context_constructor_t constructor, void *param)
{
	private_fast_dispatcher_t *this;

	INIT(this,
		.public = {
			.add_controller = _add_controller,
			.add_filter = _add_filter,
			.run = _run,
			.waitsignal = _waitsignal,
			.destroy = _destroy,
		},
		.sessions = hashtable_create((void*)session_hash,
									 (void*)session_equals, 4096),
		.controllers = linked_list_create(),
		.filters = linked_list_create(),
		.context_constructor = constructor,
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.param = param,
		.timeout = timeout,
		.last_cleanup = time_monotonic(NULL),
		.debug = debug,
	);

	FCGX_Init();

	if (socket)
	{
		unlink(socket);
		this->fd = FCGX_OpenSocket(socket, 10);
	}
	return &this->public;
}
