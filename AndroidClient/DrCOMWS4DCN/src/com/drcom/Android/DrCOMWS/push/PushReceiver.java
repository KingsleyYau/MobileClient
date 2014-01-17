package com.drcom.Android.DrCOMWS.push;

import java.util.HashMap;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;

import org.json.JSONException;

import com.drcom.Android.DrCOMWS.DrCOMWS;
import com.drcom.Android.DrCOMWS.R;
import com.drcom.drpalm.Tool.PushManager;
import com.drcom.drpalm.Tool.service.DrServiceLog;
import com.drcom.drpalm.Tool.service.PreferenceManagement;
import com.drcom.drpalm.Tool.service.PushParse;
import com.drcom.drpalm.Tool.service.RequestParse;
import com.drcom.drpalm.objs.PushActionDefine;
import com.drcom.drpalm.objs.PushMessageItem;
import com.drcom.drpalm.objs.PushUpgradeAppItem;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

/**
 * ����PUSH���񷢳����Ĺ㲥
 * @author zhaojunjie
 *
 */
public class PushReceiver extends BroadcastReceiver{

	private static final int BASE_NOTIFICATION_ID = 10000;
	private static final int MAX_NOTIFICATION_COUNT = 10;
	private static int mCurNotificationId = BASE_NOTIFICATION_ID;

	public static NotificationManager mNotification;
	private ActivityManager mActivityManager;
	private String mPackageName;
	private Context mContext;
	private PreferenceManagement mPreferenceManagement;
	private ArrayBlockingQueue<Integer> mQueue = new ArrayBlockingQueue<Integer>(MAX_NOTIFICATION_COUNT);

	////////////////////////////////////////////////////////////////////////////////////
	/*
	 *  �����־
	 */
	static final String PUSH_LOG_SAVE_PATH = "/sdcard/DrpalmPushLog/";
	static final String PUSH_LOG_NAME = "PushLog";
	static final long TIME_FOR_CLEAN_PUSH_LOG = 10*24*3600*1000;

	synchronized private void writeLog(String strLog) {
		//DrServiceLog.clearInvalidLog(PUSH_LOG_SAVE_PATH, TIME_FOR_CLEAN_PUSH_LOG);
		DrServiceLog.writeLog(PUSH_LOG_SAVE_PATH, PUSH_LOG_NAME, strLog);
	}

	@Override
	public void onReceive(Context context, Intent intent){
		mContext = context;
		mPackageName = context.getPackageName();
		mPreferenceManagement = PreferenceManagement.getInstance(context);
		mActivityManager = (ActivityManager) context.getSystemService(android.content.Context.ACTIVITY_SERVICE);
		mNotification = (NotificationManager)context.getSystemService(android.content.Context.NOTIFICATION_SERVICE);
		String stringAction = intent.getAction();
		Bundle extras = intent.getExtras();
		// ���͹㲥
		if(stringAction.equals(mContext.getPackageName() + PushActionDefine.PUSH_GETMESSAGE_ACTION)){	//��ӦDrPalmPushService 327��
			String pushjsonstr = "";
			if(extras.containsKey(PushActionDefine.PUSH_MESSAGE)){
				pushjsonstr = extras.getString(PushActionDefine.PUSH_MESSAGE);
			}
			if("" != pushjsonstr){
				String msgLog = "�յ����͹㲥:" + pushjsonstr;
				writeLog(msgLog);

				//����JSON
				MyPushMessageItem item = null;
				try {
					RequestParse parse = new RequestParse(pushjsonstr);
					HashMap<String, Object> map;
					map = parse.getHashMap();
					PushJsonParse pushParse = new PushJsonParse(map);
					item = pushParse.parseMessageBody();
				} catch (JSONException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				if(item !=null){
					boolean bSound = item.sound.length()>1?true:false;
					//boolean bSound = ("" != item.sound?true:false);
					boolean bVibrate = (1 == item.etype?true:false);

					// ȥ���ɵ�֪ͨ����Ϣ
					if(mQueue.size() >= MAX_NOTIFICATION_COUNT) {
						Integer notificationId = mQueue.poll();
						mNotification.cancel(notificationId);
					}

					mCurNotificationId = BASE_NOTIFICATION_ID + ++mCurNotificationId % MAX_NOTIFICATION_COUNT;
					showNotification(R.drawable.icon, item.alert,
							mContext.getString(R.string.app_name), item.alert,
							bVibrate, bSound, false, mCurNotificationId, item.badge);
					// ��¼�²������Ϣ
					mQueue.add(mCurNotificationId);
					Intent uncountIntent = new Intent(mContext.getPackageName() + PushActionDefine.UNGETCOUNT_ACTION);
					uncountIntent.putExtra(mContext.getPackageName() + PushActionDefine.UNGETCOUNT, String.valueOf(item.badge));
					context.sendBroadcast(uncountIntent);
				}
			}
		}
		// �����㲥
		else if (stringAction.equals(mContext.getPackageName() + PushActionDefine.PUSH_UPGRADEAPP_ACTION)){	//��ӦDrPalmPushService 335��
			Log.i("zjj", "�յ������㲥");
//			showNotification(R.drawable.icon, "�����㲥",
//					mContext.getString(R.string.app_name), "�����㲥",
//					true, true, false, mCurNotificationId, 0);
			
			PushUpgradeAppItem item = null;
			if(extras.containsKey(PushActionDefine.PUSH_UPGRADE)){
				item = (PushUpgradeAppItem)extras.getParcelable(PushActionDefine.PUSH_UPGRADE);
			}
			if(null != item){
				Log.i("zjj", "ת�������㲥");
				//ת���㲥
				Intent upgradeIntent = new Intent(mContext.getPackageName() + PushActionDefine.UPGRADEAPP_ACTION);
				upgradeIntent.putExtra(PushActionDefine.PUSH_UPGRADE, item);
				context.sendBroadcast(upgradeIntent);
				
				
				//���PUSH����
		    	PushManager.unbindService();
			}
		}
	}

	/**
	 * PUSH֪ͨ��
	 * @param icon
	 * @param tickertext
	 * @param title
	 * @param content
	 * @param isVibrate
	 * @param isSound
	 * @param isAutoCancel
	 * @param notificationId
	 * @param messagecount
	 */
	public void showNotification(int icon,String tickertext,String title,String content,
    		boolean isVibrate, boolean isSound, boolean isAutoCancel, int notificationId, int messagecount){
    	Notification notification= new Notification(icon,tickertext,System.currentTimeMillis());
    	
    	//����ر�
    	notification.flags = Notification.FLAG_AUTO_CANCEL;	
    	
    	//�Ƿ���
    	if(isVibrate){
    		notification.defaults |= Notification.DEFAULT_VIBRATE;
    		long[] vibrate = {0,100,200,300};
    		notification.vibrate = vibrate;
    	}
    	
    	//�Ƿ�������
    	if(isSound){
    		notification.defaults |= Notification.DEFAULT_SOUND;
    	}
    	
    	//************
    	//************
    	//************
    	//���֪ͨ�����ĸ�ACTIVITY
    	//************
    	//************
    	//************
    	Intent intent = new Intent(mContext, DrCOMWS.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Context context = mContext.getApplicationContext();
        PendingIntent pt = PendingIntent.getActivity(context, 0, intent, 0);
    	
    	notification.setLatestEventInfo(mContext,title,content,pt);

    	mNotification.notify(notificationId,notification);
    }
}




