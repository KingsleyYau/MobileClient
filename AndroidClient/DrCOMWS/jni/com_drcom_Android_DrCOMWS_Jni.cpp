/*
 * File         : com_drcom_Android_DrCOMWS_Jni.cpp
 * Date         : 2011-07-11
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : DrCOMWS Jni source
 */

#include "DrClientSrc/DrCOMAuth.h"
#include "com_drcom_Android_DrCOMWS_Jni.h"

static DrCOMAuth auth;

JNIEXPORT jint JNICALL Java_com_drcom_Android_DrCOMWS_Jni_httpLogin
  (JNIEnv *env, jobject thiz, jstring strGatewayAddress, jstring strUsername, jstring strPassword) {
	const char *pGatewayAddress = env->GetStringUTFChars(strGatewayAddress, 0);
	const char *pUsername = env->GetStringUTFChars(strUsername, 0);
	const char *pPassword = env->GetStringUTFChars(strPassword, 0);

	int iRet = auth.httpLogin(pGatewayAddress, pUsername, pPassword);

	env->ReleaseStringUTFChars(strGatewayAddress, pGatewayAddress);
	env->ReleaseStringUTFChars(strUsername, pUsername);
	env->ReleaseStringUTFChars(strPassword, pPassword);

	return iRet;
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    httpLoginCheck
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_drcom_Android_DrCOMWS_Jni_httpLoginCheck
  (JNIEnv *env, jobject thiz) {
	return 0;//auth.httpLoginCheck();
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    httpLoginAuth
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_drcom_Android_DrCOMWS_Jni_httpLoginAuth
  (JNIEnv *env, jobject thiz) {
	return 0;//auth.httpLoginAuth();
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    httpLogout
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_drcom_Android_DrCOMWS_Jni_httpLogout
  (JNIEnv *env, jobject thiz) {
	return auth.httpLogout();
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getGatewayAddress
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getGatewayAddress
  (JNIEnv *env, jobject thiz) {
	return env->NewStringUTF(auth.getGatewayAddress().c_str());
}
/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    setGatwatAddress
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_drcom_Android_DrCOMWS_Jni_setGatwatAddress
  (JNIEnv *env, jobject, jstring strGatewayAddress) {
	const char *pGatewayAddress = env->GetStringUTFChars(strGatewayAddress, 0);
	auth.SetGatewayAddress(pGatewayAddress);
	env->ReleaseStringUTFChars(strGatewayAddress, pGatewayAddress);

}
/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    setSSID
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_drcom_Android_DrCOMWS_Jni_setSSID
  (JNIEnv *env, jobject, jstring strSSID) {
	const char *pSSID = env->GetStringUTFChars(strSSID, 0);
	auth.SetSSID(pSSID);
	env->ReleaseStringUTFChars(strSSID, pSSID);
}
/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getLoginStatus
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getLoginStatus
  (JNIEnv *env, jobject thiz) {
	return auth.getLoginStatus();
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    httpStatus
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_drcom_Android_DrCOMWS_Jni_httpStatus
  (JNIEnv *env, jobject thiz) {
	return auth.httpStatus();
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getFluxStatus
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getFluxStatus
  (JNIEnv *env, jobject thiz) {
	return env->NewStringUTF(auth.getFluxStatus().c_str());
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getTimeStatus
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getTimeStatus
  (JNIEnv *env, jobject thiz) {
	return env->NewStringUTF(auth.getTimeStatus().c_str());
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getUnfineError
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getUnfineError
  (JNIEnv *env, jobject thiz) {
	//return env->NewStringUTF(auth.getUndefineError().c_str());

	jclass jCString;
	jmethodID jmIDStringInit;
	jbyteArray jbArray;
	jstring jsCodeType, jsString;

	//get java env
	jCString = env->FindClass("java/lang/String");
	jmIDStringInit = env->GetMethodID(jCString, "<init>", "([BLjava/lang/String;)V");

	//change ascII string to jstring
	jbArray = env->NewByteArray(auth.getUndefineError().length());
	env->SetByteArrayRegion(jbArray, 0, auth.getUndefineError().length(), (jbyte*)(auth.getUndefineError().c_str()));
	jsCodeType = env->NewStringUTF("gbk");
	jsString= (jstring)env->NewObject(jCString, jmIDStringInit, jbArray, jsCodeType);
	env->DeleteLocalRef(jbArray);

	return jsString;
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getXip
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getXip
  (JNIEnv *env, jobject thiz){
	return env->NewStringUTF(auth.getXip().c_str());
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getMac
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getMac
  (JNIEnv *env, jobject thiz) {
	return env->NewStringUTF(auth.getMac().c_str());
}

/*
 * Class:     com_drcom_Android_DrCOMWS_Jni
 * Method:    getData
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_drcom_Android_DrCOMWS_Jni_getData
  (JNIEnv *env, jobject thiz) {
	return env->NewStringUTF(auth.logString.c_str());
	//for debug
	//tcpSocket t;
	//return env->NewStringUTF(t.GetMacAddressList().c_str());
	//end debug
}


