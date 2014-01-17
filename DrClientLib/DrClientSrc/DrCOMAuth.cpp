/*
 * File         : DrCOMAuth.cpp
 * Date         : 2011-07-12
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOM auth source
 */

#include <ctype.h>
#include "DrClientInclude.h"
#include "Arithmetic.h"
#include "DrCOMAuth.h"
#include "polarssl/md5.h"
#include "NetTime.h"

#define DRCOMKEY	"drcomd007"
// const long DRCOMKEY[] = {0x6f637264, 0x3030646d, 0x00000037};

#ifdef _WIN32_WP8
#define MACSIGNED		"m1="
#endif

DrCOMAuth::DrCOMAuth() {
	m_strGatewayAddress = "";
	m_strUsername = "";
	m_strPassword = "";

	m_strUndefineError = "";
	m_strXip = "";
	m_strMac = "";
	m_strSSID = "";
	
	m_strFlux = "";
	m_strTime = "";
	
	m_bConnected = false;

	m_strMacList = m_socket.GetMacAddressList();

	memset(m_cBuffer, 0, sizeof(m_cBuffer));
    srand(time(NULL));
}

DrCOMAuth::~DrCOMAuth() {

}

int DrCOMAuth::httpLogin(const string& strGatewayAddress, const string& strUsername, const string& strPassword) {
	m_strGatewayAddress = strGatewayAddress;
	m_strUsername = strUsername;
	m_strPassword = strPassword;

	bool  bIsLogining = false;
	if (strGatewayAddress.length() > 0) {
		bIsLogining = httpStatus();
	}
	
	int iRet;
	if (0 == strUsername.compare(DRCOM_Demo_User) 
		&& 0 == strPassword.compare(DRCOM_Demo_Pass)) 
	{
		// demo
		m_strGatewayAddress = DRCOM_Demo_Adress;
		iRet = httpLoginAuth();
	}
	else if (bIsLogining == true) {
		iRet = DrCOM_SUCCESS;
	}
	else {
		iRet = httpLoginCheck();
		if (DrCOM_SUCCESS == iRet) {
			iRet = httpLoginAuth();
		}
	}
	return iRet;
}

int DrCOMAuth::httpLoginCheck() {
	int iRet = DrClientHS_NetworkError;
	int iHttpCode = 0, iContentLen = 0;
	string strHttpServerName = "", strHttpReLocal = "";
	m_socket.Close();

	//connect to test url
	memset(m_cBuffer, 0, sizeof(m_cBuffer));
	if (m_socket.Connect(DrCOM_TestUrl) == 1) {
		sprintf(m_cBuffer, DrCOM_GET, "/", DrCOM_TestUrl);
		if (m_socket.SendData(m_cBuffer, strlen(m_cBuffer)) == strlen(m_cBuffer)) {
			memset(m_cBuffer, 0, sizeof(m_cBuffer));
			if (httpHandle(&m_socket, iHttpCode, iContentLen, strHttpServerName, strHttpReLocal, (char*)m_cBuffer) == 1) {
				if (strHttpReLocal.length() > 0 && iHttpCode == DrCOM_HTTP_302) {
					m_socket.Close();
					//relocal to gateway
					if (m_socket.Connect(strHttpReLocal) == 1) {
						sprintf(m_cBuffer, DrCOM_GET, "/", strHttpReLocal.c_str());
						if (m_socket.SendData(m_cBuffer, strlen(m_cBuffer)) == strlen(m_cBuffer)) {
							memset(m_cBuffer, 0, sizeof(m_cBuffer));
							if (httpHandle(&m_socket, iHttpCode, iContentLen, strHttpServerName, strHttpReLocal, (char*)m_cBuffer) == 1) {
								if (IsPage0(iHttpCode, strHttpServerName)) {
									iRet = Page0Process(iHttpCode, iContentLen, strHttpServerName, strHttpReLocal);
								} else {
									iRet = DrClientHS_NotFoundDrCOM;
								}
							}
						}
					}
				} else if (IsPage0(iHttpCode, strHttpServerName)) {
					iRet = Page0Process(iHttpCode, iContentLen, strHttpServerName, strHttpReLocal);
				} else {
					//// 可以出外网
					iRet = DrClientHS_AlreadyOnline;
				}
			}
		}
	}

	return iRet;
}

int DrCOMAuth::httpStop()
{
	m_socket.Close();
	m_sslSocket.Close();
	return 0;
}

