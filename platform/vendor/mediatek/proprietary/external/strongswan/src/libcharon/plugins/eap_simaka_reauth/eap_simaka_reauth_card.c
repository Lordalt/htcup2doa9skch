/*
 * Copyright (C) 2009 Martin Willi
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

#include "eap_simaka_reauth_card.h"

#include <daemon.h>
#include <collections/hashtable.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <network/wod_channel.h>
#include <unistd.h>

#ifdef ANDROID
#include <private/android_filesystem_config.h> 
#include <sys/capability.h>
#include <linux/prctl.h>
#include <cutils/properties.h>
#endif

typedef struct private_eap_simaka_reauth_card_t private_eap_simaka_reauth_card_t;

struct private_eap_simaka_reauth_card_t {

	eap_simaka_reauth_card_t public;

	hashtable_t *reauth;
};

typedef struct {
	
	identification_t *id;
	
	identification_t *permanent;
	
	u_int16_t counter;
	
	char mk[HASH_SIZE_SHA1];
} reauth_data_t;

#ifdef ANDROID			 
char mtk_rand[AKA_RAND_LEN];
char mtk_auts[AKA_AUTS_LEN];
#endif

static u_int hash(identification_t *key)
{
	return chunk_hash(key->get_encoding(key));
}

static bool equals(identification_t *key1, identification_t *key2)
{
	return key1->equals(key1, key2);
}

METHOD(simaka_card_t, get_reauth, identification_t*,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char mk[HASH_SIZE_SHA1], u_int16_t *counter)
{
	reauth_data_t *data;
	identification_t *reauth;

	
	data = this->reauth->remove(this->reauth, id);
	if (!data)
	{
		return NULL;
	}
	*counter = ++data->counter;
	memcpy(mk, data->mk, HASH_SIZE_SHA1);
	reauth = data->id;
	data->permanent->destroy(data->permanent);
	free(data);
	return reauth;
}

METHOD(simaka_card_t, set_reauth, void,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	identification_t* next, char mk[HASH_SIZE_SHA1], u_int16_t counter)
{
	reauth_data_t *data;

	data = this->reauth->get(this->reauth, id);
	if (data)
	{
		data->id->destroy(data->id);
	}
	else
	{
		data = malloc_thing(reauth_data_t);
		data->permanent = id->clone(id);
		this->reauth->put(this->reauth, data->permanent, data);
	}
	data->counter = counter;
	data->id = next->clone(next);
	memcpy(data->mk, mk, HASH_SIZE_SHA1);
}

static char hexdig[] = "0123456789ABCDEF";


static int buf2hexstr(char *in_buf, int in_len, char *out_buf, int out_len)
{
	int i;

	if ((in_len*2 + 1) > out_len) {
		DBG1(DBG_IKE, "out_buf space not enough, in_len:%d, out_len:%d\n", in_len, out_len);
		return 0;
	}

	for (i = 0; i < in_len; i++)
	{
		out_buf[i*2]   = hexdig[(in_buf[i] >> 4) & 0xF];
		out_buf[i*2+1] = hexdig[(in_buf[i]     ) & 0xF];		
	}
	out_buf[in_len*2] = 0;

	return in_len*2;
}

static void showbufhex(char *msg, char *buf, int len)
{
	char hexstr[MAX_BUF_LEN] = {0};
	
	if (msg != NULL) {
		DBG1(DBG_IKE, "[%s] length: %d", msg, len);
	}
	buf2hexstr(buf, len, hexstr, MAX_BUF_LEN);
	DBG1(DBG_IKE, "%s", hexstr);
}

static unsigned char hex2bin(char hex)
{
	switch (hex)
	{
		case '0' ... '9':
			return hex - '0';
		case 'A' ... 'F':
			return hex - 'A' + 10;
		case 'a' ... 'f':
			return hex - 'a' + 10;
		default:
			return 0;
	}
}

static int hexstr2buf(char *hex_buf, int hex_len, char *bin_buf, int bin_len)
{
	int i;

	if (hex_len/2 > bin_len) {
		DBG1(DBG_IKE, "bin_buf space not enough, hex_len:%d, bin_len:%d\n", hex_len, bin_len);
		return 0;
	}

	for (i = 0; i < hex_len; i+=2)
	{
		bin_buf[i/2] = (hex2bin(hex_buf[i]) << 4) | hex2bin(hex_buf[i+1]);
	}

	return hex_len/2;
}

METHOD(simaka_card_t, resync, bool,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char auts[AKA_AUTS_LEN])
{

	if (memcmp(rand, mtk_rand, AKA_RAND_LEN) != 0) {
		return FALSE;
	}

	memcpy(auts, mtk_auts, AKA_AUTS_LEN);
	
	return TRUE;
}

METHOD(simaka_card_t, get_triplet, bool,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char rand[SIM_RAND_LEN], char sres[SIM_SRES_LEN], char kc[SIM_KC_LEN])
{
	char txbuf[MAX_BUF_LEN];
	char rxbuf[MAX_BUF_LEN];
	char rand_hexstr[SIM_RAND_LEN*2+1];
	int rxlen;

	char at_rsp[MAX_BUF_LEN/2];
	unsigned int sw1, sw2;
	char *ptr, *ptrE;
	int off, len;

	DBG1(DBG_IKE, "enter request SIM...");
	showbufhex("rand", rand, AKA_RAND_LEN);
	
	
	buf2hexstr(rand, SIM_RAND_LEN, rand_hexstr, SIM_RAND_LEN*2+1);
	sprintf(txbuf, "AT+EAUTH=\"%s\"\r", rand_hexstr);

	DBG1(DBG_IKE, "Send AT cmd: %s", txbuf);
	atcmd_txrx(txbuf, rxbuf, &rxlen);
	
	DBG1(DBG_IKE, "Got response: %s", rxbuf);
	if (strncmp(rxbuf, "+EAUTH:", 7) == 0)
	{
		
		
		ptr = ptrE = rxbuf+7;
		while (*ptrE != ',')
			ptrE++;
		*ptrE = 0;		
		sw1 = atol(ptr);
		
		
		ptr = ptrE + 1;
		while (*ptrE != ',')
			ptrE++;
		*ptrE = 0;
		sw2 = atol(ptr);
		
		
		ptr = ptrE + 1;
		while (*ptrE != '\"')
			ptrE++;
		ptr = ptrE + 1;
		ptrE = ptr+1;
		while (*ptrE != '\"')
			ptrE++;
		*ptrE = 0;
		strcpy(at_rsp, ptr);		
		DBG1(DBG_IKE, " SW[%02X%02X]: %s (%d)", sw1, sw2, at_rsp, strlen(at_rsp));

		rxlen = strlen(at_rsp);
		
		rxlen = hexstr2buf(at_rsp, rxlen, rxbuf, MAX_BUF_LEN);
		showbufhex("at_rsp", rxbuf, rxlen);
	
		
		
		
		
		

		off = 0;
		
		len = rxbuf[off++];
		if ((off + len) > rxlen)
		{
			return FALSE;
		}
		memcpy(sres, rxbuf+off, len);
		off += len;
		showbufhex("sRES", sres, len);
			
		
		len = rxbuf[off++];
		if ((off + len) > rxlen)
		{
			return FALSE;
		}
		memcpy(kc, rxbuf+off, len);
		off += len;
		showbufhex("Kc", kc, len);

		return TRUE;
	}
	return FALSE;
}

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	char txbuf[MAX_BUF_LEN];
	char rxbuf[MAX_BUF_LEN];
	char rand_hexstr[AKA_RAND_LEN*2+1];
	char autn_hexstr[AKA_AUTN_LEN*2+1];
	int rxlen;

	char at_rsp[MAX_BUF_LEN/2];
	unsigned int sw1, sw2;
	char *ptr, *ptrE;
	int off, len;
	
	DBG1(DBG_IKE, "enter...");
	showbufhex("rand", rand, AKA_RAND_LEN);
	showbufhex("autn", autn, AKA_AUTN_LEN);

	
	buf2hexstr(rand, AKA_RAND_LEN, rand_hexstr, AKA_RAND_LEN*2+1);
	buf2hexstr(autn, AKA_AUTN_LEN, autn_hexstr, AKA_AUTN_LEN*2+1);
	sprintf(txbuf, "AT+EAUTH=\"%s\",\"%s\"\r", rand_hexstr, autn_hexstr);

	DBG1(DBG_IKE, "Send AT cmd: %s", txbuf);
	atcmd_txrx(txbuf, rxbuf, &rxlen);
	
	DBG1(DBG_IKE, "Got response: %s", rxbuf);
	if (strncmp(rxbuf, "+EAUTH:", 7) == 0)
	{
		
		
		ptr = ptrE = rxbuf+7;
		while (*ptrE != ',')
			ptrE++;
		*ptrE = 0;		
		sw1 = atol(ptr);
		
		
		ptr = ptrE + 1;
		while (*ptrE != ',' && ptrE <= (rxbuf+rxlen))
			ptrE++;
		if (*ptrE == ',')
		{
			*ptrE = 0;
			sw2 = atol(ptr);			
			
			ptr = ptrE + 1;
			while (*ptrE != '\"')
				ptrE++;
			ptr = ptrE + 1;
			ptrE = ptr+1;
			while (*ptrE != '\"')
				ptrE++;
			*ptrE = 0;
			strcpy(at_rsp, ptr);		
			DBG1(DBG_IKE, " SW[%02X%02X]: %s (%d)", sw1, sw2, at_rsp, strlen(at_rsp));

			rxlen = strlen(at_rsp);
			
			rxlen = hexstr2buf(at_rsp, rxlen, rxbuf, MAX_BUF_LEN);

			showbufhex("at_rsp", rxbuf, rxlen);
			
		} 
		else
		{
			*ptrE = 0;
			sw2 = atol(ptr);			
			DBG1(DBG_IKE, " SW[%02X%02X]", sw1, sw2);
			rxbuf[0] = 0;
		}
		
		off = 0;
		
		if ((unsigned char)rxbuf[off] == 0xDB)
		{
			DBG1(DBG_IKE, "Go DB");			
			
			
			
			
			
			
			
			
			off++;
			
			len = rxbuf[off++];
			memcpy(res, rxbuf+off, len);
			off += len;
			*res_len = len;
			showbufhex("RES", res, len);
				
			
			len = rxbuf[off++];
			memcpy(ck, rxbuf+off, len);
			off += len;
			showbufhex("CK", ck, len);

			
			len = rxbuf[off++];
			memcpy(ik, rxbuf+off, len);
			off += len;
			showbufhex("IK", ik, len);

			return SUCCESS;
		}
		else if ((unsigned char)rxbuf[off] == 0xDC)
		{
			
			
			
			
			DBG1(DBG_IKE, "Go DC");			

			off++;
			
			len = rxbuf[off++];
			memcpy(mtk_auts, rxbuf+off, len);
			off += len;
			showbufhex("AUTS", mtk_auts, len);
			memcpy(mtk_rand, rand, AKA_RAND_LEN);
			
			return INVALID_STATE;
		}
		else
		{
			DBG1(DBG_IKE, "Go Other");			
		}
	}
	
	return FAILED;
}

METHOD(eap_simaka_reauth_card_t, destroy, void,
	private_eap_simaka_reauth_card_t *this)
{
	enumerator_t *enumerator;
	reauth_data_t *data;
	void *key;

	enumerator = this->reauth->create_enumerator(this->reauth);
	while (enumerator->enumerate(enumerator, &key, &data))
	{
		data->id->destroy(data->id);
		data->permanent->destroy(data->permanent);
		free(data);
	}
	enumerator->destroy(enumerator);

	this->reauth->destroy(this->reauth);
	free(this);
}

eap_simaka_reauth_card_t *eap_simaka_reauth_card_create()
{
	private_eap_simaka_reauth_card_t *this;

	if (!lib->caps->check(lib->caps, CAP_NET_BIND_SERVICE))
	{	
		DBG1(DBG_NET, "eap_simaka_reauth plugin requires CAP_NET_BIND_SERVICE capability");
		return NULL;
	}
	else if (!lib->caps->keep(lib->caps, CAP_NET_RAW))
	{	
		
		DBG1(DBG_NET, "eap_simaka_reauth plugin requires CAP_NET_RAW capability");
		return NULL;
	}

	INIT(this,
		.public = {
			.card = {
				.get_triplet = _get_triplet,
				.get_quintuplet = _get_quintuplet,
				.resync = _resync,
				.get_pseudonym = (void*)return_null,
				.set_pseudonym = (void*)nop,
				.get_reauth = _get_reauth,
				.set_reauth = _set_reauth,
			},
			.destroy = _destroy,
		},
		.reauth = hashtable_create((void*)hash, (void*)equals, 0),
	);

	return &this->public;
}

