/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2006-2009 Martin Willi
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


#ifndef BUS_H_
#define BUS_H_

typedef enum alert_t alert_t;
typedef enum narrow_hook_t narrow_hook_t;
typedef struct bus_t bus_t;

#include <stdarg.h>

#include <utils/debug.h>
#include <sa/ike_sa.h>
#include <sa/child_sa.h>
#include <processing/jobs/job.h>
#include <bus/listeners/logger.h>
#include <bus/listeners/listener.h>

#undef DBG0
#undef DBG1
#undef DBG2
#undef DBG3
#undef DBG4

#ifndef DEBUG_LEVEL
# define DEBUG_LEVEL 4
#endif 

#if DEBUG_LEVEL >= 0
#define DBG0(group, format, ...) charon->bus->log(charon->bus, group, 0, format, ##__VA_ARGS__)
#endif 
#if DEBUG_LEVEL >= 1
#define DBG1(group, format, ...) charon->bus->log(charon->bus, group, 1, "[%s() %4d] " format, &__FUNCTION__[0], __LINE__, ##__VA_ARGS__)
#endif 
#if DEBUG_LEVEL >= 2
#define DBG2(group, format, ...) charon->bus->log(charon->bus, group, 2, "[%s() %4d] " format, &__FUNCTION__[0], __LINE__, ##__VA_ARGS__)
#endif 
#if DEBUG_LEVEL >= 3
#define DBG3(group, format, ...) charon->bus->log(charon->bus, group, 3, "[%s() %4d] " format, &__FUNCTION__[0], __LINE__, ##__VA_ARGS__)
#endif 
#if DEBUG_LEVEL >= 4
#define DBG4(group, format, ...) charon->bus->log(charon->bus, group, 4, "[%s() %4d] " format, &__FUNCTION__[0], __LINE__, ##__VA_ARGS__)
#endif 

#ifndef DBG0
# define DBG0(...) {}
#endif 
#ifndef DBG1
# define DBG1(...) {}
#endif 
#ifndef DBG2
# define DBG2(...) {}
#endif 
#ifndef DBG3
# define DBG3(...) {}
#endif 
#ifndef DBG4
# define DBG4(...) {}
#endif 

enum alert_t {
	
	ALERT_RADIUS_NOT_RESPONDING,
	
	ALERT_SHUTDOWN_SIGNAL,
	
	ALERT_LOCAL_AUTH_FAILED,
	
	ALERT_PEER_AUTH_FAILED,
	
	ALERT_PEER_ADDR_FAILED,
	
	ALERT_PEER_INIT_UNREACHABLE,
	
	ALERT_INVALID_IKE_SPI,
	
	ALERT_PARSE_ERROR_HEADER,
	ALERT_PARSE_ERROR_BODY,
	
	ALERT_RETRANSMIT_SEND,
	
	ALERT_RETRANSMIT_SEND_TIMEOUT,
	
	ALERT_RETRANSMIT_RECEIVE,
	
	ALERT_HALF_OPEN_TIMEOUT,
	
	ALERT_PROPOSAL_MISMATCH_IKE,
	
	ALERT_PROPOSAL_MISMATCH_CHILD,
	ALERT_TS_MISMATCH,
	ALERT_TS_NARROWED,
	
	ALERT_INSTALL_CHILD_SA_FAILED,
	
	ALERT_INSTALL_CHILD_POLICY_FAILED,
	
	ALERT_UNIQUE_REPLACE,
	
	ALERT_UNIQUE_KEEP,
	
	ALERT_KEEP_ON_CHILD_SA_FAILURE,
	
	ALERT_VIP_FAILURE,
	
	ALERT_AUTHORIZATION_FAILED,
	
	ALERT_IKE_SA_EXPIRED,
	
	ALERT_CERT_EXPIRED,
	
	ALERT_CERT_REVOKED,
	
	ALERT_CERT_VALIDATION_FAILED,
	
	ALERT_CERT_NO_ISSUER,
	
	ALERT_CERT_UNTRUSTED_ROOT,
	
	ALERT_CERT_EXCEEDED_PATH_LEN,
	
	ALERT_CERT_POLICY_VIOLATION,
};

enum narrow_hook_t {
	
	NARROW_INITIATOR_PRE_NOAUTH,
	
	NARROW_INITIATOR_PRE_AUTH,
	
	NARROW_RESPONDER,
	
	NARROW_RESPONDER_POST,
	
	NARROW_INITIATOR_POST_NOAUTH,
	
	NARROW_INITIATOR_POST_AUTH,
};

struct bus_t {

	void (*add_listener) (bus_t *this, listener_t *listener);

	void (*remove_listener) (bus_t *this, listener_t *listener);

	void (*add_logger) (bus_t *this, logger_t *logger);

	void (*remove_logger) (bus_t *this, logger_t *logger);

	void (*set_sa) (bus_t *this, ike_sa_t *ike_sa);

	ike_sa_t* (*get_sa)(bus_t *this);

	void (*log)(bus_t *this, debug_t group, level_t level, char* format, ...);

	void (*vlog)(bus_t *this, debug_t group, level_t level,
				 char* format, va_list args);

	void (*alert)(bus_t *this, alert_t alert, ...);

	void (*ike_state_change)(bus_t *this, ike_sa_t *ike_sa,
							 ike_sa_state_t state);
	void (*child_state_change)(bus_t *this, child_sa_t *child_sa,
							   child_sa_state_t state);
	void (*message)(bus_t *this, message_t *message, bool incoming, bool plain);

	bool (*authorize)(bus_t *this, bool final);

	void (*narrow)(bus_t *this, child_sa_t *child_sa, narrow_hook_t type,
				   linked_list_t *local, linked_list_t *remote);

	void (*ike_keys)(bus_t *this, ike_sa_t *ike_sa, diffie_hellman_t *dh,
					 chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r,
					 ike_sa_t *rekey, shared_key_t *shared);

	void (*child_keys)(bus_t *this, child_sa_t *child_sa, bool initiator,
					   diffie_hellman_t *dh, chunk_t nonce_i, chunk_t nonce_r);

	void (*ike_updown)(bus_t *this, ike_sa_t *ike_sa, bool up);

	void (*ike_rekey)(bus_t *this, ike_sa_t *old, ike_sa_t *new);

	void (*ike_reestablish)(bus_t *this, ike_sa_t *old, ike_sa_t *new);

	void (*child_updown)(bus_t *this, child_sa_t *child_sa, bool up);

	void (*child_rekey)(bus_t *this, child_sa_t *old, child_sa_t *new);

	void (*assign_vips)(bus_t *this, ike_sa_t *ike_sa, bool assign);

	void (*destroy) (bus_t *this);
};

bus_t *bus_create();

#endif 