string DrCOMAuth::GetSecondGrant(const string& strGrant1)
{
	string strRet = "";
	unsigned char md[DrCOM_BUFFER_16B] = {'\0'};
	char cTmp[3]={'\0'};

	md5((unsigned char*)strGrant1.c_str(), strGrant1.length(), md);
	
	sprintf(cTmp,"%02x", md[0]);
	strRet += cTmp;
	sprintf(cTmp,"%02x", md[3]);
	strRet += cTmp;
	sprintf(cTmp,"%02x", md[10]);
	strRet += cTmp;
	sprintf(cTmp,"%02x", md[13]);
	strRet += cTmp;

	return strRet;
}

int DrCOMAuth::httpLoginAuth() {
	int iRet = DrClientHS_NotFoundDrCOM;
	char cRep[DrCOM_BUFFER_256B] = {'\0'};
	int iHttpCode = 0, iContentLen = 0;
	unsigned int uiTime = 0;
	string strHttpServerName = "", strHttpReLocal = "";
	string strDate = "", strData = "", strUip = DrCOM_UIP;
	m_sslSocket.Close();

	// 如果获取2166时间失败，则使用本地时间进行认证
	NetTime netTime;
	if (!netTime.GetNetTime(m_strGatewayAddress.c_str(), NTP_PORT, uiTime)) {
		uiTime = time(NULL);
	}

	memset(m_cBuffer, 0, sizeof(m_cBuffer));
	if (m_sslSocket.Connect(m_strGatewayAddress) == 1) {
		//grant time string
		time_t stm = time(NULL);
		struct tm tTime;
		localtime_r(&stm, &tTime);
		memset(cRep, 0, sizeof(cRep));
		snprintf(cRep, sizeof(cRep), "%04d-%02d-%02d %02d:%02d:%02d", tTime.tm_year+1900, tTime.tm_mon+1, tTime.tm_mday, tTime.tm_hour, tTime.tm_min, tTime.tm_sec);
		strDate = cRep;
		//grant data
		snprintf(cRep, sizeof(cRep), DrCOM_Login, m_strUsername.c_str(), m_strPassword.c_str(), m_strMacList.c_str(), DrCOM_Def_Num, m_strSSID.c_str(), DrCOM_Version);
		strData = cRep;
		// time
		char cTime[32] = {0};
		sprintf(cTime, "%d", uiTime);
		//grant uip
		string strGrant = grantMD5(strData + strDate);
		strGrant += GetSecondGrant(strData + cTime + DRCOMKEY);
		strUip += strGrant;
			
		sprintf(m_cBuffer, DrCOM_POST, "/", strDate.c_str(), cTime, strUip.c_str(), strData.length(), m_strGatewayAddress.c_str(), strData.c_str());
		if (m_sslSocket.SendData(m_cBuffer, strlen(m_cBuffer)) == strlen(m_cBuffer)) {
			memset(m_cBuffer, 0, sizeof(m_cBuffer));
			if (httpHandle(&m_sslSocket, iHttpCode, iContentLen, strHttpServerName, strHttpReLocal, (char*)m_cBuffer) == 1) {
				if ((iHttpCode == DrCOM_HTTP_200) && ((strHttpServerName == DrCOM_Server) || (strHttpServerName == DrCOM_Server_IIS))) {
					iRet = doWithLoginResult(m_cBuffer);
					if (iRet == 1) {
						m_bConnected = true;
					}
				}
			}
		}
	}
	

	return iRet;
}

string DrCOMAuth::getUndefineError() {
	return m_strUndefineError;
}

string DrCOMAuth::getXip() {
	return m_strXip;
}

string DrCOMAuth::getMac() {
	return m_strMac;
}

string DrCOMAuth::getLocalMac() {
#ifdef _WIN32_WP8
	int nSignedLen = strlen(MACSIGNED);
	return m_strMacList.c_str() + nSignedLen;	
#else
	return m_strMacList;
#endif
}

int DrCOMAuth::httpLogout() {
	int iRet = DrClientHS_NetworkError;
	char cRep[DrCOM_BUFFER_256B] = {'\0'};
	int iHttpCode = 0, iContentLen = 0;
	string strHttpServerName = "", strHttpReLocal = "";
	m_socket.Close();

	memset(m_cBuffer, 0, sizeof(m_cBuffer));
	if (m_socket.Connect(m_strGatewayAddress) == 1) {
		sprintf(m_cBuffer, DrCOM_GET, DrCOM_Logout, m_strGatewayAddress.c_str());
		if (m_socket.SendData(m_cBuffer, strlen(m_cBuffer)) == strlen(m_cBuffer)) {
			memset(m_cBuffer, 0, sizeof(m_cBuffer));
			if (httpHandle(&m_socket, iHttpCode, iContentLen, strHttpServerName, strHttpReLocal, (char*)m_cBuffer) == 1) {
				iRet = DrCOM_SUCCESS;
				m_bConnected = false;
			}
		}
	}

	return iRet;
}

