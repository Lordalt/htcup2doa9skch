/*
 * Copyright (C) 2006-2013 Tobias Brunner
 * Copyright (C) 2005-2013 Martin Willi
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
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <getopt.h>

#include <library.h>
#include <hydra.h>
#include <daemon.h>
#include <utils/backtrace.h>
#include <threading/thread.h>

#include "cmd/cmd_options.h"
#include "cmd/cmd_connection.h"
#include "cmd/cmd_creds.h"

static level_t default_loglevel = LEVEL_CTRL;

static level_t levels[DBG_MAX];

static cmd_connection_t *conn;

static cmd_creds_t *creds;

extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

static void dbg_stderr(debug_t group, level_t level, char *fmt, ...)
{
	va_list args;

	if (level <= default_loglevel)
	{
		va_start(args, fmt);
		fprintf(stderr, "00[%N] ", debug_names, group);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
}

static void cleanup_conn()
{
	DESTROY_IF(conn);
}

static void cleanup_creds()
{
	DESTROY_IF(creds);
}

static int run()
{
	sigset_t set;

	
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);

	while (TRUE)
	{
		int sig;
		int error;

		error = sigwait(&set, &sig);
		if (error)
		{
			DBG1(DBG_DMN, "error %d while waiting for a signal", error);
			return 1;
		}
		switch (sig)
		{
			case SIGHUP:
			{
				DBG1(DBG_DMN, "signal of type SIGHUP received. Reloading "
					 "configuration");
				if (lib->settings->load_files(lib->settings, NULL, FALSE))
				{
					charon->load_loggers(charon, levels, TRUE);
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
				return 0;
			}
			case SIGTERM:
			{
				DBG1(DBG_DMN, "signal of type SIGTERM received. Shutting down");
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return 0;
			}
			case SIGUSR1:
			{	
				charon->bus->alert(charon->bus, ALERT_SHUTDOWN_SIGNAL, sig);
				return 1;
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

static void usage(FILE *out, char *msg, char *binary)
{
	static const int padto = 18;
	char cmd[64], *pre, *post;
	int i, line, pad;

	if (msg)
	{
		fprintf(out, "%s\n", msg);
	}
	fprintf(out, "Usage: %s\n", binary);
	for (i = 0; i < CMD_OPT_COUNT; i++)
	{
		switch (cmd_options[i].has_arg)
		{
			case required_argument:
				pre = " <";
				post = ">";
				break;
			case optional_argument:
				pre = "[=";
				post = "]";
				break;
			case no_argument:
			default:
				pre = "  ";
				post = " ";
				break;
		}
		snprintf(cmd, sizeof(cmd), "  --%s%s%s%s", cmd_options[i].name,
				 pre, cmd_options[i].arg, post);
		pad = padto - strlen(cmd);
		if (pad >= 1)
		{
			fprintf(out, "%s%-*s%s\n", cmd, pad, "", cmd_options[i].desc);
		}
		else
		{	
			fprintf(out, "%s\n%-*s%s\n", cmd, padto, "", cmd_options[i].desc);
		}
		for (line = 0; line < countof(cmd_options[i].lines); line++)
		{
			if (cmd_options[i].lines[line])
			{
				fprintf(out, "%-*s%s\n", padto, "", cmd_options[i].lines[line]);
			}
		}
	}
}

static void handle_arguments(int argc, char *argv[], bool simple)
{
	struct option long_opts[CMD_OPT_COUNT + 1] = {};
	int i, opt;

	for (i = 0; i < CMD_OPT_COUNT; i++)
	{
		long_opts[i].name = cmd_options[i].name;
		long_opts[i].val = cmd_options[i].id;
		long_opts[i].has_arg = cmd_options[i].has_arg;
	}
	
	optind = 1;
	while (TRUE)
	{
		bool handled = FALSE;

		opt = getopt_long(argc, argv, "", long_opts, NULL);
		switch (opt)
		{
			case EOF:
				break;
			case CMD_OPT_HELP:
				usage(stdout, NULL, argv[0]);
				exit(0);
			case CMD_OPT_VERSION:
				printf("%s, strongSwan %s\n", "charon-cmd", VERSION);
				exit(0);
			case CMD_OPT_DEBUG:
				default_loglevel = atoi(optarg);
				continue;
			default:
				if (simple)
				{
					continue;
				}
				handled |= conn->handle(conn, opt, optarg);
				handled |= creds->handle(creds, opt, optarg);
				if (handled)
				{
					continue;
				}
				
			case '?':
				
				usage(stderr, NULL, argv[0]);
				exit(1);
		}
		break;
	}
}

int main(int argc, char *argv[])
{
	struct sigaction action;
	struct utsname utsname;
	int group;

	
	handle_arguments(argc, argv, TRUE);

	dbg = dbg_stderr;
	atexit(library_deinit);
	if (!library_init(NULL, "charon-cmd"))
	{
		exit(SS_RC_LIBSTRONGSWAN_INTEGRITY);
	}
	if (lib->integrity)
	{
		if (!lib->integrity->check_file(lib->integrity, "charon-cmd", argv[0]))
		{
			exit(SS_RC_DAEMON_INTEGRITY);
		}
	}
	atexit(libhydra_deinit);
	if (!libhydra_init())
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	atexit(libcharon_deinit);
	if (!libcharon_init())
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	for (group = 0; group < DBG_MAX; group++)
	{
		levels[group] = default_loglevel;
	}
	charon->load_loggers(charon, levels, TRUE);

	if (!lookup_uid_gid())
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	lib->settings->set_default_str(lib->settings, "charon-cmd.port", "0");
	lib->settings->set_default_str(lib->settings, "charon-cmd.port_nat_t", "0");
	if (!charon->initialize(charon,
			lib->settings->get_str(lib->settings, "charon-cmd.load", PLUGINS)))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}
	if (!lib->caps->drop(lib->caps))
	{
		exit(SS_RC_INITIALIZATION_FAILED);
	}

	conn = cmd_connection_create();
	atexit(cleanup_conn);
	creds = cmd_creds_create();
	atexit(cleanup_creds);

	
	handle_arguments(argc, argv, FALSE);

	if (uname(&utsname) != 0)
	{
		memset(&utsname, 0, sizeof(utsname));
	}
	DBG1(DBG_DMN, "Starting charon-cmd IKE client (strongSwan %s, %s %s, %s)",
		 VERSION, utsname.sysname, utsname.release, utsname.machine);
	lib->plugins->status(lib->plugins, LEVEL_CTRL);

	action.sa_handler = segv_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGINT);
	sigaddset(&action.sa_mask, SIGTERM);
	sigaddset(&action.sa_mask, SIGHUP);
	sigaddset(&action.sa_mask, SIGUSR1);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGILL, &action, NULL);
	sigaction(SIGBUS, &action, NULL);
	action.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &action, NULL);

	pthread_sigmask(SIG_SETMASK, &action.sa_mask, NULL);

	
	charon->start(charon);
	
	return run();
}
