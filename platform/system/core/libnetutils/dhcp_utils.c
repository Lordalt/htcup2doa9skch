/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cutils/properties.h>
#include <netutils/dhcp_utils.h>

static const char DAEMON_NAME[]        = "dhcpcd";
static const char DAEMON_PROP_NAME[]   = "init.svc.dhcpcd";
static const char HOSTNAME_PROP_NAME[] = "net.hostname";
static const char DHCP_PROP_NAME_PREFIX[]  = "dhcp";
static const char DHCP_CONFIG_PATH[]   = "/system/etc/dhcpcd/dhcpcd.conf";
static const int NAP_TIME = 50;   
                                  
static const char DAEMON_NAME_RENEW[]  = "iprenew";
static char errmsg[100] = "\0";
#define MAX_INTERFACE_LENGTH 25

void get_p2p_interface_replacement(const char *interface, char *p2p_interface) {
    
    if (strncmp(interface, "p2p",3) == 0) {
        strncpy(p2p_interface, "p2p", MAX_INTERFACE_LENGTH);
    } else {
        strncpy(p2p_interface, interface, MAX_INTERFACE_LENGTH);
    }
}

static int wait_for_property(const char *name, const char *desired_value, int maxwait)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int maxnaps = (maxwait * 1000) / NAP_TIME;

    if (maxnaps < 1) {
        maxnaps = 1;
    }

    while (maxnaps-- >= 0) {
        if (property_get(name, value, NULL)) {
            if (desired_value == NULL) {
                if (value != NULL && strcmp(value, "obtaining"))
                    return 0;
            } else if (strcmp(value, desired_value) == 0) {
                return 0;
            }
        }
        if (maxnaps >= 0) {
            usleep(NAP_TIME * 1000);
        }
    }
    return -1; 
}

static int fill_ip_info(const char *interface,
                     char *ipaddr,
                     char *gateway,
                     uint32_t *prefixLength,
                     char *dns[],
                     char *server,
                     uint32_t *lease,
                     char *vendorInfo,
                     char *domain,
                     char *mtu)
{
    char prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX];
    
    char p2p_interface[MAX_INTERFACE_LENGTH];
    int x;

    get_p2p_interface_replacement(interface, p2p_interface);

    snprintf(prop_name, sizeof(prop_name), "%s.%s.ipaddress", DHCP_PROP_NAME_PREFIX, p2p_interface);
#if 0
    property_get(prop_name, ipaddr, NULL);
#endif
    if (!property_get(prop_name, ipaddr, NULL)) {
        ALOGE("Script haven't set properties correctlly.");
        return -1;
    }

    snprintf(prop_name, sizeof(prop_name), "%s.%s.gateway", DHCP_PROP_NAME_PREFIX, p2p_interface);
    property_get(prop_name, gateway, NULL);

    snprintf(prop_name, sizeof(prop_name), "%s.%s.server", DHCP_PROP_NAME_PREFIX, p2p_interface);
    property_get(prop_name, server, NULL);

    
    if (gateway[0] == '\0' || strncmp(gateway, "0.0.0.0", 7) == 0) {
        
        strncpy(gateway, server, PROPERTY_VALUE_MAX);
    }

    snprintf(prop_name, sizeof(prop_name), "%s.%s.mask", DHCP_PROP_NAME_PREFIX, p2p_interface);
    if (property_get(prop_name, prop_value, NULL)) {
        int p;
        
        in_addr_t mask = ntohl(inet_addr(prop_value));
        
        
        if (mask == INADDR_NONE && strcmp(prop_value, "255.255.255.255") != 0) {
            snprintf(errmsg, sizeof(errmsg), "DHCP gave invalid net mask %s", prop_value);
            return -1;
        }
        for (p = 0; p < 32; p++) {
            if (mask == 0) break;
            
            if ((mask & 0x80000000) == 0) {
                snprintf(errmsg, sizeof(errmsg), "DHCP gave invalid net mask %s", prop_value);
                return -1;
            }
            mask = mask << 1;
        }
        *prefixLength = p;
    }

    for (x=0; dns[x] != NULL; x++) {
        snprintf(prop_name, sizeof(prop_name), "%s.%s.dns%d", DHCP_PROP_NAME_PREFIX, p2p_interface, x+1);
        property_get(prop_name, dns[x], NULL);
    }

    snprintf(prop_name, sizeof(prop_name), "%s.%s.leasetime", DHCP_PROP_NAME_PREFIX, p2p_interface);
    if (property_get(prop_name, prop_value, NULL)) {
        *lease = atol(prop_value);
    }

    snprintf(prop_name, sizeof(prop_name), "%s.%s.vendorInfo", DHCP_PROP_NAME_PREFIX,
            p2p_interface);
    property_get(prop_name, vendorInfo, NULL);

    snprintf(prop_name, sizeof(prop_name), "%s.%s.domain", DHCP_PROP_NAME_PREFIX,
            p2p_interface);
    property_get(prop_name, domain, NULL);

    snprintf(prop_name, sizeof(prop_name), "%s.%s.mtu", DHCP_PROP_NAME_PREFIX,
            p2p_interface);
    property_get(prop_name, mtu, NULL);

    return 0;
}