void DrCOMAuth::SetGatewayAddress(string strGatewayAddr)
 {
	 m_strGatewayAddress = strGatewayAddr;
 }

string DrCOMAuth::getGatewayAddress() {
	return m_strGatewayAddress;
}

bool DrCOMAuth::getLoginStatus() {
	return m_bConnected;
}

bool DrCOMAuth::httpStatus() {
	bool bRet = false;
	char cRep[DrCOM_BUFFER_256B] = {'\0'};
	int iHttpCode = 0, iContentLen = 0;
	string strHttpServerName = "", strHttpReLocal = "";
	m_socket.Close();

	memset(m_cBuffer, 0, sizeof(m_cBuffer));
	if (m_socket.Connect(m_strGatewayAddress) == 1) {
		sprintf(m_cBuffer, DrCOM_GET, "/", m_strGatewayAddress.c_str());
		if (m_socket.SendData(m_cBuffer, strlen(m_cBuffer)) == strlen(m_cBuffer)) {
			memset(m_cBuffer, 0, sizeof(m_cBuffer));
			if (httpHandle(&m_socket, iHttpCode, iContentLen, strHttpServerName, strHttpReLocal, (char*)m_cBuffer) == 1) {
				if ((iHttpCode == DrCOM_HTTP_200) && ((strHttpServerName == DrCOM_Server) || (strHttpServerName == DrCOM_Server_IIS)) && strstr(m_cBuffer, DrCOM_1_html)) {
					string strTime = findStringBetween(m_cBuffer, (char*)"time='", (char*)"';flow", cRep, sizeof(cRep));
					string strFlow = findStringBetween(m_cBuffer, (char*)"flow='", (char*)"';fsele", cRep, sizeof(cRep));
					if ((strTime.length() > 0) && (strFlow.length() > 0)) {
						m_strTime = trim(strTime);
						strFlow = trim(strFlow);
						//copy from web script
						int iflow = atoi(strFlow.c_str());
						int iflow0 = 0;
						//int iflow1 = 0;
						iflow0 = iflow % 1024;
						//iflow1 = iflow - iflow0;
						iflow0 = iflow0 * 1000;
						iflow0 = iflow0 - iflow0 % 1024;
						string flow3;
						if ((iflow0 / 1024) < 10) {
							flow3 = ".00";
						} else if ((iflow0 / 1024) < 100) {
							flow3 = ".0";
						} else {
							flow3 = ".";
						}
						sprintf(cRep, "%d%s%d", (int)(iflow/1024), flow3.c_str(), (int)(iflow0/1024));
						m_strFlux = cRep;
						bRet = true;
					}
				}
			}
		}
	}

	return bRet;
}

string DrCOMAuth::getFluxStatus() {
	return m_strFlux;
}

string DrCOMAuth::getTimeStatus() {
	return m_strTime;
}

