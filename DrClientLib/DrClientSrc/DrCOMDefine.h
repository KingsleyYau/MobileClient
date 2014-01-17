/*
 * File         : DrCOMDefine.h
 * Date         : 2011-07-11
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOM define
 */

#ifndef _INC_DRCOMDEFINE_
#define _INC_DRCOMDEFINE_

#define NTP_SEND_TIMEOUT		3000
#define NTP_RECV_TIMEOUT		3000

#define HTTP_SEND_TIMEOUT		3000
#define HTTP_RECV_TIMEOUT		1000
#define HTTP_PROT				80
#define HTTPS_PROT				443

#define DrCOM_BUFFER_16B		16
#define DrCOM_BUFFER_32B		32
#define DrCOM_BUFFER_256B		256
#define DrCOM_BUFFER_512B		512
#define DrCOM_BUFFER_1K			1024
#define DrCOM_BUFFER_64K		65536
#define DrCOM_MAX_INFERFACE		16

#define DrCOM_FAIL				-1
#define DrCOM_SUCCESS			1
#define DrCOM_CHECK				2

#define DrCOM_0_html			"<!--Dr.COMWebLoginID_0.htm-->"
#define DrCOM_1_html			"<!--Dr.COMWebLoginID_1.htm-->"
#define DrCOM_2_html			"<!--Dr.COMWebLoginID_2.htm-->"
#define DrCOM_3_html			"<!--Dr.COMWebLoginID_3.htm-->"
#define DrCOM_Server			"DrcomServer1.0"
#define DrCOM_Server_IIS		"DRCOM-IIS-2.00"
#define DrCOM_Logout			"/F.htm"
#define DrCOM_Login				"DDDDD=%s&upass=%s&%s&0MKKey=%s&ssid=%s&ver=%s"
#define DrCOM_TestUrl			"www.baidu.com"
#define DrCOM_Def_Num			"0123456789"
#define DrCOM_UIP				"va5=1.2.3.4."
#define DrCOMWS					"DrCOMWS"

#if defined(_WIN32_WP8)
	#define DrCOM_Version           "1.0.4.201309161.P.W.A"
#elif defined(_IOS)
	#define DrCOM_Version           "1.0.4.201309161.I.M.A"
#elif defined(_ANDROID)
	#define DrCOM_Version           "1.0.4.201309161.G.L.A"
#endif

#define DrCOM_HTTP_TYPE_0000    "0000"
#define DrCOM_HTTP_TYPE_0003    "0003"
#define DrCOM_HTTP_TYPE_0006    "0006"
#define DrCOM_Hash              "4a6bb2a07eb9472e9e5bcccd0571d52c"//DrCOMWS
#define DrCOM_Update_Svr        "update.doctorcom.com"
#define DrCOM_Update_Parm       "/DRCLIENT/UPDATE?TIME=%s&TYPE=%s&KEY=%s&HASH=%s&VER=%s&RND=%s&CHK=%s&M=%s"

#define DrCOM_POST				"POST %s HTTP/1.1\r\nContent-Type: text/xml\r\nCharset: utf-8\r\nDate: %s\r\nTime: %s\r\nUip: %s\r\nContent-Length: %d\r\nHost: %s\r\nUser-Agent: DrCOM-HttpClient\r\n\r\n%s"
#define DrCOM_GET				"GET %s HTTP/1.1\r\nContent-Type: text/xml\r\nCharset: utf-8\r\nHost: %s\r\nUser-Agent: DrCOM-HttpClient\r\n\r\n"
#define DrCOM_HTTP_LINE_END		"\r\n"
#define DrCOM_HTTP_END			"\r\n\r\n"
#define DrCOM_HTTP_CON_LEN		"CONTENT-LENGTH: "
#define DrCOM_HTTP_SERVER		"SERVER: "
#define DrCOM_HTTP_LOCATION		"LOCATION: HTTP://"
#define DrCOM_HTTP_200			200
#define DrCOM_HTTP_302			302

#define DRCOM_Demo_User			"dev.demo"
#define DRCOM_Demo_Pass			"dev@dr.com"
#define DRCOM_Demo_Adress		"210.21.59.56"
#endif
