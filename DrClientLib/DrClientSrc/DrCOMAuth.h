/*
 * File         : DrCOMAuth.h
 * Date         : 2011-07-12
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOM auth include
 */

#ifndef _INC_DRCOMAUTH_
#define _INC_DRCOMAUTH_


#include "DrCOMSocket.h"
#include "IDrCOMAuth.h"

class DrCOMAuth : public IDrCOMAuth
{
public:
	DrCOMAuth();
	~DrCOMAuth();

public:
	virtual int httpLogin(const string& strGatewayAddress, const string& strUsername, const string& strPassword);
	virtual int httpLogout();
	virtual int httpStop();
#ifdef _WIN32_WP8
	virtual void SetMac(const string& mac);
#endif
	virtual void SetSSID(const string& ssid);

	virtual string getUndefineError();
	virtual string getXip();
	virtual string getMac();
	virtual string getLocalMac();

	virtual void SetGatewayAddress(string strGatewayAddr);
	virtual string getGatewayAddress();
	virtual bool getLoginStatus();

	virtual bool httpStatus();
	virtual string getFluxStatus();
	virtual string getTimeStatus();

// 初始化/清除 socket环境
public:
	static bool InitSocketEnvironment();
	static void ReleaseSocketEnvironment();

private:
	int httpLoginCheck();
	int httpLoginAuth();
	int httpHandle(tcpSocket* ptSocket, int& iHttpCode, int& iContentLen, string& strHttpServerName, string& strHttpReLocal, char* pHttpBody);
	char* strIstr(const char *haystack, const char *needle);
	string findStringBetween(char* pData, char* pBegin, char* pEnd, char* pTmpBuffer = NULL, int iTmpLen = 0);
	string grantMD5(string strData);
	string trim(string strData);
	int doWithLoginResult(char* pData);
	int loginStatus(string strMsg, string strMsga, string strXip, string strMac, string strTime, string strFlow, string strCode);
	string grantUpdateRequest();
	string GetSecondGrant(const string& strGrant1);
#ifdef WIN32
	static DWORD WINAPI requestUpdate(void* pData);
#else
	static void* requestUpdate(void* pData);
#endif
	bool IsPage0(int iHttpCode, string strHttpServerName);
	int Page0Process(int iHttpCode, int iContentLen, string strHttpServerName, string strHttpReLocal);

protected:
	string m_strGatewayAddress;
	string m_strUsername;
	string m_strPassword;

	string m_strUndefineError;
	string m_strXip;
	string m_strMac;
	string m_strSSID;

	string m_strFlux;
	string m_strTime;
	
	bool m_bConnected;

	string m_strMacList;

	char m_cBuffer[DrCOM_BUFFER_64K];

	tcpSocket	m_socket;
	sslSocket	m_sslSocket;
};

#endif
