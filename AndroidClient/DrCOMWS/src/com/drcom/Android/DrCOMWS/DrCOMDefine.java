/*
 * File         : DrCOMDefine.java
 * Date         : 2011-06-21
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : Android DrCOM define
 */

package com.drcom.Android.DrCOMWS;

public class DrCOMDefine {
	final static int iLoginResult = 1;
	final static int iLogoutResult = 2;
	final static int iFluxResult = 3;
	final static int iTimeResult = 4;
	final static int iStatusResult = 5;

	final static int iFAIL = -1;
	final static int iSUCCESS = 1;
	final static int iCHECK = 2;

	final static String DrCOMClientWS = "DrCOMClientWS";
	final static String DrCOMUsername = "DrCOMUsername";
	final static String DrCOMPass = "DrCOMPass";
	final static String DrCOMUrl = "DrCOMUrl";
	final static String DrCOMRememberPass = "DrCOMRememberPass";
	final static String DrCOMSignIn = "DrCOMSignIn";
	final static String DrCOMYES = "YES";
	final static String DrCOMYNO = "NO";

	final static int Network_connection_interruption_check_the_network_configuration_please = -101;
	final static int This_account_does_not_allow_use_NAT									= -102;
	final static int Can_not_find_Dr_COM_web_protocol										= -103;
	final static int This_equipment_already_online_do_not_need_to_log_in					= -104;
	final static int The_IP_does_not_allow_login_base_Dr_COM_web_technology					= -105;
	final static int The_account_does_not_allow_login_base_Dr_COM_web_technology			= -106;
	final static int The_account_does_not_allow_change_password								= -107;
	final static int Invalid_account_or_password_please_login_again							= -108;
	final static int This_account_is_tie_up_please_contact_network_administration_IP_MAC	= -109;
	final static int This_account_use_on_appointed_address_only_IP							= -110;
	final static int This_account_charge_be_overspend_or_flux_over							= -111;
	final static int This_account_be_break_off												= -112;
	final static int System_buffer_full														= -113;
	final static int This_account_is_tie_up_can_not_amend									= -114;
	final static int The_new_and_the_confirm_password_are_differ_can_not_amend				= -115;
	final static int The_password_amend_successed											= -116;
	final static int This_account_use_on_appointed_address_only_MAC							= -117;

	final static int Undefine_error															= -130;
}
