/*
 * File         : DrCOMSocket.h
 * Date         : 2011-07-11
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOM socket include
 */

#ifndef _INC_DRCOMSOCKET_
#define _INC_DRCOMSOCKET_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <vector>
#include <iostream>

#include "polarssl/config.h"
#include "polarssl/net.h"
#include "polarssl/ssl.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/error.h"
#include "polarssl/certs.h"

#include "DrCOMDefine.h"

using namespace std;

class tcpSocket
{
public:
	tcpSocket();
	~tcpSocket();

	virtual int Connect(string strAddress, unsigned int uiPort = HTTP_PROT);
	virtual int SendData(char* pBuffer, unsigned int uiSendLen, unsigned int uiTimeout = HTTP_SEND_TIMEOUT);
	virtual int RecvData(char* pBuffer, unsigned int uiRecvLen, bool bRecvAll = true, unsigned int uiTimeout = HTTP_RECV_TIMEOUT);
	virtual int Close();
	
public:
	static bool CompareLocalAddress(string strAddress);
	static string GetFirstMacAddress();
	static string GetFirstIpAddress();
	static string GetMacAddressList();

protected:
	int BaseConnect(string strAddress, unsigned int uiPort, bool bBlock);

protected:
#ifndef WIN32
	int m_iSocket;
#else
	SOCKET	m_iSocket;
#endif
	string m_strAddress;
	unsigned int m_uiPort;
};

class udpSocket
{
public:
	udpSocket();
	~udpSocket();

	virtual int Connect(string strAddress, unsigned int uiPort = HTTP_PROT);
	virtual int SendData(const char* pBuffer, unsigned int uiSendLen, unsigned int uiTimeout = NTP_SEND_TIMEOUT);
	virtual int RecvData(const char* pBuffer, unsigned int uiRecvLen, unsigned int uiTimeout = NTP_RECV_TIMEOUT);
	virtual int Close();

protected:
#ifndef WIN32
	int m_iSocket;
#else
	SOCKET	m_iSocket;
#endif
	string m_strAddress;
	unsigned int m_uiPort;
};

class sslSocket : public tcpSocket
{
public:
	sslSocket();
	~sslSocket();

	int Connect(string strAddress, unsigned int uiPort = HTTPS_PROT);
	int SendData(char* pBuffer, unsigned int uiSendLen, unsigned int uiTimeout = HTTP_SEND_TIMEOUT);
	int RecvData(char* pBuffer, unsigned int uiRecvLen, bool bRecvAll = true, unsigned int uiTimeout = HTTP_RECV_TIMEOUT);
	int Close();

private:
	int InitializeEntropy();					// Initialize the RNG and the session data
	int InitializeCertificates();			// Initialize certificates
	int StartConnect(const char *host, int port);		// Start the connection
	int InitializeSSL();
	int SSLHandshake();					// Handshake
	int VerifySrvCert();						// Verify the server certificate
	bool Write(const unsigned char *buffer, int ilen);			// 向服务器写入数据
	bool Read(char buffer[2048]);						// 向服务器读取数据
	void ReleaseResource();					// 释放相关的资源

private:
	int					m_socket;	// Server Socket
	unsigned char		m_buf[10000];
	ssl_context			m_ssl;
	entropy_context		m_entropy;				// 熵
	ctr_drbg_context	m_ctr_drbg;
	x509_cert			m_cacert;
};

#endif