int dhcp_get_results(const char *interface,
                     char *ipaddr,
                     char *gateway,
                     uint32_t *prefixLength,
                     char *dns[],
                     char *server,
                     uint32_t *lease,
                     char *vendorInfo,
                     char *domain,
                     char *mtu)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX];

    
    char p2p_interface[MAX_INTERFACE_LENGTH];
    get_p2p_interface_replacement(interface, p2p_interface);
    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            DHCP_PROP_NAME_PREFIX,
            p2p_interface);

    memset(prop_value, '\0', PROPERTY_VALUE_MAX);
    if (!property_get(result_prop_name, prop_value, NULL)) {
        snprintf(errmsg, sizeof(errmsg), "%s", "DHCP result property was not set");
        return -1;
    }
    if (strcmp(prop_value, "ok") == 0) {
        if (fill_ip_info(interface, ipaddr, gateway, prefixLength, dns,
                server, lease, vendorInfo, domain, mtu) == -1) {
            return -1;
        }
        return 0;
    } else {
        snprintf(errmsg, sizeof(errmsg), "DHCP result was %s", prop_value);
        property_set(result_prop_name, "timeout");
        return -1;
    }
}

int dhcp_start(const char *interface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    char daemon_cmd[PROPERTY_VALUE_MAX * 2 + sizeof(DHCP_CONFIG_PATH)];
    const char *ctrl_prop = "ctl.start";
    const char *desired_status = "running";
    
    char p2p_interface[MAX_INTERFACE_LENGTH];

    get_p2p_interface_replacement(interface, p2p_interface);

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            DHCP_PROP_NAME_PREFIX,
            p2p_interface);

    snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
            DAEMON_PROP_NAME,
            p2p_interface);

    
#if 0
    property_set(result_prop_name, "");
#endif
    property_set(result_prop_name, "obtaining");

    
#if 0
    if (property_get(HOSTNAME_PROP_NAME, prop_value, NULL) && (prop_value[0] != '\0'))
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s:-f %s -h %s %s", DAEMON_NAME,
                 p2p_interface, DHCP_CONFIG_PATH, prop_value, interface);
    else
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s:-f %s %s", DAEMON_NAME,
                 p2p_interface, DHCP_CONFIG_PATH, interface);
#endif
    if (property_get(HOSTNAME_PROP_NAME, prop_value, NULL) && (prop_value[0] != '\0')) {
#ifdef MTK_AUTOIP_SUPPORT
        ALOGD(" AUTOIP is SUPPORTed \n");
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s:-f %s -h %s %s", DAEMON_NAME,
                 p2p_interface, DHCP_CONFIG_PATH, prop_value, interface);
#else
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s:-A -f %s -h %s %s", DAEMON_NAME,
                 p2p_interface,DHCP_CONFIG_PATH, prop_value, interface);
