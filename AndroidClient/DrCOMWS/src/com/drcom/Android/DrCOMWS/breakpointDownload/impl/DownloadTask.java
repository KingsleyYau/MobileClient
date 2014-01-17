package com.drcom.Android.DrCOMWS.breakpointDownload.impl;

import java.io.File;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.List;

import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

/**
 * �����ļ�,������,ÿ1�뷢��һ�����ؽ���,msg.what=1;obj=percentage;
 * 
 * @author xpf
 * 
 */
public class DownloadTask extends Thread {
	public static final int DOWN = 1;		//�������سɹ�
	public static final int NOTDOWN = -1;	//����ʧ��
	public static final int STOP = 2;		//���ر�ֹͣ
	
	private int blockSize, downloadSizeMore;
	private int threadNum;
	private int fileSize, downloadedSize;
	private String urlStr, fileName;
	private String dowloadDir;
	private Handler handler;
	private boolean stop = false;
	
//	private List<FileDownloadThread> listThreads = new ArrayList<FileDownloadThread>();
	private FileDownloadThread[] fds;

	/**
	 * 
	 * @param handler
	 *            ÿ1�뷢��һ�����ؽ���
	 * @param urlStr
	 *            ���ص�url
	 * @param threadNum
	 *            �����Ľ�����,Ĭ��Ϊ3
	 * @param fileName
	 *            ������ļ���,if null save with the last string after '/'
	 */
	public DownloadTask(Handler handler, String urlStr, int threadNum, String fileName, String savaDirector) {
		this.urlStr = urlStr;
		if (threadNum <= 0) {
			this.threadNum = 1;
		} else {
			this.threadNum = threadNum;
		}
		this.handler = handler;

		// ����Ŀ¼
		if ("".equals(savaDirector) || null == savaDirector) {
			dowloadDir = Environment.getExternalStorageDirectory() + "/Ebabydownload/";
		} else {
			dowloadDir = Environment.getExternalStorageDirectory() + "/" + savaDirector + "/";
		}
		File file = new File(dowloadDir);
		// ��������Ŀ¼
		if (!file.exists()) {
			file.mkdirs();
		}
		// ȥ���ļ����е�"/"
		if ("".equals(fileName) || null == fileName) {
			// ��������ļ���Ϊ�����ȡUrlβΪ�ļ���
			this.fileName = urlStr.substring(urlStr.lastIndexOf("/") + 1, urlStr.length());
		} else {
			this.fileName = fileName;
		}
		this.fileName = dowloadDir + this.fileName;
		Log.i("xpf", "filename-"+this.fileName);
	}

	public void StopDownload(){
		stop = true;
		
		for(int i = 0 ; i < threadNum;i++){
			fds[i].StopDownload();
		}
		
		Message msg = handler.obtainMessage();
		msg.what = STOP;
		msg.obj = "dowload stoped";
		handler.sendMessage(msg);
	}
	
	@Override
	public void run() {
//		FileDownloadThread[] fds = new FileDownloadThread[threadNum];
		fds = new FileDownloadThread[threadNum];
		try {
			URL url = new URL(urlStr);
			URLConnection conn = url.openConnection();
			// ��ȡ�����ļ����ܴ�С
			fileSize = conn.getContentLength();
			Log.i("xpf", "filelenght="+fileSize);
			// ����ÿ���߳�Ҫ���ص�������
			blockSize = fileSize / threadNum;
			// ���������ٷֱȼ������
			downloadSizeMore = (fileSize % threadNum);
			File file = new File(fileName);
			for (int i = 0; i < threadNum-1; i++) {
				// �����̣߳��ֱ������Լ���Ҫ���صĲ���
				FileDownloadThread fdt = new FileDownloadThread(url, file, i * blockSize, (i + 1) * blockSize );
				fdt.setName("Thread" + i);
				fdt.start();
				fds[i] = fdt;
			}
			
			//���ش����������Ĳ���
			FileDownloadThread fdt = new FileDownloadThread(url, file,(threadNum-1) * blockSize, ((threadNum-1) + 1) * blockSize +downloadSizeMore);
			fdt.setName("Thread" + (threadNum-1));
			fdt.start();
			fds[threadNum-1] = fdt;
			
			boolean finished = false;
			if (fileSize <= 0) {
				Message msg = handler.obtainMessage();
				msg.what = NOTDOWN;
				msg.obj = "dowload failed";
				handler.sendMessage(msg);
				finished = true;
			}
			while (!finished && !stop) {
				// �Ȱ������������㶨
				downloadedSize = downloadSizeMore;
				finished = true;
				for (int i = 0; i < fds.length; i++) {
					downloadedSize += fds[i].getDownloadSize();
					if (!fds[i].isFinished()) {
						finished = false;
					}
				}
				// ֪ͨhandlerȥ������ͼ���
				Message msg = handler.obtainMessage();
				msg.what = DOWN;
				
				msg.obj = (Double.valueOf((downloadedSize * 1.0 / fileSize * 100))).intValue();
				Log.i("xpf", "downloadedSize="+downloadedSize+"  fileSize="+fileSize+" p="+Double.valueOf((downloadedSize * 1.0 / fileSize * 100)));
				if (downloadedSize == fileSize) {
					finished = true;
					msg.obj=100;
				}
				handler.sendMessage(msg);
			
				// ��Ϣ1����ٶ�ȡ���ؽ���
				sleep(1000);
			}
		} catch (Exception e) {
			Log.i("xpf", "url eeror");
			Message msg = handler.obtainMessage();
			msg.what = NOTDOWN;
			msg.obj = "dowload failed";
			handler.sendMessage(msg);
		}

	}

}
