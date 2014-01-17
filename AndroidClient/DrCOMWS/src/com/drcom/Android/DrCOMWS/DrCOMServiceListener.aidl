/*
 * File         : DrCOMServiceListener.aidl
 * Date         : 2011-06-17
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : Android DrCOM Service listener
 */

package com.drcom.Android.DrCOMWS;

interface DrCOMServiceListener {
	void onLoginResult(String strError);
	void onLogoutResult(String strError);
	void onRecvFlux(String strFlux);
	void onRecvTime(String strTime);
	void onStatus(String strMessage);
}