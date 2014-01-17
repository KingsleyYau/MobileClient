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
	public static final String UPGRADEAPP_ACTION = ".UPGRADEAPP_ACTION";	//更新广播(界面处理)
	public static final String UPGRADEAPP_CANCEL_ACTION = ".UPGRADEAPP_CANCEL_ACTION";	//取消更新广播(通知栏处理)
	
	private Context mContext;
	// --------------------------自动更新代码--------------------------------//
	// 记录更新的sharedpreferences
	private String Sharedpreferences_UPGRADE = "sharedpreferences_upgrade";
	private String Sharedpreferences_UPGRADE_NEVER_KEY = "never_upgrade_version";
	
	private GroupReceiver mGroupReceiver;
	
	// 更新广播标识
	private boolean isNotifyUpgrade = false;
	// 下载器
//	private SmartFileDownloader loader;
	private DownloadTask downloadTask;
	// 通知栏变量
	private final int NF_ID = 1111;
	private Notification nf;
	private NotificationManager nm;
	// apk下载地址
	String upgradeurl;// =
						// "http://shouji.baidu.com/download/1426l/AppSearch_Android_1426l.apk";
	String apkname;// = upgradeurl.substring(upgradeurl.lastIndexOf("/"));
	String dir = Environment.getExternalStorageDirectory() + "/DrCOMWSDownload/";// 获取SDCard目录

	public AppUpdateManagement(Context c){
		mContext = c;
		
		initReceiver();
	}
	
	public void Destory(){
		mContext.unregisterReceiver(mGroupReceiver);
	}
	
	/**
	 * 用户选择"以后不提示"后
	 */
	public boolean updatenever(String version){
		SharedPreferences sharedata = mContext.getSharedPreferences(Sharedpreferences_UPGRADE, 0);
		String type = sharedata.getString(Sharedpreferences_UPGRADE_NEVER_KEY, "");
		if (type.equals(version)) {
			Log.i("zjj", "用户已设置不再提示更新");
			return true;
		}
		return false;
	}
	
	/**
	 * 取包名
	 * @return
	 */
	private String getPackagename(){
		return mContext.getPackageName();
	}
	
	/**
	 * 更新对话框
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

				// 取更新信息
				upgradeurl = item.url;
				apkname = upgradeurl.substring(upgradeurl.lastIndexOf("/") + 1);

				// Log.i("zjj", "upgradeurl:" + upgradeurl);
				// Log.i("zjj", "apkname:" + apkname);
				// Log.i("zjj", "本地路径:" + dir + "/" + apkname);

				// 判断手机是否装有SDCard
				if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
					download(upgradeurl, dir);
				} else {
					Toast.makeText(mContext, mContext.getResources().getString(R.string.no_SDcard), Toast.LENGTH_LONG).show();
				}

				// 通知栏
				nf = new Notification(R.drawable.icon, "", System.currentTimeMillis());
				nf.icon = R.drawable.icon;

				nf.contentView = new RemoteViews(mContext.getPackageName(), R.layout.upgrade_notification);
				nf.contentView.setProgressBar(R.id.ProgressBar01, 100, 0, false);
				//点击关闭
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
//	 * 处理下载中的信息
//	 */
//	private Handler handler = new Handler() {
//
//		@Override
//		// 信息
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
//				// 下载完成
//				if (size == mFileLenght) {
//					// Toast提示
//					Toast.makeText(mContext, mContext.getResources().getString(R.string.upgrade_notify_finishandsetup_text), Toast.LENGTH_LONG).show();
////					GlobalVariables.toastShow(mContext.getResources().getString(R.string.upgrade_notify_finishandsetup_text));
//					// Toast.makeText(DrPalmActivity.this,
//					// R.string.upgrade_notify_finishandsetup_text,
//					// Toast.LENGTH_LONG).show();
//					// 设置点击通知栏安装的事件
//					// 取消通知栏的X标志
//					nf.contentView.setTextViewText(R.id.upgrade_context_textview, mContext.getResources().getString(R.string.upgrade_notify_finish_text));
//					nf.contentView.setImageViewResource(R.id.upgrade_cancel_imgview, R.drawable.transparent);
//					// 打开安装包
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
	 * 处理下载中的信息
	 */
	private Handler handler = new Handler() {
		int size = 0;
		
		@Override
		// 信息
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case DownloadTask.DOWN:
				// int size = msg.getData().getInt("size");
				Log.i("zjj", "" + msg.obj);
				size =(Integer) msg.obj;
				nf.contentView.setProgressBar(R.id.ProgressBar01, 100, size, false);
				
				if (size == 100) {
					Log.i("zjj", "更新包下载完成");
					// Toast提示
					Toast.makeText(mContext, mContext.getResources().getString(R.string.upgrade_notify_finishandsetup_text), Toast.LENGTH_LONG).show();
					// 设置点击通知栏安装的事件
					// 取消通知栏的X标志
					nf.contentView.setTextViewText(R.id.upgrade_context_textview, mContext.getResources().getString(R.string.upgrade_notify_finish_text));
					nf.contentView.setImageViewResource(R.id.upgrade_cancel_imgview, R.drawable.transparent);
					// 打开安装包
					Log.i("zjj", "----安装包完整路径-----:" + "file:///sdcard" + dir + apkname);
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
	 *            下载的目标地址
	 * @param dir
	 *            存放的目标地址
	 */
	private void download(final String path, final String dir) {
//		new Thread(new Runnable() {
//			@Override
//			public void run() {
//				try {
//					loader = new SmartFileDownloader(mContext, path, dir, 1);
//					mFileLenght = loader.getFileSize();// 获取文件的长度
//					// downloadbar.setMax(length);//将文件的总长度设置为进度条的总长度
//					loader.download(new SmartDownloadProgressListener() {
//						@Override
//						public void onDownloadSize(int size) {// 可以实时得到文件下载的长度
//							Message msg = new Message();
//							msg.what = 1;
//							msg.getData().putInt("size", size);
//							handler.sendMessage(msg);
//						}
//					});
//				} catch (Exception e) {
//					Message msg = new Message();// 信息提示
//					msg.what = -1;
//					msg.getData().putString("error", mContext.getResources().getString(R.string.download_faile));// 如果下载错误，显示提示失败！
//					handler.sendMessage(msg);
//				}
//			}
//		}).start();// 开始
		
		Log.i("zjj", "更新包保存路径:" + dir + ",apk:" + apkname);
		if(downloadTask == null)
			downloadTask = new DownloadTask(handler, path, 3, null, dir);
		
		downloadTask.start();
	}
	
	/**
	 * 停止下载更新
	 */
	public void Stop(){
		if (downloadTask != null)
			downloadTask.StopDownload();
		
//		if (downloadTask != null) {
//			downloadTask.stop();
//			if (nm != null) {
//				// 取消通知栏
//				nm.cancel(NF_ID);
//				// 删除已下载文件
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
		filter.addAction(getPackagename() + UPGRADEAPP_ACTION); // 应用更新广播
		filter.addAction(getPackagename() + UPGRADEAPP_CANCEL_ACTION); // 取消更新下载广播
		mContext.registerReceiver(mGroupReceiver, filter);
	}
	
	/**
	 * *************** 广播接收 ***************
	 */
	private class GroupReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			try {
				String stringAction = intent.getAction();
				Bundle extras = intent.getExtras();
				Log.i("zjj", "接收PUSH广播:" + stringAction);
				
				if (stringAction.equals(getPackagename() + UPGRADEAPP_ACTION)) {
					// 提示过一次后,这次运行不再提示
					if (!isNotifyUpgrade) {
						isNotifyUpgrade = true;
						PushUpgradeAppItem item = null;
						if (extras.containsKey(PUSH_UPGRADE)) {
							item = (PushUpgradeAppItem) extras.getParcelable(PUSH_UPGRADE);
						}

						// 检测本地版本号
						PackageManager packageManager = mContext.getPackageManager();
						try {
							PackageInfo packInfo = packageManager.getPackageInfo(mContext.getPackageName(), 0);
							if (item.version.equals(packInfo.versionName)) {
								Log.i("zjj", "服务器返回版本号与本地相同,不提示");
								return;
							}
						} catch (NameNotFoundException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}

						// 用户选择"以后不提示"后
						if(updatenever(item.version)){
							return;
						}

						if (null != item) {
							showCustomMessageUpgrade(item);
						}
					}
				}else if (stringAction.equals(getPackagename() + UPGRADEAPP_CANCEL_ACTION)) {
					Log.i("zjj", "接收取消下载更新包广播");
					Stop();
				}
			}catch (Exception e) {
				// TODO: handle exception
			}
		}
	}
}
