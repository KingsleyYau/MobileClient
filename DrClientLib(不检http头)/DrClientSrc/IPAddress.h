/*
 *  IPAddress.h
 *  DrCOMClientWS
 *
 *  Created by Keqin Su on 11-5-6.
 *  Copyright 2011 City Hotspot Co., Ltd. All rights reserved.
 *
 */

#ifndef _IPADDRESS_INC__
#define _IPADDRESS_INC__
#if !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WIN32_WP8)

#define MAXADDRS    32

extern char *if_names[MAXADDRS];
extern char *ip_names[MAXADDRS];
extern char *hw_addrs[MAXADDRS];

// Function prototypes

void InitAddresses();
void FreeAddresses();
int GetIPAddresses();
int GetHWAddresses();
#endif	// !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WIN32_WP8)
#endif	// _IPADDRESS_INC__