int DrCOMAuth::httpHandle(tcpSocket* ptSocket, int& iHttpCode, int& iContentLen, string& strHttpServerName, string& strHttpReLocal, char* pHttpBody) {
	iHttpCode = 0;
	strHttpServerName = "";
	iContentLen = 0;

	int iRet = -1;
	bool bFlag = true;
	int iRLen = 0, iBufferLen = DrCOM_BUFFER_64K;
	char *pHttpEnd = NULL;
	char cRep[DrCOM_BUFFER_256B] = {'\0'};

	while (true) {
		int iLen = ptSocket->RecvData(pHttpBody + iRLen, iBufferLen - iRLen, false);
		if (iLen == -1) {
			break;
		}
		if (iLen > 0) {
			iRLen += iLen;
		}
		//find http end "\r\n\r\n"
		pHttpEnd = strIstr(pHttpBody, DrCOM_HTTP_END);
		if (pHttpEnd) {
			//get http reponse code
			memset(cRep, 0, sizeof(cRep));
			memcpy(cRep, pHttpBody + 9, 3);
			iHttpCode = atoi(cRep);
			//get content length begin
			iContentLen = atoi(findStringBetween(pHttpBody, (char*)DrCOM_HTTP_CON_LEN, (char*)DrCOM_HTTP_LINE_END, cRep, sizeof(cRep)).c_str());
			//get http server name
			strHttpServerName = findStringBetween(pHttpBody, (char*)DrCOM_HTTP_SERVER, (char*)DrCOM_HTTP_LINE_END, cRep, sizeof(cRep));
			//get location while response code is 302
			if (iHttpCode == DrCOM_HTTP_302) {
				strHttpReLocal = findStringBetween(pHttpBody, (char*)DrCOM_HTTP_LOCATION, (char*)DrCOM_HTTP_LINE_END, cRep, sizeof(cRep));
				int index = strHttpReLocal.find("/");
				if (index > 0) {
					strHttpReLocal = strHttpReLocal.substr(0, index - 1);
				}
			}
			iRet = 1;
			break;
		}
	}

	if (iContentLen > 0) {
		iRet = -1;
		int iHeaderLen = (pHttpEnd - pHttpBody) + strlen(DrCOM_HTTP_END);
		if ((iRLen - iHeaderLen) > 0) {
			iRLen = iRLen - iHeaderLen;
			memcpy(pHttpBody, pHttpBody + iHeaderLen, iRLen);
			memset(pHttpBody + iRLen, 0, iHeaderLen);
		} else {
			iRLen = 0;
		}

		while (iRLen < iContentLen) {
			int iLen = ptSocket->RecvData(pHttpBody + iRLen, iContentLen - iRLen);
			if (iLen == -1) {
				break;
			}
			if (iLen > 0) {
				iRLen += iLen;
			}
		}
		if (iRLen >= iContentLen) {
			iRet = 1;
		}
	}
	return iRet;
}

inline char* DrCOMAuth::strIstr(const char *haystack, const char *needle) {
	if (!*needle) {
		return (char*)haystack;
	}
	for (; *haystack; ++haystack) {
		if (toupper(*haystack) == toupper(*needle)) {
			const char *h, *n;
			for (h = haystack, n = needle; *h && *n; ++h, ++n) {
				if (toupper(*h) != toupper(*n)) {
					break;
				}
			}
			if (!*n) {
				return (char*)haystack;
			}
		}
	}
	return 0;
}

inline string DrCOMAuth::findStringBetween(char* pData, char* pBegin, char* pEnd, char* pTmpBuffer, int iTmpLen) {
	string strRet = "";
	char *pC_Begin = NULL, *pC_End = NULL, *pRep = NULL;
	int iLen = DrCOM_BUFFER_256B;

	if (pTmpBuffer && iTmpLen > 0) {
		pRep = pTmpBuffer;
		iLen = iTmpLen;
	} else {
		pRep = new char[DrCOM_BUFFER_256B];
	}
	memset(pRep, 0, iLen);

	if ((pC_Begin = strIstr(pData, pBegin)) > 0) {
		pC_Begin = pC_Begin + strlen(pBegin);
		if ((pC_End = strIstr(pC_Begin, pEnd)) > 0) {
			memcpy(pRep, pC_Begin, pC_End - pC_Begin);
			strRet = pRep;
		}
	}

	if (!pTmpBuffer || iTmpLen <= 0) {
		delete pRep;
	}

	return strRet;
}

inline string DrCOMAuth::grantMD5(string strData) {
	string strRet = "";
	unsigned char md[DrCOM_BUFFER_16B] = {'\0'};
	char cTmp[3]={'\0'};

	md5((unsigned char*)strData.c_str(), strData.length(), md);
	for (int i = 0; i < 16; i++){
		sprintf(cTmp,"%02x", md[i]);
		strRet += cTmp;
	}

	return strRet;
}

inline string DrCOMAuth::trim(string strData) {
	return strData.erase(0, strData.find_first_not_of(" \t\r\n")).erase(strData.find_last_not_of(" \t\r\n") + 1);
}

inline bool DrCOMAuth::IsPage0(int iHttpCode, string strHttpServerName)
{
	return ((iHttpCode == DrCOM_HTTP_200) && ((strHttpServerName == DrCOM_Server) || (strHttpServerName == DrCOM_Server_IIS)) && strstr(m_cBuffer, DrCOM_0_html));
}