#endif
    }
    ALOGI("dhcp_do_request: %s", daemon_cmd);
    memset(prop_value, '\0', PROPERTY_VALUE_MAX);
    property_set(ctrl_prop, daemon_cmd);
    if (wait_for_property(daemon_prop_name, desired_status, 10) < 0) {
        snprintf(errmsg, sizeof(errmsg), "%s", "Timed out waiting for dhcpcd to start");
        return -1;
    }

    
#ifndef MTK_AUTOIP_SUPPORT
    if (wait_for_property(result_prop_name, NULL, 30) < 0) {
#else
    if (wait_for_property(result_prop_name, NULL, 40) < 0) {
#endif
        snprintf(errmsg, sizeof(errmsg), "%s", "Timed out waiting for DHCP to finish");
        return -1;
    }

    return 0;
}

int dhcp_stop(const char *interface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};

    const char *ctrl_prop = "ctl.stop";
    const char *desired_status = "stopped";

    char p2p_interface[MAX_INTERFACE_LENGTH];

    get_p2p_interface_replacement(interface, p2p_interface);

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            DHCP_PROP_NAME_PREFIX,
            p2p_interface);

    snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
            DAEMON_PROP_NAME,
            p2p_interface);

    snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s", DAEMON_NAME, p2p_interface);

    
    ALOGI("dhcp_stop.");
    
    if (strcmp(prop_value, "obtaining") == 0)
        wait_for_property(daemon_prop_name, "running", 1);

    property_set(ctrl_prop, daemon_cmd);
    if (wait_for_property(daemon_prop_name, desired_status, 5) < 0) {
        return -1;
    }
    property_set(result_prop_name, "failed");
    return 0;
}

int dhcp_release_lease(const char *interface)
{
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.stop";
    const char *desired_status = "stopped";

    char p2p_interface[MAX_INTERFACE_LENGTH];

    get_p2p_interface_replacement(interface, p2p_interface);

    snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
            DAEMON_PROP_NAME,
            p2p_interface);

    snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s", DAEMON_NAME, p2p_interface);

    
    property_set(ctrl_prop, daemon_cmd);
    if (wait_for_property(daemon_prop_name, desired_status, 5) < 0) {
        return -1;
    }
    return 0;
}

char *dhcp_get_errmsg() {
    return errmsg;
}

int dhcp_start_renew(const char *interface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.start";

    char p2p_interface[MAX_INTERFACE_LENGTH];

    get_p2p_interface_replacement(interface, p2p_interface);

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            DHCP_PROP_NAME_PREFIX,
            p2p_interface);

    
    property_set(result_prop_name, "");

    
    snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s:%s", DAEMON_NAME_RENEW,
            p2p_interface, interface);
    memset(prop_value, '\0', PROPERTY_VALUE_MAX);
    property_set(ctrl_prop, daemon_cmd);

    
    if (wait_for_property(result_prop_name, NULL, 30) < 0) {
        snprintf(errmsg, sizeof(errmsg), "%s", "Timed out waiting for DHCP Renew to finish");
        return -1;
    }

    return 0;
}

static int wait_for_property_usedByPD(const char *name, const char *desired_value, int maxwait)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int maxnaps = (maxwait * 1000) / NAP_TIME;

    if (maxnaps < 1) {
        maxnaps = 1;
    }

    while (maxnaps-- > 0) {
        usleep(NAP_TIME * 1000);
        if (property_get(name, value, NULL)) {
            if (desired_value == NULL || 
                    strcmp(value, desired_value) == 0) {
                return 0;
            }
        }
    }
    return -1; 
}

enum GET_RA_RET {ERR=0, O_SET=1, M_SET=2, DEF_VAL=4};
enum GET_RA_RET ra_flag;

static enum GET_RA_RET getMbitFromRA(const char * iface, int maxwait)
{
	char ch;
    char filename[64];
	snprintf(filename, sizeof(filename), "/proc/sys/net/ipv6/conf/%s/ra_info_flag", iface);

