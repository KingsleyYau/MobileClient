package com.drcom.Android.DrCOMWS;

public class Jni {
	//minus for error
	public native int httpLogin(String strGatewayAddress, String strUsername, String strPassword);
	public native int httpLoginCheck();
	public native int httpLoginAuth();

	public native int httpLogout();

	public native String getGatewayAddress();
	public native void setGatwatAddress(String strGatewayAddress);
	public native boolean getLoginStatus();
	public native boolean setSSID(String strSSID);

	public native boolean httpStatus();
	public native String getFluxStatus();
	public native String getTimeStatus();

	public native String getUnfineError();
	public native String getXip();
	public native String getMac();
	public native String getData();
}