inline int DrCOMAuth::Page0Process(int iHttpCode, int iContentLen, string strHttpServerName, string strHttpReLocal) 
{
	int iRet = DrClientHS_NotFoundDrCOM;
	char cRep[DrCOM_BUFFER_256B] = {'\0'};
	
	string strAddress = findStringBetween(m_cBuffer, (char*)"v46ip='", (char*)"'", cRep, sizeof(cRep));
	if (strAddress.length() < 1) {
		strAddress = findStringBetween(m_cBuffer, (char*)"v46ip=\"", (char*)"\"", cRep, sizeof(cRep));
	}
	
	if (strHttpReLocal.length() < 1) {
		strHttpReLocal = findStringBetween(m_cBuffer, (char*)"v4serip='", (char*)"'", cRep, sizeof(cRep));
		if (strHttpReLocal.length() < 1) {
			strHttpReLocal = findStringBetween(m_cBuffer, (char*)"v4serip=\"", (char*)"\"", cRep, sizeof(cRep));
		}
	}
	
	if ((strAddress.length() < 1) || (!tcpSocket::CompareLocalAddress(strAddress))) {
		iRet = DrClientHS_UsingNAT;
	} else if (strHttpReLocal.length() > 0) {
		m_strGatewayAddress = strHttpReLocal;
		iRet = DrCOM_SUCCESS;
	}
	return iRet;
}

int DrCOMAuth::doWithLoginResult(char* pData) {
	m_strUndefineError = "";
	m_strXip = "";
	m_strMac = "";

	int iRet = -1;
	char cRep[DrCOM_BUFFER_256B] = {'\0'};

	if (strstr(m_cBuffer, DrCOM_2_html)) {
		string strMsg = findStringBetween(pData, (char*)"Msg=", (char*)";time", cRep, sizeof(cRep));
		string strMsga = findStringBetween(pData, (char*)"msga='", (char*)"';", cRep, sizeof(cRep));
		string strXip = findStringBetween(pData, (char*)"xip=", (char*)";mac", cRep, sizeof(cRep));
		string strMac = findStringBetween(pData, (char*)"mac=", (char*)";va", cRep, sizeof(cRep));
		string strTime = findStringBetween(pData, (char*)"time='", (char*)"';flow", cRep, sizeof(cRep));
		string strFlow = findStringBetween(pData, (char*)"flow='", (char*)"';fsele", cRep, sizeof(cRep));
		string strCode = findStringBetween(pData, (char*)"mcode = ", (char*)";", cRep, sizeof(cRep));
		iRet = loginStatus(strMsg, strMsga, strXip, strMac, strTime, strFlow, strCode);
	} else if (strstr(m_cBuffer, DrCOM_3_html)) {
		iRet = DrCOM_SUCCESS;
	} else {
		iRet = DrClientHS_NotFoundDrCOM;
	}

	if (iRet == DrCOM_SUCCESS) {
		//connect to update server
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
		//HANDLE hThread = CreateThread(NULL, 0, requestUpdate, (void*)this, 0, NULL);
		//CloseHandle(hThread);
		requestUpdate((void*)this);
#else
		pthread_t th;
		pthread_create(&th, NULL, &requestUpdate, (void*)this);
#endif
	}

	return iRet;
}

int DrCOMAuth::loginStatus(string strMsg, string strMsga, string strXip, string strMac, string strTime, string strFlow, string strCode) {
	int iRet = DrCOM_SUCCESS;

	if (strMsg.length() > 1) {
		if (strMsg == "00" || strMsg == "01") {
			if (strMsga.length() > 0) {
				if (strMsga == "error0") {
					iRet = DrClientHS_IpNotAllowLogin;
				} else if (strMsga == "error1") {
					iRet = DrClientHS_AccountNotAllow;
				} else if (strMsga == "error2") {
					iRet = DrClientHS_NotAllowChangePwd;
				} else {
					m_strUndefineError = strMsga;
					iRet = DrClientHS_UndefineError;
				}
			} else {
				iRet = DrClientHS_InvalidAccountOrPwd;
			}
		} else if (strMsg == "02") {
			m_strXip = strXip;
			m_strMac = strMac;
			iRet = DrClientHS_AccountTieUp;
		} else if (strMsg == "03") {
			m_strXip = strXip;
			iRet = DrClientHS_UsingOnAppointedIp;
		} else if (strMsg == "04") {
			iRet = DrClientHS_ChargeOrFluxOver;
		} else if (strMsg == "05") {
			iRet = DrClientHS_AccountBreakOff;
		} else if (strMsg == "06") {
			iRet = DrClientHS_SystemBufferFull;
		} else if (strMsg == "08") {
			iRet = DrClientHS_TieUpCannotAmend;
		} else if (strMsg == "09") {
			iRet = DrClientHS_NewAndConfirmPwdDiffer;
		} else if (strMsg == "10") {
			iRet = DrClientHS_PwdAmendSuccessed;
		} else if (strMsg == "11") {
			m_strMac = strMac;
			iRet = DrClientHS_UsingAppointedMac;
		} else if (strMsg == "15") {
			iRet = DrCOM_SUCCESS;
		}
	} else {
		if (strCode == "0000" || strCode == "FFFF") {
			iRet = DrCOM_SUCCESS;
		}
	}

	return iRet;
}

