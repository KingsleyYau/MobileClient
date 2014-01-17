/*
 * File         : DrClientInclude.h
 * Date         : 2013-05-02
 * Author       : FGX
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : ϵͳ��صĹ�����������
 */

#pragma once

// �������
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
	#include <windows.h>
	#include <time.h>
	#include <Winsock2.h>

	#define localtime_r(stm, time)	_localtime64_s(time, stm)
	#define snprintf				_snprintf
	#define SHUT_RDWR				SD_BOTH
	#define close					closesocket
	#define ftime           _ftime
	#define timeb           _timeb
	#define GMTIMEFUNC(time, tm) _gmtime64_s(tm, time)
#else
    #define GMTIMEFUNC(time, tm) (gmtime_r(time, tm)==0)
#endif