	if (maxwait < 1)
	{
		maxwait = 1;
	}

	while (maxwait-- > 0)
	{
        usleep(1000*1000); 
		int fd = open(filename, O_RDONLY);

		if (fd < 0) {
			ALOGE("Can't open %s: %s", filename, strerror(errno));
			
			continue;
		}

		int len = read(fd, &ch, 1);
		close(fd);

		if (len < 0) {
			ALOGE("Can't read %s: %s", filename, strerror(errno));
			continue;
		}

		ALOGD("read:ra_info_flag=%c\n", ch);
		if (ch == '2') 
		{
			return M_SET;
		}
		else if (ch == '1')
		{
			return O_SET;
		}
		else if (ch == '4')
		{
			return DEF_VAL;
		}
	}

	return ERR;
}


static int fill_ip6_info(const char *interface,
		char *ipaddr,
		char *dns1,
		char *dns2,
		uint32_t *lease)
{
    char prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX];

    snprintf(prop_name, sizeof(prop_name), "%s.%s.ipaddress", DHCPv6_PROP_NAME_PREFIX, interface);
    property_get(prop_name, ipaddr, NULL);

    snprintf(prop_name, sizeof(prop_name), "%s.%s.dns1", DHCPv6_PROP_NAME_PREFIX, interface);
    property_get(prop_name, dns1, NULL);

    snprintf(prop_name, sizeof(prop_name), "%s.%s.dns2", DHCPv6_PROP_NAME_PREFIX, interface);
    property_get(prop_name, dns2, NULL);

    snprintf(prop_name, sizeof(prop_name), "%s.%s.leasetime", DHCPv6_PROP_NAME_PREFIX, interface);
    if (property_get(prop_name, prop_value, NULL))
		*lease = atol(prop_value);
    else 
		*lease = 0x7FFFFFFF - 1;
    ALOGD("(int)leasetime=%d\n", *lease);
	
    return 0;
}

static void clear_ip6_info(const char *interface)
{
    char prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX];

    ALOGD("[clear_ip6_info]");
    snprintf(prop_name, sizeof(prop_name), "%s.%s.ipaddress", DHCPv6_PROP_NAME_PREFIX, interface);
    property_set(prop_name, "");

    snprintf(prop_name, sizeof(prop_name), "%s.%s.dns1", DHCPv6_PROP_NAME_PREFIX, interface);
    property_set(prop_name, "");

    snprintf(prop_name, sizeof(prop_name), "%s.%s.dns2", DHCPv6_PROP_NAME_PREFIX, interface);
    property_set(prop_name, "");

    snprintf(prop_name, sizeof(prop_name), "%s.%s.leasetime", DHCPv6_PROP_NAME_PREFIX, interface);
    property_set(prop_name, "");
}

