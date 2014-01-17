#pragma once

#include "IDrCOMAuth.h"

namespace DrClientDll
{

	// 注：此枚举与下面的定义必须一致，此枚举专门给C#调用
	public enum class ClientHandleStatus{
		DrClient_Success = DrClientHS_Success,						// Success
		DrClient_NetworkError = DrClientHS_NetworkError,			// Network connection interruption, check the network configuration please
		DrClient_UsingNAT = DrClientHS_UsingNAT,					// This account does not allow use NAT
		DrClient_NotFoundDrCOM = DrClientHS_NotFoundDrCOM,			// Can not find Dr.COM web protocol
		DrClient_AlreadyOnline = DrClientHS_AlreadyOnline,			// This equipment already online, do not need to log in
		DrClient_IpNotAllowLogin = DrClientHS_IpNotAllowLogin,		// The IP does not allow login base Dr.COM web technology
		DrClient_AccountNotAllow = DrClientHS_AccountNotAllow,		// The account does not allow login base Dr.COM web technology
		DrClient_NotAllowChangePwd = DrClientHS_NotAllowChangePwd,	// The account does not allow change password
		DrClient_InvalidAccountOrPwd = DrClientHS_InvalidAccountOrPwd,// Invalid account or password, please login again
		DrClient_AccountTieUp = DrClientHS_AccountTieUp,			// This account is tie up, please contact network administration
		DrClient_UsingOnAppointedIp = DrClientHS_UsingOnAppointedIp,	// This account use on appointed IP address only
		DrClient_ChargeOrFluxOver = DrClientHS_ChargeOrFluxOver,	// This account charge be overspend or flux over
		DrClient_AccountBreakOff = DrClientHS_AccountBreakOff,		// This account be break off
		DrClient_SystemBufferFull = DrClientHS_SystemBufferFull,	// System buffer full
		DrClient_TieUpCannotAmend = DrClientHS_TieUpCannotAmend,	// This account is tie up, can not amend
		DrClient_NewAndConfirmPwdDiffer = DrClientHS_NewAndConfirmPwdDiffer,	// The new and the confirm password are differ, can not amend
		DrClient_PwdAmendSuccessed = DrClientHS_PwdAmendSuccessed,	// The password amend successed
		DrClient_UsingAppointedMac = DrClientHS_UsingAppointedMac,	// This account use on appointed Mac address only
		DrClient_UndefineError = DrClientHS_UndefineError,			// Undefine error
	};

	//class IDrCOMAuth;
	public ref class DrClient sealed
	{
	public:
		DrClient();
		virtual ~DrClient();

	public:
		bool Init();
		void SetDeviceID(const Platform::Array<unsigned char>^ data);
		int Login(Platform::String^ gatewayAddr, Platform::String^ account, Platform::String^ password);
		int Logout();
		int GetStatus();
		int Stop();
		Platform::String^ GetUndefineError();
		Platform::String^ GetXip();
		Platform::String^ GetMac();
		Platform::String^ GetLocalMac();
		Platform::Boolean SetWlanSSID(Platform::String^ wlanSSID);
		Platform::String^ GetGatewayAddress();
		Platform::Boolean SetGatewayAddress(Platform::String^ gatewayAddr);
		Platform::Boolean GetLoginStatus();
		Platform::String^ GetFluxStatus();
		Platform::String^ GetTimeStatus();


	private:
		string TransString(const wstring& str);
		wstring TransWString(const string& str);

	private:
		IDrCOMAuth	*m_pAuth;
		
	};
}