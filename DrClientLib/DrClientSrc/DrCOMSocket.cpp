/*
 * File         : DrCOMSocket.cpp
 * Date         : 2011-07-11
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOM socket
 */

#include "DrClientInclude.h"
#if (!defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WIN32_WP8))//ISNOTWINDOWSENVIROMENT
#include <unistd.h>
#include <resolv.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#ifdef _IOS
#include "IPAddress.h"
#endif /* _IOS */
#endif

#include "DrCOMSocket.h"

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8) || defined(_IOS)
    #if !defined(_IOS)
        #define ioctl		ioctlsocket
    #endif
	#define tv_set		tv_sec
	#define tv_uset		tv_usec
	#define socklen_t	int
	//#define usleep		Sleep
#endif


#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm. tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif

// ---------- global function ------------
inline unsigned int GetTick()
{
	timeval tNow;
	gettimeofday(&tNow, NULL);
	if (tNow.tv_usec != 0){
		return (tNow.tv_sec * 1000 + (unsigned int)(tNow.tv_usec / 1000));
	}else{
		return (tNow.tv_sec * 1000);
	}
}
	
inline bool isTimeout(unsigned int uiStart, unsigned int uiTimeout)
{
	unsigned int uiCurr = GetTick();
	unsigned int uiDiff;

	if (uiTimeout == 0)
		return false;
	if (uiCurr >= uiStart) {
		uiDiff = uiCurr - uiStart;
	}else{
		uiDiff = (0xFFFFFFFF - uiStart) + uiCurr;
	}
	if(uiDiff >= uiTimeout){
		return true;
	}
	return false;
}
// --------------------------------------

tcpSocket::tcpSocket() {
	m_iSocket = -1;
	m_strAddress = "";
	m_uiPort = 0;
}

tcpSocket::~tcpSocket() {
	Close();
}

int tcpSocket::Connect(string strAddress, unsigned int uiPort) {
	return BaseConnect(strAddress, uiPort, false);
}

int tcpSocket::SendData(char* pBuffer, unsigned int uiSendLen, unsigned int uiTimeout) {
	unsigned int uiBegin = GetTick();
	int iOrgLen = uiSendLen;
	int iRet = -1, iRetS = -1;
	int iSendedLen = 0;
	fd_set wset;

	while (true) {
		struct timeval tout;
		tout.tv_sec = uiTimeout / 1000;
		tout.tv_usec = (uiTimeout % 1000) * 1000;
		FD_ZERO(&wset);
		FD_SET(m_iSocket, &wset);
		iRetS = select(m_iSocket + 1, NULL, &wset, NULL, &tout);
		if (iRetS == -1) {
			return -1;
		} else if (iRetS == 0) {
			return iSendedLen;
		}
		iRet = send(m_iSocket, pBuffer, uiSendLen, 0);
		if (iRet == -1 || (iRetS == 1 && iRet == 0)) {
			if (iSendedLen == 0){
				return -1;
			}
			return iSendedLen;
		} else if (iRet == 0) {
			return iSendedLen;
		}
		pBuffer += iRet;
		iSendedLen += iRet;
		uiSendLen -= iRet;
		if (iSendedLen >= iOrgLen) {
			return iSendedLen;
		}
		if (isTimeout(uiBegin, uiTimeout)) {
			return iSendedLen;
		}
	}
	return 0;
}

int tcpSocket::RecvData(char* pBuffer, unsigned int uiRecvLen, bool bRecvAll, unsigned int uiTimeout) {
	unsigned int uiBegin = GetTick();
	int iOrgLen = uiRecvLen;
	int iRet = -1, iRetS = -1;
	int iRecvedLen = 0;
	fd_set rset;

	while (true) {
		timeval tout;
		tout.tv_sec = uiTimeout / 1000;
		tout.tv_usec = (uiTimeout % 1000) * 1000;
		FD_ZERO(&rset);
		FD_SET(m_iSocket, &rset);
		iRetS = select(m_iSocket + 1, &rset, NULL, NULL, &tout);
		if (iRetS == -1) {
			return -1;
		} else if (iRetS == 0) {
			return iRecvedLen;
		}
		iRet = recv(m_iSocket, pBuffer, uiRecvLen, 0);
		if (iRet==-1 || (iRetS == 1 && iRet == 0)) {
			if (iRecvedLen == 0){
				return -1;
			}
			return iRecvedLen;
		} else if (iRet == 0) {
			return iRecvedLen;
		}
		pBuffer += iRet;
		iRecvedLen += iRet;
		uiRecvLen -= iRet;
		if (iRecvedLen >= iOrgLen) {
			return iRecvedLen;
		}
		if (!bRecvAll) {
			return iRecvedLen;
		}
		if (isTimeout(uiBegin, uiTimeout)){
			return iRecvedLen;
		}
	}
	return 0;
}