static int clear_RAflag(const char *interface)
{
    char proc[64];
    snprintf(proc, sizeof(proc), "/proc/sys/net/ipv6/conf/%s/ra_info_flag", interface);

    int fd = open(proc, O_WRONLY);
    if (fd < 0) {
        ALOGE("Failed to open ra_info_flag (%s)", strerror(errno));
        return -1;
    }

    if (write(fd, "0", 1) != 1) {
        ALOGE("Failed to write ra_info_flag (%s)", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;		
}

int dhcpv6_start(const char *interface, uint32_t *pid_ptr)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.start";
    const char *desired_status = "running";
    FILE * fp;

    
    clear_ip6_info(interface);

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            DHCPv6_PROP_NAME_PREFIX,
            interface);


    
    property_set(result_prop_name, "");

    
    ra_flag = getMbitFromRA("wlan0", 10);

    if (ra_flag == M_SET) {
        snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
				DHCPv6_DAEMON_PROP_NAME,
				interface);
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s", DHCPv6_DAEMON_NAME, interface);
    } else if (ra_flag == O_SET) {
        snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
				DHCPv6DNS_DAEMON_PROP_NAME,
				interface);
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s", DHCPv6DNS_DAEMON_NAME, interface);
    } else {
        ALOGD("AP didn't support ipv6");
        return -1;
    }

    memset(prop_value, '\0', PROPERTY_VALUE_MAX);
    property_set(ctrl_prop, daemon_cmd);
    if (wait_for_property(daemon_prop_name, desired_status, 10) < 0) {
        snprintf(errmsgv6, sizeof(errmsgv6), "%s", "Timed out waiting for dhcp6c to start");

        return -1;
    }

    
    if (wait_for_property(result_prop_name, NULL, 30) < 0) {
        snprintf(errmsgv6, sizeof(errmsgv6), "%s", "Timed out waiting for DHCPv6 to finish");
        return -1;
    }
    
    if ((fp = fopen(DHCP6C_PIDFILE, "r")) == NULL) {
        ALOGE("%s", "Failed to open pid file.");
        return -1;
    }

    if (fscanf(fp, "%d", pid_ptr) != 1)
        *pid_ptr = 0;
    fclose(fp);

    if (*pid_ptr <= 0) {
        ALOGE("pid value is invalid. pid=%d", *pid_ptr);
        return -1;
    }
    return 0;
}

int dhcpv6_stop(const char *interface)
{
#if 1
    char result_prop_name[PROPERTY_KEY_MAX];
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.stop";
    const char *desired_status = "stopped";

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            DHCPv6_PROP_NAME_PREFIX,
            interface);

    if (ra_flag == M_SET) {
        snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
				DHCPv6_DAEMON_PROP_NAME,
				interface);
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s", DHCPv6_DAEMON_NAME, interface);
    } else if (ra_flag == O_SET) {
        snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
				DHCPv6DNS_DAEMON_PROP_NAME,
				interface);
        snprintf(daemon_cmd, sizeof(daemon_cmd), "%s_%s", DHCPv6DNS_DAEMON_NAME, interface);
    } else {
        return 0;
    }

    ALOGE("[dhcpv6_stop] ctrl_prop = %s; daemon_cmd = %s; result_prop_name = %s", ctrl_prop,
            daemon_cmd, result_prop_name);

    
    property_set(ctrl_prop, daemon_cmd);
    if (wait_for_property(daemon_prop_name, desired_status, 5) < 0) {
        return -1;
    }

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(daemon_prop_name, value, NULL);
    ALOGE("[dhcpv6_stop] value = %s", value);

    property_set(result_prop_name, "released");

    
    clear_ip6_info(interface);
    clear_RAflag("wlan0");
    return 0;
#else
    char result_prop_name[PROPERTY_KEY_MAX];
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *desired_status = "stopped";

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            DHCPv6_PROP_NAME_PREFIX,
            interface);

    snprintf(daemon_prop_name, sizeof(daemon_prop_name), "%s_%s",
            DHCPv6_DAEMON_PROP_NAME,
            interface);


    
	system("dhcp6ctl stop");

    if (wait_for_property(daemon_prop_name, desired_status, 5) < 0) {
        return -1;
    }
    property_set(result_prop_name, "released");

    
    clear_ip6_info(interface);
	
    return 0;
#endif
}

char *dhcpv6_get_errmsg() {
    return errmsgv6;
}

char *PD_get_errmsg() {
    return errmsgPD;
}


int dhcp_do_sip_request(const char *iface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.start";

    ALOGD("dhcp_do_sip_request for %s\n", iface);
    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.v4sipinfo",
            DHCP_PROP_NAME_PREFIX,iface);
    
    property_set(result_prop_name, "");

    
    snprintf(daemon_cmd, sizeof(daemon_cmd), "%s:%s", 
    		DAEMON_NAME_INFORM, iface);
    memset(prop_value, '\0', PROPERTY_VALUE_MAX);
    property_set(ctrl_prop, daemon_cmd);

    if (wait_for_property(result_prop_name, NULL, 30) < 0) {
        snprintf(errmsg, sizeof(errmsg), "%s", "Timed out waiting for DHCP inform to finish");
        return -1;
    }

    if (!property_get(result_prop_name, prop_value, NULL)) {
        
        snprintf(errmsg, sizeof(errmsg), "%s", "DHCP Inform result property was not set");
        return -1;
    }
    if (strcmp(prop_value, "success") == 0) {
        return 0;
    } else {
        snprintf(errmsg, sizeof(errmsg), "DHCP Inform result was %s", prop_value);
        return -1;
    }
}


