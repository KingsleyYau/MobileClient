package com.drcom.Android.DrCOMWS.push;

import java.io.InputStream;
import java.util.HashMap;

import com.drcom.drpalm.objs.PushMessageItem;

/**
 *  解释PUSH接口传送回来的的JSON
 * @author Administrator
 *
 */
public class PushJsonParse {
	protected InputStream mStream = null;
	protected HashMap<String, Object> mHashMap = null;
	public PushJsonParse(HashMap<String, Object> map){
		mHashMap = map;
	}

	public MyPushMessageItem parseMessageBody() {
		MyPushMessageItem item = new MyPushMessageItem();
		if(mHashMap.containsKey("body")) {
			try{
				HashMap<String,Object> bodyMap = ((HashMap<String,Object>)mHashMap.get("body"));
				if(bodyMap.containsKey("aps")){
					HashMap<String,Object> apsMap = ((HashMap<String,Object>)bodyMap.get("aps"));
					if(apsMap.containsKey("alert")){
						item.alert = (String)apsMap.get("alert");
					}
					if(apsMap.containsKey("badge")){
						 String value = (String)apsMap.get("badge");
						 item.badge = Integer.valueOf(value);
					}
					if(apsMap.containsKey("sound")){
						item.sound = (String)apsMap.get("sound");
					}
				}
				if(bodyMap.containsKey("etype")){
					String value = (String)bodyMap.get("etype");
					item.etype = Integer.valueOf(value);
				}
			}catch(Exception e){

			}

		}
		return item;
	}
}