int tcpSocket::Close() {
	if (m_iSocket != -1) {
		shutdown(m_iSocket, SHUT_RDWR);
		close(m_iSocket);
		m_iSocket = -1;
	}
	return 1;
}

// 比较本地IP是否存在输入IP
bool tcpSocket::CompareLocalAddress(string strAddress) 
{
	bool bRet = false;
#if (!defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WIN32_WP8) && !defined(_IOS))
	int fd = 0, intrface = 0;
	struct ifconf ifc;
	string strTmp = "";
	struct ifreq buf[DrCOM_MAX_INFERFACE];
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = (caddr_t)buf;

		if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc)) {
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0){
				if (!(ioctl(fd, SIOCGIFADDR, (char *) &buf[intrface]))) {
					strTmp = inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
					if (strTmp == strAddress) {
						bRet = true;
						break;
					}
				}
			}
		}
	}
	close(fd);
#elif defined (_IOS)
	InitAddresses();
    int count = GetIPAddresses();
	for (int i = 0; i < count; i++) {
		string name = if_names[i];
		string ip = ip_names[i];
		if (strAddress == ip) {
			bRet = true;
			break;
		}
	}
	FreeAddresses();
#else
	char szHostName[256]; 
	struct hostent * pHost;  
	unsigned int i;  
	if( gethostname(szHostName, sizeof(szHostName)) == 0 ) 
	{ 
		pHost = gethostbyname(szHostName);  
		for( i = 0; pHost!= NULL && pHost->h_addr_list[i]!= NULL; i++ )  
		{
			string localIP = inet_ntoa((*(struct in_addr *)pHost->h_addr_list[i]));
			if (strAddress.compare(localIP) == 0) {
				bRet = true;
				break;
			}
		} 
	}
#endif

	return bRet;
}

string tcpSocket::GetFirstMacAddress() {
	string strRet = "";

#if !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WIN32_WP8) && !defined(_IOS)
	int fd = 0, intrface = 0;
	struct ifconf ifc;
	struct ifreq buf[DrCOM_MAX_INFERFACE];
	char cBuffer[DrCOM_BUFFER_256B] = {'\0'};
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = (caddr_t)buf;

		if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc)) {
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0){
				if (!(ioctl(fd, SIOCGIFHWADDR, (char *) &buf[intrface]))) {
					sprintf(cBuffer, "%02x:%02x:%02x:%02x:%02x:%02x", buf[intrface].ifr_hwaddr.sa_data[0], buf[intrface].ifr_hwaddr.sa_data[1],
							buf[intrface].ifr_hwaddr.sa_data[2], buf[intrface].ifr_hwaddr.sa_data[3], buf[intrface].ifr_hwaddr.sa_data[4],
							buf[intrface].ifr_hwaddr.sa_data[5]);
					if (strcmp(cBuffer, "00:00:00:00:00:00")) {
						strRet = cBuffer;
					}
					if (strRet.length() > 0) {
						break;
					}
				}
			}
		}
	}
	close(fd);
#else 


#endif

	return strRet;
}

string tcpSocket::GetFirstIpAddress() {
	string strRet = "";

#if !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WIN32_WP8)
	int fd = 0, intrface = 0;
	struct ifconf ifc;
	struct ifreq buf[DrCOM_MAX_INFERFACE];
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = (caddr_t)buf;

		if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc)) {
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0){
				if (!(ioctl(fd, SIOCGIFADDR, (char *) &buf[intrface]))) {
					strRet = inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
					if ((strRet.length() > 0) && (strRet != "127.0.0.1")) {
						break;
					}
				}
			}
		}
	}
	close(fd);
#else
	char szHostName[256]; 
	struct hostent * pHost;  
	unsigned int i;  
	if( gethostname(szHostName, sizeof(szHostName)) == 0 ) 
	{ 
		pHost = gethostbyname(szHostName);  
		for( i = 0; pHost!= NULL && pHost->h_addr_list[i]!= NULL; i++ )  
		{
			strRet = inet_ntoa((*(struct in_addr *)pHost->h_addr_list[i]));
			if ((strRet.length() > 0) && (strRet != "127.0.0.1")) {
						break;
			}
		} 
	}