int dhcp_stop_sip_request(const char *iface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.stop";
    const char *desired_status = "stopped";

    char p2p_interface[MAX_INTERFACE_LENGTH];
	
    ALOGD("dhcp_stop_sip_request for %s\n", iface);
    if (NULL == iface) {
        snprintf(errmsg, sizeof(errmsg), "%s", "interface is NULL.");
        return -1;
    }

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.v4sipinfo",
            DHCP_PROP_NAME_PREFIX,iface);

    snprintf(daemon_cmd, sizeof(daemon_cmd), "%s", DAEMON_NAME_INFORM);
    
    property_set(ctrl_prop, daemon_cmd);
	snprintf(daemon_prop_name, sizeof(daemon_prop_name), "init.svc.%s", 
		DAEMON_NAME_INFORM);
    if (wait_for_property(daemon_prop_name, desired_status, 5) < 0) {
        return -1;
    }
    property_set(result_prop_name, "failed");
    return 0;
}

int dhcpv6_do_sip_request(const char *iface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.start";
	
    ALOGD("dhcpv6_do_sip_request for %s\n", iface);
    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.v6sipinfo",
            DHCPv6_PROP_NAME_PREFIX,iface);
    
    property_set(result_prop_name, "");

    
    snprintf(daemon_cmd, sizeof(daemon_cmd), "%s:%s", 
    		DAEMON_NAME_INFORMV6, iface);
    memset(prop_value, '\0', PROPERTY_VALUE_MAX);
    property_set(ctrl_prop, daemon_cmd);

    if (wait_for_property(result_prop_name, NULL, 30) < 0) {
		 snprintf(errmsgv6, sizeof(errmsgv6), "%s", "Timed out waiting for DHCPv6 inform to finish");
		 return -1;
    }

    if (!property_get(result_prop_name, prop_value, NULL)) {
        
        snprintf(errmsgv6, sizeof(errmsgv6), "%s", "DHCPv6 Inform result property was not set");
        return -1;
    }
    if (strcmp(prop_value, "success") == 0) {
        return 0;
    } else {
        snprintf(errmsgv6, sizeof(errmsgv6), "DHCPv6 Inform result was %s", prop_value);
        return -1;
    }
}


int dhcpv6_stop_sip_request(const char *iface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char daemon_prop_name[PROPERTY_KEY_MAX];
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.stop";
    const char *desired_status = "stopped";

    char p2p_interface[MAX_INTERFACE_LENGTH];
	
    ALOGD("dhcpv6_stop_sip_request for %s\n", iface);
    if (NULL == iface) {
		snprintf(errmsgv6, sizeof(errmsgv6), "%s", "interface is NULL.");
		return -1;
    }

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.v6sipinfo",
            DHCPv6_PROP_NAME_PREFIX,iface);

    snprintf(daemon_cmd, sizeof(daemon_cmd), "%s", DAEMON_NAME_INFORMV6);
    
    property_set(ctrl_prop, daemon_cmd);
	snprintf(daemon_prop_name, sizeof(daemon_prop_name), "init.svc.%s", 
		DAEMON_NAME_INFORMV6);
    if (wait_for_property(daemon_prop_name, desired_status, 5) < 0) {
        return -1;
    }
    property_set(result_prop_name, "failed");
    return 0;
}

