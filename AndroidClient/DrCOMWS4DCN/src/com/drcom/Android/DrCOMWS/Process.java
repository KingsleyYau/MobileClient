/*
 * File         : Process.java
 * Date         : 2011-06-17
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : Android ProcessBar
 */

package com.drcom.Android.DrCOMWS;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;

public class Process extends Activity {
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	    setContentView(R.layout.process);
	}
	public boolean onKeyDown(int keyCode, KeyEvent event) {
	    if (keyCode == KeyEvent.KEYCODE_BACK) {
	        return true;
	    }
	    return super.onKeyDown(keyCode, event);
	}
}