#endif

	return strRet;
}

string tcpSocket::GetMacAddressList() {
	string strRet = "";

#if !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(_WIN32_WP8) && !defined(_IOS)
	int fd = 0, intrface = 0, iCount = 1;
	struct ifconf ifc;
	struct ifreq buf[DrCOM_MAX_INFERFACE];
	char cBuffer[DrCOM_BUFFER_256B] = {'\0'};
	char cBufferTmp[DrCOM_BUFFER_256B] = {'\0'};
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = (caddr_t)buf;

		if (!ioctl(fd, SIOCGIFCONF, (char*)&ifc)) {
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			while (intrface-- > 0){
				if (!(ioctl(fd, SIOCGIFHWADDR, (char *) &buf[intrface]))) {
					sprintf(cBuffer, "%02X:%02X:%02X:%02X:%02X:%02X", buf[intrface].ifr_hwaddr.sa_data[0], buf[intrface].ifr_hwaddr.sa_data[1],
							buf[intrface].ifr_hwaddr.sa_data[2], buf[intrface].ifr_hwaddr.sa_data[3], buf[intrface].ifr_hwaddr.sa_data[4],
							buf[intrface].ifr_hwaddr.sa_data[5]);
					if (strcmp(cBuffer, "00:00:00:00:00:00")) {
						sprintf(cBufferTmp, "&m%d=%s", iCount++, cBuffer);
						strRet += cBufferTmp;
					}
				}
			}
		}
	}
	close(fd);
    
#elif defined (_IOS)
    InitAddresses();
    int count = GetIPAddresses();
    GetHWAddresses();

    char cBufferTmp[DrCOM_BUFFER_256B] = {'\0'};    
    int iCount = 1;
	for (int i = 0; i < count; i++) {
        string name = if_names[i];
		string mac = hw_addrs[i];
        if ((name.length() > 0) && 0!=name.compare("lo") && 0!=mac.compare("00:00:00:00:00:00")) {
            memset(cBufferTmp, '\0', DrCOM_BUFFER_256B);
            sprintf(cBufferTmp, "&m%d=%s", iCount++, hw_addrs[i]);
			strRet += cBufferTmp;
		}
	}
    FreeAddresses();
#endif
    
    if (strRet.length() > 1) {
        strRet = strRet.substr(1, strRet.length());
    }
	return strRet;
}

inline int tcpSocket::BaseConnect(string strAddress, unsigned int uiPort, bool bBlock) {
	m_strAddress = strAddress;
	m_uiPort = uiPort;

	int iRet = -1, iFlag = 1;
	struct sockaddr_in dest;
	hostent* hent = NULL;

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
	if (INVALID_SOCKET != (m_iSocket = socket(AF_INET, SOCK_STREAM, 0))) 
#else
	if ((m_iSocket = socket(AF_INET, SOCK_STREAM, 0)) >= 0) 
#endif
	{
		memset(&dest, 0, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons(uiPort);

		dest.sin_addr.s_addr = inet_addr((const char*)strAddress.c_str());
		if (dest.sin_addr.s_addr == -1L) {
			if ((hent = gethostbyname((const char*)strAddress.c_str())) != NULL) {
				dest.sin_family = hent->h_addrtype;
				memcpy((char*)&dest.sin_addr, hent->h_addr, hent->h_length);
			} else {
				goto EXIT_ERROR_TCP;
			}
		}

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
		setsockopt(m_iSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&iFlag, sizeof(iFlag));
#endif
		iFlag = 1500;
		setsockopt(m_iSocket, SOL_SOCKET, SO_RCVBUF, (const char *)&iFlag, sizeof(iFlag));
		iFlag = 1;
		setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&iFlag, sizeof(iFlag));

		unsigned long ul = 1;
		ioctl(m_iSocket, FIONBIO, &ul); //设置为非阻塞模式

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
		if (connect(m_iSocket, (struct sockaddr *)&dest, sizeof(dest)) != SOCKET_ERROR 
			|| WSAGetLastError() == WSAEWOULDBLOCK) 
		{
#else
		if (connect(m_iSocket, (struct sockaddr *)&dest, sizeof(dest)) != -1
            || errno == EINPROGRESS)
        {
#endif
			timeval tm;
			fd_set set;
			int error = 0;
			int len = sizeof(error);
            tm.tv_sec  = 3;
			tm.tv_usec = 0;
			FD_ZERO(&set);
			FD_SET(m_iSocket, &set);
			if( select(m_iSocket+1, NULL, &set, NULL, &tm) > 0)
			{
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
				getsockopt(m_iSocket, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t*)&len);
#endif
				iRet = (error == 0 ? 1 : -1);
			}

			if (iRet == 1) {
				ul = 0;
				ioctl(m_iSocket, FIONBIO, &ul); //设置为阻塞模式
			}
		}
	}

EXIT_ERROR_TCP:
	if (iRet != 1) {
		Close();
	}
	return iRet;
}