int dhcpv6_start_renew(const char *interface, const int pid)
{
    int ret;
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.renewresult",
            DHCPv6_PROP_NAME_PREFIX,
            interface);

    ALOGE("[dhcpv6_start_renew] result_prop_name = %s; kill(%d, %d)", result_prop_name, pid, SIGUSR1);

    
	
	
        
		
	

	kill(pid, SIGUSR1);

	
	if (wait_for_property(result_prop_name, NULL, 30) < 0) {
            char value[PROPERTY_VALUE_MAX] = {'\0'};
            property_get(result_prop_name, value, NULL);
            ALOGE("[dhcpv6_start_renew] wait_for_property() value = %s; return < 0", value);
		snprintf(errmsgv6, sizeof(errmsgv6), "%s", "Timed out waiting for DHCPv6 renew to finish");
		return -1;
	}
        ALOGE("[dhcpv6_start_renew] wreturn 0");
    return 0;
}

#if 1
static int fill_PD_info(const char *interface,
		char *prefix,
		uint32_t *lease)
{
    char prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX];

	snprintf(prop_name, PROPERTY_KEY_MAX, "net.pd.%s.prefix", interface);
    property_get(prop_name, prefix, NULL);
	if (strlen(prefix) == 0)
	{
		ALOGD("pd.prefix is null string.");
		return -1;
	}
	else
	{
		ALOGD("pd.prefix=%s", prefix);
	}

	snprintf(prop_name, sizeof(prop_name), "%s.%s.leasetime", DHCPv6_PROP_NAME_PREFIX, interface);
    if (property_get(prop_name, prop_value, NULL))
	{
		*lease = atol(prop_value);
	}
	else 
	{
		*lease = 0x7FFFFFFF - 1;
	}

	ALOGD("(int)leasetime=%d\n", *lease);
	
	return 0;
}

void clear_pd_info(const char *interface)
{
	char prop_name[PROPERTY_KEY_MAX];
	char prefix[PROPERTY_VALUE_MAX];

	char plen[PROPERTY_VALUE_MAX];

	snprintf(prop_name, PROPERTY_KEY_MAX, "net.pd.%s.prefix", interface);
    if (property_get(prop_name, prefix, NULL) > 0)
	{
		property_set("net.pd.lastprefix", prefix);
	}
		
	property_set(prop_name, "");

	snprintf(prop_name, PROPERTY_KEY_MAX, "net.pd.%s.plen", interface);
    if (property_get(prop_name, plen, NULL) > 0)
	{
		property_set("net.pd.lastplen", plen);
	}
	property_set(prop_name, "");
}

int dhcpv6_PD_request(const char *interface, char *prefix, uint32_t *lease)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    char daemon_cmd[PROPERTY_VALUE_MAX * 2];
    const char *ctrl_prop = "ctl.start";
    const char *desired_status = "running";

	if (interface == NULL)
	{
        snprintf(errmsgPD, sizeof(errmsgPD), "%s", "PD interface is NULL");

		return -1;
	}

	
	clear_pd_info(interface);

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            PD_PROP_NAME_PREFIX,
            interface);


    
    property_set(result_prop_name, "");

	snprintf(daemon_cmd, sizeof(daemon_cmd), "%s:%s", PD_DAEMON_NAME, interface);

    memset(prop_value, '\0', PROPERTY_VALUE_MAX);

	ALOGD("Got stop_pd command when start PD request.");
	snprintf(errmsgPD, sizeof(errmsgPD), "%s", "Got stop_pd command when start PD request.");

	return -1;

	property_set(ctrl_prop, daemon_cmd);

	ALOGD("%s", daemon_cmd);
    if (wait_for_property_usedByPD(PD_DAEMON_PROP_NAME, desired_status, 10) < 0) {
		ALOGD("PD_Request: start dhcp6c_Pd service fail.");
        snprintf(errmsgPD, sizeof(errmsgPD), "%s", "Timed out waiting for dhcp6c to start");

        return -1;
    }

    
    if (wait_for_property_usedByPD(result_prop_name, NULL, 30) < 0) {
        snprintf(errmsgPD, sizeof(errmsgPD), "%s", "Timed out waiting for DHCPv6PD to finish");
        return -1;
    }

    if (!property_get(result_prop_name, prop_value, NULL)) {
        
        snprintf(errmsgPD, sizeof(errmsgPD), "%s", "DHCPv6 result property was not set");
        return -1;
    }
	
	
	fill_PD_info(interface, prefix, lease);

    if (strcmp(prop_value, "ok") == 0) {
		ALOGD("PD all ok.");
        return 0;
    } else {
        snprintf(errmsgPD, sizeof(errmsgPD), "DHCPv6 result was %s", prop_value);
        return -1;
    }
}

