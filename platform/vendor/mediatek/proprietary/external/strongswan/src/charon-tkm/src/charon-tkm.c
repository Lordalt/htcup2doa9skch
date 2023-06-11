/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#define _GNU_SOURCE

#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

#include <hydra.h>
#include <daemon.h>
#include <library.h>
#include <utils/backtrace.h>
#include <threading/thread.h>
#include <sa/keymat.h>
#include <credentials/credential_manager.h>

#include "tkm.h"
#include "tkm_nonceg.h"
#include "tkm_diffie_hellman.h"
#include "tkm_keymat.h"
#include "tkm_listener.h"
#include "tkm_kernel_ipsec.h"
#include "tkm_public_key.h"
#include "tkm_cred.h"
#include "tkm_encoder.h"

static tkm_listener_t *listener;

static char *pidfile_name = NULL;

static FILE *pidfile = NULL;

extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

static void dbg_syslog(debug_t group, level_t level, char *fmt, ...)
{
	if (level <= 1)
	{
		char buffer[8192];
		va_list args;

		va_start(args, fmt);
		
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		syslog(LOG_DAEMON|LOG_INFO, "00[%s] %s", debug_names->names[group],
				buffer);
		va_end(args);
	}
}

static void run()
{
	sigset_t set;

	
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	while (TRUE)
	{
		int sig;
		int error;

		error = sigwait(&set, &sig);
		if (error)
		{
			DBG1(DBG_DMN, "error %d while waiting for a signal", error);
			return;
		}
		switch (sig)
		{
			case SIGINT:
			{
				DBG1(DBG_DMN, "signal of type SIGINT received. Shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return;
			}
			case SIGTERM:
			{
				DBG1(DBG_DMN, "signal of type SIGTERM received. Shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return;
			}
			default:
			{
				DBG1(DBG_DMN, "unknown signal %d received. Ignored", sig);
				break;
			}
		}
	}
}

static void segv_handler(int signal)
{
	backtrace_t *backtrace;

	DBG1(DBG_DMN, "thread %u received %d", thread_current_id(), signal);
	backtrace = backtrace_create(2);
	backtrace->log(backtrace, stderr, TRUE);
	backtrace->destroy(backtrace);

	DBG1(DBG_DMN, "killing ourself, received critical signal");
	abort();
}

static bool lookup_uid_gid()
{
#ifdef IPSEC_USER
	if (!lib->caps->resolve_uid(lib->caps, IPSEC_USER))
	{
		return FALSE;
	}
#endif
#ifdef IPSEC_GROUP
	if (!lib->caps->resolve_gid(lib->caps, IPSEC_GROUP))
	{
		return FALSE;
	}
#endif
	return TRUE;
}

static bool check_pidfile()
{
	struct stat stb;

	if (stat(pidfile_name, &stb) == 0)
	{
		pidfile = fopen(pidfile_name, "r");
		if (pidfile)
		{
			char buf[64];
			pid_t pid = 0;

			memset(buf, 0, sizeof(buf));
			if (fread(buf, 1, sizeof(buf), pidfile))
			{
				buf[sizeof(buf) - 1] = '\0';
				pid = atoi(buf);
			}
			fclose(pidfile);
			if (pid && kill(pid, 0) == 0)
			{	
				return TRUE;
			}
		}
		DBG1(DBG_DMN, "removing pidfile '%s', process not running", pidfile_name);
		unlink(pidfile_name);
	}

	
	pidfile = fopen(pidfile_name, "w");
	if (pidfile)
	{
		ignore_result(fchown(fileno(pidfile),
							 lib->caps->get_uid(lib->caps),
							 lib->caps->get_gid(lib->caps)));
		fprintf(pidfile, "%d\n", getpid());
		fflush(pidfile);
	}
	return FALSE;
}

static void unlink_pidfile()
{
	if (pidfile)
	{
		ignore_result(ftruncate(fileno(pidfile), 0));
		fclose(pidfile);
	}
	unlink(pidfile_name);
}
int main(int argc, char *argv[])
{
	char *dmn_name;
	if (argc > 0 && strlen(argv[0]) > 0)
	{
		dmn_name = basename(argv[0]);
	}
	else
	{
		dmn_name = "charon-tkm";
	}

	
	tkm_cred_t *creds;

	struct sigaction action;
	int status = SS_RC_INITIALIZATION_FAILED;

	
	dbg = dbg_syslog;

	
	if (!library_init(NULL, dmn_name))
	{
		library_deinit();
		exit(status);
	}

	if (!libhydra_init())
	{
		dbg_syslog(DBG_DMN, 1, "initialization failed - aborting %s", dmn_name);
		libhydra_deinit();
		library_deinit();
		exit(status);
	}

	if (!libcharon_init())
	{
		dbg_syslog(DBG_DMN, 1, "initialization failed - aborting %s", dmn_name);
		goto deinit;
	}

	if (!lookup_uid_gid())
	{
		dbg_syslog(DBG_DMN, 1, "invalid uid/gid - aborting %s", dmn_name);
		goto deinit;
	}

	
	lib->settings->set_int(lib->settings, "%s.syslog.daemon.default",
			lib->settings->get_int(lib->settings, "%s.syslog.daemon.default", 1,
								   dmn_name), dmn_name);
	charon->load_loggers(charon, NULL, FALSE);

	DBG1(DBG_DMN, "Starting charon with TKM backend (strongSwan "VERSION")");

	
	static plugin_feature_t features[] = {
		PLUGIN_REGISTER(NONCE_GEN, tkm_nonceg_create),
			PLUGIN_PROVIDE(NONCE_GEN),
		PLUGIN_REGISTER(PUBKEY, tkm_public_key_load, TRUE),
			PLUGIN_PROVIDE(PUBKEY, KEY_RSA),
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA1),
			PLUGIN_PROVIDE(PUBKEY_VERIFY, SIGN_RSA_EMSA_PKCS1_SHA256),
		PLUGIN_CALLBACK(kernel_ipsec_register, tkm_kernel_ipsec_create),
			PLUGIN_PROVIDE(CUSTOM, "kernel-ipsec"),
	};
	lib->plugins->add_static_features(lib->plugins, "tkm-backend", features,
			countof(features), TRUE);

	if (!register_dh_mapping())
	{
		DBG1(DBG_DMN, "no DH group mapping defined - aborting %s", dmn_name);
		goto deinit;
	}

	
	keymat_register_constructor(IKEV2, (keymat_constructor_t)tkm_keymat_create);

	
	if (!charon->initialize(charon, PLUGINS))
	{
		DBG1(DBG_DMN, "initialization failed - aborting %s", dmn_name);
		goto deinit;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	
	if (asprintf(&pidfile_name, IPSEC_PIDDIR"/%s.pid", dmn_name) < 0)
	{
		DBG1(DBG_DMN, "unable to set pidfile name - aborting %s", dmn_name);
		goto deinit;
	};

	if (check_pidfile())
	{
		DBG1(DBG_DMN, "%s already running (\"%s\" exists)", dmn_name,
			 pidfile_name);
		goto deinit;
	}

	if (!lib->caps->drop(lib->caps))
	{
		DBG1(DBG_DMN, "capability dropping failed - aborting %s", dmn_name);
		goto deinit;
	}

	
	if (!tkm_init())
	{
		DBG1(DBG_DMN, "init of TKM client failed - aborting %s", dmn_name);
		goto deinit;
	}

	
	listener = tkm_listener_create();
	charon->bus->add_listener(charon->bus, &listener->listener);

	
	creds = tkm_cred_create();
	lib->credmgr->add_set(lib->credmgr, (credential_set_t*)creds);

	
	lib->encoding->add_encoder(lib->encoding, tkm_encoder_encode);

	action.sa_handler = segv_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);

	pthread_sigmask(SIG_SETMASK, &action.sa_mask, NULL);

	
	charon->start(charon);

	
	run();

	unlink_pidfile();
	status = 0;
	charon->bus->remove_listener(charon->bus, &listener->listener);
	listener->destroy(listener);
	creds->destroy(creds);
	lib->encoding->remove_encoder(lib->encoding, tkm_encoder_encode);

deinit:
	destroy_dh_mapping();
	libcharon_deinit();
	libhydra_deinit();
	library_deinit();
	tkm_deinit();
	return status;
}
