package com.drcom.Android.DrCOMWS;

import android.app.Dialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.RemoteViews;
import android.widget.TextView;
import android.widget.Toast;

import com.drcom.Android.DrCOMWS.breakpointDownload.impl.DownloadTask;
import com.drcom.drpalm.objs.PushUpgradeAppItem;


public class AppUpdateManagement {
	public static final String PUSH_UPGRADE = "push_upgrade";
	public static final String UPGRADEAPP_ACTION = ".UPGRADEAPP_ACTION";	//���¹㲥(���洦��)
	public static final String UPGRADEAPP_CANCEL_ACTION = ".UPGRADEAPP_CANCEL_ACTION";	//ȡ�����¹㲥(֪ͨ������)
	
	private Context mContext;
	// --------------------------�Զ����´���--------------------------------//
	// ��¼���µ�sharedpreferences
	private String Sharedpreferences_UPGRADE = "sharedpreferences_upgrade";
	private String Sharedpreferences_UPGRADE_NEVER_KEY = "never_upgrade_version";
	
	private GroupReceiver mGroupReceiver;
	
	// ���¹㲥��ʶ
	private boolean isNotifyUpgrade = false;
	// ������
//	private SmartFileDownloader loader;
	private DownloadTask downloadTask;
	// ֪ͨ������
	private final int NF_ID = 1111;
	private Notification nf;
	private NotificationManager nm;
	// apk���ص�ַ
	String upgradeurl;// =
						// "http://shouji.baidu.com/download/1426l/AppSearch_Android_1426l.apk";
	String apkname;// = upgradeurl.substring(upgradeurl.lastIndexOf("/"));
	String dir = Environment.getExternalStorageDirectory() + "/DrCOMWSDownload/";// ��ȡSDCardĿ¼

	public AppUpdateManagement(Context c){
		mContext = c;
		
		initReceiver();
	}
	
	public void Destory(){
		mContext.unregisterReceiver(mGroupReceiver);
	}
	
	/**
	 * �û�ѡ��"�Ժ���ʾ"��
	 */
	public boolean updatenever(String version){
		SharedPreferences sharedata = mContext.getSharedPreferences(Sharedpreferences_UPGRADE, 0);
		String type = sharedata.getString(Sharedpreferences_UPGRADE_NEVER_KEY, "");
		if (type.equals(version)) {
			Log.i("zjj", "�û������ò�����ʾ����");
			return true;
		}
		return false;
	}
	
	/**
	 * ȡ����
	 * @return
	 */
	private String getPackagename(){
		return mContext.getPackageName();
	}
	
	/**
	 * ���¶Ի���
	 * 
	 * @param pTitle
	 * @param pMsg
	 */
	public void showCustomMessageUpgrade(final PushUpgradeAppItem item) {
		final Dialog lDialog = new Dialog(mContext, android.R.style.Theme_Translucent_NoTitleBar);
		lDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
		lDialog.setContentView(R.layout.r_upgradedialogview);
		((TextView) lDialog.findViewById(R.id.dialog_title)).setText(item.version + mContext.getResources().getString(R.string.upgrade_title));
		((TextView) lDialog.findViewById(R.id.dialog_message)).setText(item.des);
		Button btn_ok = (Button) lDialog.findViewById(R.id.ok);
		btn_ok.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// write your code to do things after users clicks OK

				// ȡ������Ϣ
				upgradeurl = item.url;
				apkname = upgradeurl.substring(upgradeurl.lastIndexOf("/") + 1);

				// Log.i("zjj", "upgradeurl:" + upgradeurl);
				// Log.i("zjj", "apkname:" + apkname);
				// Log.i("zjj", "����·��:" + dir + "/" + apkname);

				// �ж��ֻ��Ƿ�װ��SDCard
				if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
					download(upgradeurl, dir);
				} else {
					Toast.makeText(mContext, mContext.getResources().getString(R.string.no_SDcard), Toast.LENGTH_LONG).show();
				}

				// ֪ͨ��
				nf = new Notification(R.drawable.icon, "", System.currentTimeMillis());
				nf.icon = R.drawable.icon;

				nf.contentView = new RemoteViews(mContext.getPackageName(), R.layout.upgrade_notification);
				nf.contentView.setProgressBar(R.id.ProgressBar01, 100, 0, false);
				//����ر�
		    	nf.flags = Notification.FLAG_AUTO_CANCEL;	
		    	