int dhcpv6_PD_renew(const char *interface, char *prefix, uint32_t *lease)
{
	int ret;
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.renewresult",
            PD_PROP_NAME_PREFIX,
            interface);

    

	if ((ret = system("dhcp6ctl renew")) < 0)
	{
		snprintf(errmsgPD, sizeof(errmsgPD), "%s:%d\n", "execute renew command, got error return code:", ret);
		return -1;
	}

	
	if (wait_for_property_usedByPD(result_prop_name, NULL, 30) < 0) {
		snprintf(errmsgPD, sizeof(errmsgPD), "%s", "Timed out waiting for PD renew to finish");
		return -1;
	}

	if (!property_get(result_prop_name, prop_value, NULL)) {
		
		snprintf(errmsgPD, sizeof(errmsgPD), "%s", "PD renew result property was not set");
		return -1;
	}
	if (strcmp(prop_value, "ok") == 0) {
		
		property_set(result_prop_name, "");

		fill_PD_info(interface, prefix, lease);
		return 0;
	} else {
		snprintf(errmsgPD, sizeof(errmsgPD), "PD renew result was %s", prop_value);
		return -1;
	}
}

int dhcpv6_PD_stop(const char *interface)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    const char *ctrl_prop = "ctl.stop";
    const char *desired_status = "stopped";

	ALOGD("dhcpv6_PD_STOP.");
	if (interface == NULL)
	{
		snprintf(errmsgPD, sizeof(errmsgPD), "PD interface is NULL");
		return -1;
	}

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            PD_PROP_NAME_PREFIX,
            interface);


    
    property_set(ctrl_prop, PD_DAEMON_NAME);
	
    if (wait_for_property(PD_DAEMON_PROP_NAME, desired_status, 5) < 0) {
        snprintf(errmsgPD, sizeof(errmsgPD), "%s", "Timed out waiting for dhcpv6PD to stop");
        return -1;
    }
    property_set(result_prop_name, "released");
	clear_pd_info(interface);

    return 0;
}

int dhcpv6_get_results(const char *interface, char *ipaddr,
        char *dns1, char *dns2, uint32_t *lease, int isrenew)
{
    char result_prop_name[PROPERTY_KEY_MAX];
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    FILE * fp;

    if (isrenew)
        snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.renewresult",
                 DHCPv6_PROP_NAME_PREFIX, interface);
    else
        snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
                 DHCPv6_PROP_NAME_PREFIX, interface);

    ALOGD("[dhcpv6_get_results] result_prop_name = %s; isrenew = %d", result_prop_name, isrenew);
    if (!property_get(result_prop_name, prop_value, NULL)) {
        
        snprintf(errmsgv6, sizeof(errmsgv6), "%s", "DHCPv6 result property was not set");
        ALOGD("[dhcpv6_get_results] prop_value = %s", prop_value);
        return -1;
    }
    ALOGD("[dhcpv6_get_results] prop_value = %s", prop_value);
    if (strcmp(prop_value, "ok") == 0) {
        if (isrenew) {
            
            property_set(result_prop_name, "");
        }
        
        fill_ip6_info(interface, ipaddr, dns1, dns2, lease);
        return 0;
    } else {
        snprintf(errmsgv6, sizeof(errmsgv6), "DHCPv6 result was %s", prop_value);
        return -1;
    }
}
#endif
