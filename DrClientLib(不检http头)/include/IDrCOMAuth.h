/*
 * File         : IDrCOMAuth.h
 * Date         : 2013-05-02
 * Author       : FGX
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOM auth interface
 */

#pragma once

#include <string>
using namespace std;

enum DrClientHandleStatus
{
	DrClientHS_Success = 0,				// Success
	DrClientHS_NetworkError = -101,		// Network connection interruption, check the network configuration please
	DrClientHS_UsingNAT = -102,			// This account does not allow use NAT
	DrClientHS_NotFoundDrCOM = -103,		// Can not find Dr.COM web protocol
	DrClientHS_AlreadyOnline = -104,		// This equipment already online, do not need to log in
	DrClientHS_IpNotAllowLogin = -105,	// The IP does not allow login base Dr.COM web technology
	DrClientHS_AccountNotAllow = -106,	// The account does not allow login base Dr.COM web technology
	DrClientHS_NotAllowChangePwd = -107,	// The account does not allow change password
	DrClientHS_InvalidAccountOrPwd = -108,// Invalid account or password, please login again
	DrClientHS_AccountTieUp = -109,		// This account is tie up, please contact network administration
	DrClientHS_UsingOnAppointedIp = -110,	// This account use on appointed IP address only
	DrClientHS_ChargeOrFluxOver = -111,	// This account charge be overspend or flux over
	DrClientHS_AccountBreakOff = -112,	// This account be break off
	DrClientHS_SystemBufferFull = -113,	// System buffer full
	DrClientHS_TieUpCannotAmend = -114,	// This account is tie up, can not amend
	DrClientHS_NewAndConfirmPwdDiffer = -115,	// The new and the confirm password are differ, can not amend
	DrClientHS_PwdAmendSuccessed = -116,	// The password amend successed
	DrClientHS_UsingAppointedMac = -117,	// This account use on appointed Mac address only
	DrClientHS_UndefineError = -130,		// Undefine error
};

class IDrCOMAuth
{
public:
	static IDrCOMAuth* CreateDrCOMAuth();
	static void ReleaseDrCOMAuth(IDrCOMAuth* object);

public:
	virtual int httpLogin(const string& strGatewayAddress, const string& strUsername, const string& strPassword) = 0;
	virtual int httpLogout() = 0;
	virtual int httpStop() = 0;
#ifdef _WIN32_WP8
	virtual void SetMac(const string& mac) = 0;
#endif
	virtual void SetSSID(const string& ssid) = 0;
	virtual string getUndefineError() = 0;
	virtual string getXip() = 0;
	virtual string getMac() = 0;
	virtual string getLocalMac() = 0;
	virtual void SetGatewayAddress(string strGatewayAddr) = 0;
	virtual string getGatewayAddress() = 0;
	
	virtual bool getLoginStatus() = 0;
	virtual bool httpStatus() = 0;
	virtual string getFluxStatus() = 0;
	virtual string getTimeStatus() = 0;

// 初始化/清除 socket环境
public:
	static bool InitSocketEnvironment();
	static void ReleaseSocket();
};