/*****************************************************************/
udpSocket::udpSocket()
{
	m_iSocket = -1;
	m_strAddress = "";
	m_uiPort = 0;
}
	
udpSocket::~udpSocket()
{
	Close();
}

int udpSocket::Connect(string strAddress, unsigned int uiPort)
{
	m_strAddress = strAddress;
	m_uiPort = uiPort;
	
	int iRet = -1, iFlag = 1;
	struct sockaddr_in dest;
	hostent* hent = NULL;

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
	if (INVALID_SOCKET != (m_iSocket = socket(AF_INET, SOCK_DGRAM, 0))) 
#else
	if ((m_iSocket = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) 
#endif
	{
		memset(&dest, 0, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons(uiPort);

		dest.sin_addr.s_addr = inet_addr((const char*)strAddress.c_str());
		if (dest.sin_addr.s_addr == -1L) {
			if ((hent = gethostbyname((const char*)strAddress.c_str())) != NULL) {
				dest.sin_family = hent->h_addrtype;
				memcpy((char*)&dest.sin_addr, hent->h_addr, hent->h_length);
			} else {
				goto EXIT_ERROR_UDP;
			}
		}

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
		if (connect(m_iSocket, (struct sockaddr *)&dest, sizeof(dest)) != SOCKET_ERROR ) 
#else
		if (connect(m_iSocket, (struct sockaddr *)&dest, sizeof(dest)) != -1) 
#endif
		{
			iRet = 1;
		}
	}

EXIT_ERROR_UDP:
	if (iRet != 1) {
		Close();
	}
	return iRet;
}


int udpSocket::SendData(const char* pBuffer, unsigned int uiSendLen, unsigned int uiTimeout)
{
	int iRet = -1;
	const char* pBufferSend = pBuffer;
	unsigned int uiBufferSendLen = uiSendLen;
	const unsigned int uiSendStep = 1000;
	int iFailCount = 0;

	// 设置超时
	struct timeval timeout;
	timeout.tv_sec = uiTimeout/1000;   
    timeout.tv_usec = uiTimeout % 1000;
    if (setsockopt(m_iSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout.tv_sec,sizeof(struct timeval)) >= 0) {
		iRet = 1;
		while (uiBufferSendLen > 0) 
		{
			unsigned int uiToSend = uiBufferSendLen > uiSendStep ? uiSendStep : uiBufferSendLen;
			unsigned int uiSent = send(m_iSocket, pBufferSend, uiToSend, 0);
			if (uiSent > 0) {
				pBufferSend += uiSent;
				uiBufferSendLen -= uiSent;
			}
			else if (uiSent == 0 && iFailCount < 5) {
				iFailCount++;
			}
			else {
				iRet = -1;
				break;
			}
		}
	}

	return iRet;
}
	
int udpSocket::RecvData(const char* pBuffer, unsigned int uiRecvLen, unsigned int uiTimeout)
{
	unsigned int uiBegin = GetTick();
	int iRet = -1;
	int iRecvedLen = 0;

	// 设置超时
	struct timeval timeout;
	timeout.tv_sec = uiTimeout/1000;   
    timeout.tv_usec = uiTimeout % 1000;
	if (setsockopt(m_iSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout.tv_sec,sizeof(struct timeval)) >= 0) {
		while (true) {
			iRet = recv(m_iSocket, (char*)pBuffer, uiRecvLen, 0);
			if (iRet==-1 || iRet == 0) {
				return iRecvedLen == 0 ? -1 : iRecvedLen;
			} else if (iRet == 0) {
				return iRecvedLen;
			}
			else {
				pBuffer += iRet;
				iRecvedLen += iRet;
				uiRecvLen -= iRet;
				return iRecvedLen;
			}
		}
	}
	return 0;
}

int udpSocket::Close()
{
	if (m_iSocket != -1) {
		shutdown(m_iSocket, SHUT_RDWR);
		close(m_iSocket);
		m_iSocket = -1;
	}
	return 1;
}

/*****************************************************************/
#define GetResult(result) ((result >= 0) ? 1 : -1)
#define IsSuccess(result) (result == 1)
sslSocket::sslSocket() 
{
	memset(&m_ssl, 0, sizeof(m_ssl));
	memset(&m_cacert, 0, sizeof(m_cacert));
	memset(&m_socket, 0, sizeof(m_socket));
}

sslSocket::~sslSocket() 
{
	Close();
}

int sslSocket::Connect(string strAddress, unsigned int uiPort) 
{
	int iRet = -1, iFlag = -1;

	iRet = GetResult( InitializeEntropy() );
	if ( IsSuccess(iRet) ) {
		iRet = GetResult( StartConnect(strAddress.c_str(), uiPort) );
	}
	if ( IsSuccess(iRet) ) {
		iRet = GetResult( InitializeSSL() );
	}
	if ( IsSuccess(iRet) ) {
		iRet = GetResult( SSLHandshake() );
	}
	if ( IsSuccess(iRet) ) {
		iRet = GetResult( VerifySrvCert() );
	}
	return iRet;
}

int sslSocket::SendData(char* pBuffer, unsigned int uiSendLen, unsigned int uiTimeout) 
{
	int ret = ssl_write(&m_ssl, (const unsigned char*)pBuffer, uiSendLen);
	return ret > 0 ? ret : -1;
}

int sslSocket::RecvData(char* pBuffer, unsigned int uiRecvLen, bool bRecvAll, unsigned int uiTimeout) 
{
	int ret = ssl_read(&m_ssl, (unsigned char*)pBuffer, uiRecvLen);
	return ret > 0 ? ret : -1;
}

int sslSocket::Close() 
{
	ReleaseResource();
	return 1;
}

#pragma region ssl函数
// Initialize the RNG and the session data
int sslSocket::InitializeEntropy()
{
	char *pers = "ssl_client1";
	memset(&m_ssl, 0, sizeof(ssl_context) );
	memset(&m_cacert, 0, sizeof(x509_cert) );
	entropy_init(&m_entropy);
	return ctr_drbg_init(&m_ctr_drbg, entropy_func, &m_entropy,(unsigned char*)pers, strlen(pers));
}

// Initialize certificates
int sslSocket::InitializeCertificates()
{
	int ret;
#if defined(POLARSSL_CERTS_C)
	ret = x509parse_crt(&m_cacert, (unsigned char*)test_ca_crt, strlen(test_ca_crt) );
#else
	ret = 1;
#endif
	
	return ret;
}
// Start the connection
int sslSocket::StartConnect(const char *host, int port)
{
	return net_connect(&m_socket, host, port);
}

int  sslSocket::InitializeSSL()
{
	int ret = ssl_init(&m_ssl);
	if( ret == 0 )
	{
		ssl_set_endpoint(&m_ssl, SSL_IS_CLIENT);
		ssl_set_authmode(&m_ssl, SSL_VERIFY_NONE);
		ssl_set_ca_chain(&m_ssl, &m_cacert, NULL, "PolarSSL");
		ssl_set_bio(&m_ssl, net_recv, &m_socket,
			net_send, &m_socket);
		// Set  Debug
		ssl_set_rng(&m_ssl, ctr_drbg_random, &m_ctr_drbg);
	}
	return ret;
}
// Handshake

int sslSocket::SSLHandshake()
{
	int ret;
	while( ( ret = ssl_handshake( &m_ssl ) ) != 0 )
	  {
			if( ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE )
			{
				break;
			}
	  }
	return ret;
}

int sslSocket::VerifySrvCert()
{
	return ssl_get_verify_result(&m_ssl);
}

//释放资源
void sslSocket::ReleaseResource()
{
	x509_free(&m_cacert);
	memset(&m_cacert, 0, sizeof(m_cacert));
	net_close(m_socket);
	memset(&m_socket, 0, sizeof(m_socket));
	ssl_free(&m_ssl);
	memset(&m_ssl, 0, sizeof(m_ssl));
}
#pragma endregion
