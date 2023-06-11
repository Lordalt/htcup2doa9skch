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

#ifndef _WIFI_DHCP_H_
#define _WIFI_DHCP_H_

#include <sys/types.h>

#define PORT_BOOTP_SERVER 67
#define PORT_BOOTP_CLIENT 68

typedef struct dhcp_msg dhcp_msg;

#define OP_BOOTREQUEST 1
#define OP_BOOTREPLY   2

#define FLAGS_BROADCAST 0x8000

#define HTYPE_ETHER    1

struct dhcp_msg
{
    uint8_t op;           
    uint8_t htype;        
    uint8_t hlen;         
    uint8_t hops;         
    
    uint32_t xid;         

    uint16_t secs;        
    uint16_t flags;

    uint32_t ciaddr;      
    uint32_t yiaddr;      
    uint32_t siaddr;      
                          
    uint32_t giaddr;      

    uint8_t chaddr[16];  
    char sname[64];      
    char file[128];      

    uint8_t options[1024];  
};

#define DHCP_MSG_FIXED_SIZE 236

#define OPT_COOKIE1          0x63
#define OPT_COOKIE2          0x82
#define OPT_COOKIE3          0x53
#define OPT_COOKIE4          0x63

#define OPT_PAD              0

#define OPT_SUBNET_MASK      1     
#define OPT_TIME_OFFSET      2     
#define OPT_GATEWAY          3     
#define OPT_DNS              6     
#define OPT_DOMAIN_NAME      15    
#define OPT_BROADCAST_ADDR   28    

#define OPT_REQUESTED_IP     50    
#define OPT_LEASE_TIME       51    
#define OPT_MESSAGE_TYPE     53    
#define OPT_SERVER_ID        54    
#define OPT_PARAMETER_LIST   55    
#define OPT_MESSAGE          56    
#define OPT_CLASS_ID         60    
#define OPT_CLIENT_ID        61    
#define OPT_END              255

#define DHCPDISCOVER         1
#define DHCPOFFER            2
#define DHCPREQUEST          3
#define DHCPDECLINE          4
#define DHCPACK              5
#define DHCPNAK              6
#define DHCPRELEASE          7
#define DHCPINFORM           8

int init_dhcp_discover_msg(dhcp_msg *msg, void *hwaddr, uint32_t xid);

int init_dhcp_request_msg(dhcp_msg *msg, void *hwaddr, uint32_t xid,
                          uint32_t ipaddr, uint32_t serveraddr);

#endif
