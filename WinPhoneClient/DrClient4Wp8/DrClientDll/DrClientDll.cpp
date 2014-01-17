// DrClientDll.cpp
#include "pch.h"
#include "DrClientDll.h"
#include <vector>
//#include "IDrCOMAuth.h"

using namespace DrClientDll;
using namespace Platform;
using namespace std;

DrClient::DrClient()
{
	m_pAuth = NULL;
}

DrClient::~DrClient()
{
	IDrCOMAuth::ReleaseDrCOMAuth(m_pAuth);
}

// 初始化
bool DrClient::Init()
{
	m_pAuth = IDrCOMAuth::CreateDrCOMAuth();
	if (NULL != m_pAuth) {
		if ( !IDrCOMAuth::InitSocketEnvironment() ) {
			// 初始化失败
			IDrCOMAuth::ReleaseDrCOMAuth(m_pAuth);
			m_pAuth = NULL;
		}
	}
	return NULL != m_pAuth;
}

// 设置设备ID
void DrClient::SetDeviceID(const Platform::Array<unsigned char>^ data)
{
	char cDeviceID[8] = {0};
	const int nMaxMacLength = 6;
	if (data->Length >= nMaxMacLength && NULL != m_pAuth) {
		string strMac;
		for (int i = nMaxMacLength; i > 0; i--)
		{
			sprintf_s(cDeviceID, sizeof(cDeviceID), "%0.2X", data[data->Length - i]);
			strMac += cDeviceID;
			if (i>1) {
				strMac += ":";
			}
		}
		m_pAuth->SetMac(strMac);
	}
}

// 登录
int DrClient::Login(Platform::String^ gatewayAddr, Platform::String^ account, Platform::String^ password)
{
	int result = DrClientHS_UndefineError;
	if (NULL != m_pAuth) {
		string strGatewayAddr = TransString(gatewayAddr->Data());
		string strAccount = TransString(account->Data());
		string strPassword = TransString(password->Data());
		result = m_pAuth->httpLogin(strGatewayAddr, strAccount, strPassword);
		result = result > 0 ? DrClientHS_Success : result;
	}
	return result;
}

// 注销
int DrClient::Logout()
{
	int result = DrClientHS_UndefineError;
	if (NULL != m_pAuth) {
		result = m_pAuth->httpLogout();
		result = result > 0 ? DrClientHS_Success : result;
	}
	return result;
}

// 获取网络状态
int DrClient::GetStatus()
{
	int result = DrClientHS_UndefineError;
	if (NULL != m_pAuth) {
		// result = result > 0 ? DrClient_Success : result;     // 写法有问题，会造成result返回 恒为0;
		result =  m_pAuth->httpStatus() ? DrClientHS_Success : result;
	}
	return result;
}

// 停止（停止登录/注销动作）
int DrClient::Stop()
{
	int result = DrClientHS_UndefineError;
	if (NULL != m_pAuth) {
		result = m_pAuth->httpStop();
		result = result > 0 ? DrClientHS_Success : result;
	}
	return result;
}

// 获取未定义错误描述
Platform::String^ DrClient::GetUndefineError()
{
	if (NULL != m_pAuth) {
		string strError = m_pAuth->getUndefineError();
		wstring wstrError = TransWString(strError);
		return ref new String(wstrError.c_str());
	}
	return ref new String(L"");
}

// 获取ip
Platform::String^ DrClient::GetXip()
{
	if (NULL != m_pAuth) {
		string strError = m_pAuth->getXip();
		wstring wstrError = TransWString(strError);
		return ref new String(wstrError.c_str());
	}
	return ref new String(L"");
}

// 获取网关传过来Mac地址
Platform::String^ DrClient::GetMac()
{
	if (NULL != m_pAuth) {
		string strError = m_pAuth->getMac();
		wstring wstrError = TransWString(strError);
		return ref new String(wstrError.c_str());
	}
	return ref new String(L"");
}
// 获取假MAC（通过设备ID生成）
Platform::String^ DrClient::GetLocalMac()
{
	if (NULL != m_pAuth) {
		string strError = m_pAuth->getLocalMac();
		wstring wstrError = TransWString(strError);
		return ref new String(wstrError.c_str());
	}
	return ref new String(L"");
}
// 设置无线网络的SSID
Platform::Boolean DrClient::SetWlanSSID(Platform::String^ wlanSSID)
{
	string strWlanSSID = TransString(wlanSSID->Data());
	if (NULL != m_pAuth)
	{
		m_pAuth->SetSSID(strWlanSSID);
		return true;
	}
	else
	{
		return false;
	}
}

// 设置网关地址
Platform::Boolean DrClient::SetGatewayAddress(Platform::String^ gatewayAddr)
{
	string strGatewayAddr = TransString(gatewayAddr->Data());
	if (NULL != m_pAuth)
	{
		m_pAuth->SetGatewayAddress(strGatewayAddr);
		return true;
	}
	else
	{
		return false;
	}

}

// 获取网关地址
Platform::String^ DrClient::GetGatewayAddress()
{
	if (NULL != m_pAuth) {
		string strError = m_pAuth->getGatewayAddress();
		wstring wstrError = TransWString(strError);
		return ref new String(wstrError.c_str());
	}
	return  ref new String(L"");
}

// 获取登录状态（是否登录）
Platform::Boolean DrClient::GetLoginStatus()
{
	if (NULL != m_pAuth) {
		return m_pAuth->getLoginStatus();
	}
	return false;
}

// 获取流量
Platform::String^ DrClient::GetFluxStatus()
{
	if (NULL != m_pAuth) {
		string strError = m_pAuth->getFluxStatus();
		wstring wstrError = TransWString(strError);
		return ref new String(wstrError.c_str());
	}
	return ref new String(L"");
}

// 获取时长
Platform::String^ DrClient::GetTimeStatus()
{
	if (NULL != m_pAuth) {
		string strError = m_pAuth->getTimeStatus();
		wstring wstrError = TransWString(strError);
		return ref new String(wstrError.c_str());
	}
	return ref new String(L"");
}

// wstring转string（字符串转换）
string DrClient::TransString(const wstring& str)
{
	// 获取区域设置
	locale const loc("");
	wchar_t const* from = str.c_str();
	size_t const len = str.size();
	vector<char> buffer(len+1);
	
	// 字符转换为类型char对应的字符在本机字符集
	use_facet<ctype<wchar_t>>(loc).narrow(from, from+len, '_', &buffer[0]);
	return string(&buffer[0], &buffer[len]);
}

// string转wstring（字符串转换）
wstring DrClient::TransWString(const string& str)
{
	// 把std::string转化为wstring
	std::wstring wstr;
	wstr.resize(str.size()+1);
	size_t charsConverted;
	// 将多字节字符序列转换为相应的宽字符序列
	errno_t err = ::mbstowcs_s(&charsConverted, (wchar_t*)wstr.data(), wstr.size(), str.data(), str.size());
	wstr.pop_back();
	return wstr;
}