				Intent upgradecancelIntent = new Intent(getPackagename() + UPGRADEAPP_CANCEL_ACTION);
				nf.contentIntent = PendingIntent.getBroadcast(mContext, 0, upgradecancelIntent, 0);
				// nf.contentIntent=PendingIntent.getActivity( this, 0, new
				// Intent(this,remoteview.class) ,0);
				nm = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);

				lDialog.dismiss();
			}
		});
		Button btn_never = (Button) lDialog.findViewById(R.id.dont_notice_again);
		btn_never.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Editor sharedata = mContext.getSharedPreferences(Sharedpreferences_UPGRADE, 0).edit();
				sharedata.putString(Sharedpreferences_UPGRADE_NEVER_KEY, item.version);
				sharedata.commit();
				lDialog.dismiss();
			}
		});
		Button btn_cancel = (Button) lDialog.findViewById(R.id.cancel);
		btn_cancel.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				lDialog.dismiss();
			}
		});
		lDialog.show();

	}

//	/**
//	 * ���������е���Ϣ
//	 */
//	private Handler handler = new Handler() {
//
//		@Override
//		// ��Ϣ
//		public void handleMessage(Message msg) {
//			switch (msg.what) {
//			case 1:
//				int size = msg.getData().getInt("size");
//				// downloadbar.setProgress(size);
//				// float result = (float)downloadbar.getProgress()/
//				// (float)downloadbar.getMax();
//				// int p = (int)(result*100);
//				// resultView.setText(p+"%");
//				// if(downloadbar.getProgress()==downloadbar.getMax())
//				// Toast.makeText(SmartDownloadActivity.this, R.string.success,
//				// 0).show();
//				nf.contentView.setProgressBar(R.id.ProgressBar01, mFileLenght, size, false);
//				// �������
//				if (size == mFileLenght) {
//					// Toast��ʾ
//					Toast.makeText(mContext, mContext.getResources().getString(R.string.upgrade_notify_finishandsetup_text), Toast.LENGTH_LONG).show();
////					GlobalVariables.toastShow(mContext.getResources().getString(R.string.upgrade_notify_finishandsetup_text));
//					// Toast.makeText(DrPalmActivity.this,
//					// R.string.upgrade_notify_finishandsetup_text,
//					// Toast.LENGTH_LONG).show();
//					// ���õ��֪ͨ����װ���¼�
//					// ȡ��֪ͨ����X��־
//					nf.contentView.setTextViewText(R.id.upgrade_context_textview, mContext.getResources().getString(R.string.upgrade_notify_finish_text));
//					nf.contentView.setImageViewResource(R.id.upgrade_cancel_imgview, R.drawable.transparent);
//					// �򿪰�װ��
//					Intent i = new Intent(Intent.ACTION_VIEW);
//					i.setDataAndType(Uri.parse("file://" + dir + "/" + apkname), "application/vnd.android.package-archive");
//					nf.contentIntent = PendingIntent.getActivity(mContext, 0, i, 0);
//					nm.cancel(NF_ID);
//				}
//				nm.notify(NF_ID, nf);
//				break;
//
//			case -1:
//				Toast.makeText(mContext, msg.getData().getString("error"), Toast.LENGTH_LONG).show();
////				GlobalVariables.toastShow(msg.getData().getString("error"));
//				// Toast.makeText(DrPalmActivity.this,
//				// msg.getData().getString("error"), 1).show();
//				break;
//			}
//
//		}
//	};
	
	/**
	 * ���������е���Ϣ
	 */
	private Handler handler = new Handler() {
		int size = 0;
		
		@Override
		// ��Ϣ
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case DownloadTask.DOWN:
				// int size = msg.getData().getInt("size");
				Log.i("zjj", "" + msg.obj);
				size =(Integer) msg.obj;
				nf.contentView.setProgressBar(R.id.ProgressBar01, 100, size, false);
				
				if (size == 100) {
					Log.i("zjj", "���°��������");
					// Toast��ʾ
					Toast.makeText(mContext, mContext.getResources().getString(R.string.upgrade_notify_finishandsetup_text), Toast.LENGTH_LONG).show();
					// ���õ��֪ͨ����װ���¼�
					// ȡ��֪ͨ����X��־
					nf.contentView.setTextViewText(R.id.upgrade_context_textview, mContext.getResources().getString(R.string.upgrade_notify_finish_text));
					nf.contentView.setImageViewResource(R.id.upgrade_cancel_imgview, R.drawable.transparent);
					// �򿪰�װ��
					Log.i("zjj", "----��װ������·��-----:" + "file:///sdcard" + dir + apkname);
					Intent i = new Intent(Intent.ACTION_VIEW);
					i.setDataAndType(Uri.parse("file:///sdcard" + dir + apkname), "application/vnd.android.package-archive");
					nf.contentIntent = PendingIntent.getActivity(mContext, 0, i, 0);
					nm.cancel(NF_ID);
				}
				nm.notify(NF_ID, nf);
				break;

			case DownloadTask.NOTDOWN:
				// GlobalVariables.toastShow(msg.getData().getString("error"));
				// Toast.makeText(DrPalmActivity.this,
				// msg.getData().getString("error"), 1).show();
				break;
			}

		}
	};

	/**
	 * 
	 * @param path
	 *            ���ص�Ŀ���ַ
	 * @param dir
	 *            ��ŵ�Ŀ���ַ
	 */
	private void download(final String path, final String dir) {
//		new Thread(new Runnable() {
//			@Override
//			public void run() {
//				try {
//					loader = new SmartFileDownloader(mContext, path, dir, 1);
//					mFileLenght = loader.getFileSize();// ��ȡ�ļ��ĳ���
//					// downloadbar.setMax(length);//���ļ����ܳ�������Ϊ���������ܳ���
//					loader.download(new SmartDownloadProgressListener() {
//						@Override
//						public void onDownloadSize(int size) {// ����ʵʱ�õ��ļ����صĳ���
//							Message msg = new Message();
//							msg.what = 1;
//							msg.getData().putInt("size", size);
//							handler.sendMessage(msg);
//						}
//					});
//				} catch (Exception e) {
//					Message msg = new Message();// ��Ϣ��ʾ
//					msg.what = -1;
//					msg.getData().putString("error", mContext.getResources().getString(R.string.download_faile));// ������ش�����ʾ��ʾʧ�ܣ�
//					handler.sendMessage(msg);
//				}
//			}
//		}).start();// ��ʼ
		
		Log.i("zjj", "���°�����·��:" + dir + ",apk:" + apkname);
		if(downloadTask == null)
			downloadTask = new DownloadTask(handler, path, 3, null, dir);
		
		downloadTask.start();
	}
	
	/**
	 * ֹͣ���ظ���
	 */
	public void Stop(){
		if (downloadTask != null)
			downloadTask.StopDownload();
		
//		if (downloadTask != null) {
//			downloadTask.stop();
//			if (nm != null) {
//				// ȡ��֪ͨ��
//				nm.cancel(NF_ID);
//				// ɾ���������ļ�
//				File delFile = new File(dir + "/" + apkname);
//				if (delFile.exists()) {
//					delFile.delete();
//				}
//
//			}
//		}
	}
	
	/**
	 * initialize receiver
	 */
	private void initReceiver() {
		mGroupReceiver = new GroupReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(getPackagename() + UPGRADEAPP_ACTION); // Ӧ�ø��¹㲥
		filter.addAction(getPackagename() + UPGRADEAPP_CANCEL_ACTION); // ȡ���������ع㲥
		mContext.registerReceiver(mGroupReceiver, filter);
	}
	
	/**
	 * *************** �㲥���� ***************
	 */
	private class GroupReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			try {
				String stringAction = intent.getAction();
				Bundle extras = intent.getExtras();
				Log.i("zjj", "����PUSH�㲥:" + stringAction);
				
				if (stringAction.equals(getPackagename() + UPGRADEAPP_ACTION)) {
					// ��ʾ��һ�κ�,������в�����ʾ
					if (!isNotifyUpgrade) {
						isNotifyUpgrade = true;
						PushUpgradeAppItem item = null;
						if (extras.containsKey(PUSH_UPGRADE)) {
							item = (PushUpgradeAppItem) extras.getParcelable(PUSH_UPGRADE);
						}

						// ��Ȿ�ذ汾��
						PackageManager packageManager = mContext.getPackageManager();
						try {
							PackageInfo packInfo = packageManager.getPackageInfo(mContext.getPackageName(), 0);
							if (item.version.equals(packInfo.versionName)) {
								Log.i("zjj", "���������ذ汾���뱾����ͬ,����ʾ");
								return;
							}
						} catch (NameNotFoundException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}

						// �û�ѡ��"�Ժ���ʾ"��
						if(updatenever(item.version)){
							return;
						}

						if (null != item) {
							showCustomMessageUpgrade(item);
						}
					}
				}else if (stringAction.equals(getPackagename() + UPGRADEAPP_CANCEL_ACTION)) {
					Log.i("zjj", "����ȡ�����ظ��°��㲥");
					Stop();
				}
			}catch (Exception e) {
				// TODO: handle exception
			}
		}
	}
}
