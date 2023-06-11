/*
 * Copyright (C) 2006-2012 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
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

#include <stdio.h>
#define _POSIX_PTHREAD_SEMANTICS 
#include <signal.h>
#undef _POSIX_PTHREAD_SEMANTICS
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <getopt.h>

#include <hydra.h>
#include <daemon.h>

#include <library.h>
#include <utils/backtrace.h>
#include <threading/thread.h>

#ifdef ANDROID
#include <private/android_filesystem_config.h> 
#endif

#define PID_FILE IPSEC_PIDDIR "/charon.pid"

#ifndef IPSEC_USER
#define IPSEC_USER NULL
#endif

#ifndef IPSEC_GROUP
#define IPSEC_GROUP NULL
#endif

static FILE *pidfile = NULL;

static level_t levels[DBG_MAX];

static bool use_syslog = FALSE;

extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

static void dbg_stderr(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	if (level <= 1)
	{
		va_start(args, fmt);
		fprintf(stderr, "00[%N] ", debug_names, group);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
}

static void run()
{
	sigset_t set;

	
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGHUP);
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
			case SIGHUP:
			{
				DBG1(DBG_DMN, "signal of type SIGHUP received. Reloading "
					 "configuration");
				if (lib->settings->load_files(lib->settings, NULL, FALSE))
				{
					charon->load_loggers(charon, levels, !use_syslog);
					lib->plugins->reload(lib->plugins, NULL);
				}
				else
				{
					DBG1(DBG_DMN, "reloading config failed, keeping old");
				}
				break;
			}
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

static bool lookup_uid_gid()
{
	char *name;

	name = lib->settings->get_str(lib->settings, "charon.user", IPSEC_USER);
	if (name && !lib->caps->resolve_uid(lib->caps, name))
	{
		return FALSE;
	}
	name = lib->settings->get_str(lib->settings, "charon.group", IPSEC_GROUP);
	if (name && !lib->caps->resolve_gid(lib->caps, name))
	{
		return FALSE;
	}
#ifdef ANDROID
#endif
	return TRUE;
}

static void segv_handler(int signal)
{
	backtrace_t *backtrace;

	DBG1(DBG_DMN, "thread %u received %d", thread_current_id(), signal);
	backtrace = backtrace_create(2);
	backtrace->log(backtrace, NULL, TRUE);
	backtrace->log(backtrace, stderr, TRUE);
	backtrace->destroy(backtrace);

	DBG1(DBG_DMN, "killing ourself, received critical signal");
	if (signal == SIGABRT)
	{
		exit(EXIT_FAILURE);
	}
	else
	{
		abort();
	}
}

static bool check_pidfile()
{
	struct stat stb;

	if (stat(PID_FILE, &stb) == 0)
	{
		pidfile = fopen(PID_FILE, "r");
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
		DBG1(DBG_DMN, "removing pidfile '"PID_FILE"', process not running");
		unlink(PID_FILE);
	}

	
	pidfile = fopen(PID_FILE, "w");
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
	unlink(PID_FILE);
}

static void usage(const char *msg)
{
	if (msg != NULL && *msg != '\0')
	{
		fprintf(stderr, "%s\n", msg);
	}
	fprintf(stderr, "Usage: charon\n"
					"         [--help]\n"
					"         [--version]\n"
					"         [--use-syslog]\n"
					"         [--debug-<type> <level>]\n"
					"           <type>:  log context type (dmn|mgr|ike|chd|job|cfg|knl|net|asn|enc|tnc|imc|imv|pts|tls|esp|lib)\n"
					"           <level>: log verbosity (-1 = silent, 0 = audit, 1 = control,\n"
					"                                    2 = controlmore, 3 = raw, 4 = private)\n"
					"\n"
		   );
}

int main(int argc, char *argv[])
{
	struct sigaction action;
	int group, status = SS_RC_INITIALIZATION_FAILED;
	struct utsname utsname;

	
	dbg = dbg_stderr;

	
	if (!library_init(NULL, "charon"))
	{
		library_deinit();
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}

	if (lib->integrity &&
		!lib->integrity->check_file(lib->integrity, "charon", argv[0]))
	{
		dbg_stderr(DBG_DMN, 1, "integrity check of charon failed");
		library_deinit();
		exit(SS_RC_DAEMON_INTEGRITY);
	}

	if (!libhydra_init())
	{
		dbg_stderr(DBG_DMN, 1, "initialization failed - aborting charon");
		libhydra_deinit();
		library_deinit();
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	if (!libcharon_init())
	{
		dbg_stderr(DBG_DMN, 1, "initialization failed - aborting charon");
		goto deinit;
	}

	
	for (group = 0; group < DBG_MAX; group++)
	{
		levels[group] = LEVEL_CTRL;
	}

	
	for (;;)
	{
		struct option long_opts[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "version", no_argument, NULL, 'v' },
			{ "use-syslog", no_argument, NULL, 'l' },
			
			{ "debug-dmn", required_argument, &group, DBG_DMN },
			{ "debug-mgr", required_argument, &group, DBG_MGR },
			{ "debug-ike", required_argument, &group, DBG_IKE },
			{ "debug-chd", required_argument, &group, DBG_CHD },
			{ "debug-job", required_argument, &group, DBG_JOB },
			{ "debug-cfg", required_argument, &group, DBG_CFG },
			{ "debug-knl", required_argument, &group, DBG_KNL },
			{ "debug-net", required_argument, &group, DBG_NET },
			{ "debug-asn", required_argument, &group, DBG_ASN },
			{ "debug-enc", required_argument, &group, DBG_ENC },
			{ "debug-tnc", required_argument, &group, DBG_TNC },
			{ "debug-imc", required_argument, &group, DBG_IMC },
			{ "debug-imv", required_argument, &group, DBG_IMV },
			{ "debug-pts", required_argument, &group, DBG_PTS },
			{ "debug-tls", required_argument, &group, DBG_TLS },
			{ "debug-esp", required_argument, &group, DBG_ESP },
			{ "debug-lib", required_argument, &group, DBG_LIB },
			{ 0,0,0,0 }
		};

		int c = getopt_long(argc, argv, "", long_opts, NULL);
		switch (c)
		{
			case EOF:
				break;
			case 'h':
				usage(NULL);
				status = 0;
				goto deinit;
			case 'v':
				printf("Linux strongSwan %s\n", VERSION);
				status = 0;
				goto deinit;
			case 'l':
				use_syslog = TRUE;
				continue;
			case 0:
				
				levels[group] = atoi(optarg);
				continue;
			default:
				usage("");
				status = 1;
				goto deinit;
		}
		break;
	}

	if (!lookup_uid_gid())
	{
		dbg_stderr(DBG_DMN, 1, "invalid uid/gid - aborting charon");
		goto deinit;
	}

	charon->load_loggers(charon, levels, !use_syslog);

	if (uname(&utsname) != 0)
	{
		memset(&utsname, 0, sizeof(utsname));
	}
	DBG1(DBG_DMN, "Starting IKE charon daemon (strongSwan "VERSION", %s %s, %s)",
		  utsname.sysname, utsname.release, utsname.machine);
	if (lib->integrity)
	{
		DBG1(DBG_DMN, "integrity tests enabled:");
		DBG1(DBG_DMN, "lib    'libstrongswan': passed file and segment integrity tests");
		DBG1(DBG_DMN, "lib    'libhydra': passed file and segment integrity tests");
		DBG1(DBG_DMN, "lib    'libcharon': passed file and segment integrity tests");
		DBG1(DBG_DMN, "daemon 'charon': passed file integrity test");
	}

	
	if (!charon->initialize(charon,
				lib->settings->get_str(lib->settings, "charon.load", PLUGINS)))
	{
		DBG1(DBG_DMN, "initialization failed - aborting charon");
		goto deinit;
	}
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	if (check_pidfile())
	{
		DBG1(DBG_DMN, "charon already running (\""PID_FILE"\" exists)");
		goto deinit;
	}

	if (!lib->caps->drop(lib->caps))
	{
		DBG1(DBG_DMN, "capability dropping failed - aborting charon");
		goto deinit;
	}

	action.sa_handler = segv_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);
	sigaddset(&action.sa_mask, SIGHUP);
	sigaction(SIGABRT, &action, NULL);
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);

	pthread_sigmask(SIG_SETMASK, &action.sa_mask, NULL);

	
	charon->start(charon);

	
	run();

	
	unlink_pidfile();
	status = 0;

deinit:
	libcharon_deinit();
	libhydra_deinit();
	library_deinit();
	return status;
}