string DrCOMAuth::grantUpdateRequest() {
	char cBuffer[DrCOM_BUFFER_512B] = {'\0'};
	//grant time
	string strTime = "";
	time_t stm = time(NULL);
	struct tm tTime;
	localtime_r(&stm,&tTime);
	sprintf(cBuffer, "%04d%02d%02d%02d%02d%02d", tTime.tm_year+1900, tTime.tm_mon+1, tTime.tm_mday, tTime.tm_hour, tTime.tm_min, tTime.tm_sec);
	strTime = cBuffer;

	//grant time
	string strType = "";
	int iType = (int)time(NULL) % 3;
	switch (iType) {
		case 0: strType = DrCOM_HTTP_TYPE_0000; break;
		case 1: strType = DrCOM_HTTP_TYPE_0003; break;
		case 2: strType = DrCOM_HTTP_TYPE_0006; break;
	}

	//grant key
	string strKey = "";

	//grant hash & version
	string strHash = DrCOM_Hash;
	string strVer = DrCOM_Version;

	//grant rand
	int iRnd = rand() % 1000000;
	sprintf(cBuffer, "%06d", iRnd);
	string strRnd = cBuffer;

	//grant chk
	string strChk = "";
	string strTmp = "";

	if (strType == DrCOM_HTTP_TYPE_0000) {
		strTmp = strTime+  strHash + strVer + strKey;
		strChk = grantMD5(strTmp);
		strChk = strChk.substr(strChk.length() - 8, 8);
	} else {
		strTmp = strTime + strHash + strVer + strKey + strRnd;
		if (strType == DrCOM_HTTP_TYPE_0003) {
			Arithmetic ath;
			unsigned long ulChk = ath.MakeCRC32((char*)strTmp.c_str(), strTmp.length());
			sprintf(cBuffer, "%u", ulChk);
			strChk = cBuffer;
		} else if (strType == DrCOM_HTTP_TYPE_0006) {
			strChk = grantMD5(strTmp);
			strChk = strChk.substr(strChk.length() - 8, 8);
		}
	}

	//get mac hash
	string strMac = grantMD5(m_strMacList);
	sprintf(cBuffer, DrCOM_Update_Parm, strTime.c_str(), strType.c_str(), strKey.c_str(), strHash.c_str(), strVer.c_str(), strRnd.c_str(), strChk.c_str(), strMac.c_str());
	string strRet = cBuffer;
	return strRet;
}

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
DWORD DrCOMAuth::requestUpdate(void* pData)
#else
void* DrCOMAuth::requestUpdate(void* pData) 
#endif
{
	if (!pData) {
		return NULL;
	}
	DrCOMAuth* pDrCOMAuth = (DrCOMAuth*)pData;
	tcpSocket tSocket;

	char cBuffer[DrCOM_BUFFER_1K] = {'\0'};
	memset(cBuffer, 0, sizeof(cBuffer));
	if (tSocket.Connect(DrCOM_Update_Svr) == 1) {
		sprintf(cBuffer, DrCOM_GET, pDrCOMAuth->grantUpdateRequest().c_str(), DrCOM_Update_Svr);
		if (tSocket.SendData(cBuffer, strlen(cBuffer)) == strlen(cBuffer)) {
		}
	}
	tSocket.Close();
	return NULL;
}

// 初始化socket环境
bool DrCOMAuth::InitSocketEnvironment()
{
	bool result = false;
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	 
	wVersionRequested = MAKEWORD( 2, 2 );
	 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err == 0 ) {
		result = true;
	}
#else 
	result = true;
#endif
	return result;
}

// 清除socket环境
void DrCOMAuth::ReleaseSocketEnvironment()
{
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
	WSACleanup();
#endif
}

void DrCOMAuth::SetSSID(const string& ssid)
{
	m_strSSID = ssid;
}

#ifdef _WIN32_WP8
void DrCOMAuth::SetMac(const string& mac)
{
	m_strMacList = MACSIGNED;
	m_strMacList += mac;
}
#endif
