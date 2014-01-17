package com.drcom.Android.DrCOMWS.breakpointDownload.impl;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.net.URL;
import java.net.URLConnection;

import android.util.Log;

public class FileDownloadThread extends Thread {
	private static final int BUFFER_SIZE = 1024;
	private URL url;
	private File file;
	private int startPosition;
	private int endPosition;
	private int curPosition;
	// ���ڱ�ʶ��ǰ�߳��Ƿ��������
	private boolean finished = false;
	private int downloadSize,tatol=0;
	//ֹͣ����
	private boolean stop = false;

	public FileDownloadThread(URL url, File file, int startPosition, int endPosition) {
		this.url = url;
		this.file = file;
		this.startPosition = startPosition;
		this.curPosition = startPosition;
		this.endPosition = endPosition;
		this.downloadSize = 0;
	}

	@Override
	public void run() {
		BufferedInputStream bis = null;
		RandomAccessFile fos = null;
		byte[] buf = new byte[BUFFER_SIZE];
		URLConnection con = null;
		try {
			con = url.openConnection();
			con.setAllowUserInteraction(true);
			// ���õ�ǰ�߳����ص���㣬�յ�
			Log.i("xpf", "startPosition=" + startPosition + " endPosition" + endPosition);
			con.setRequestProperty("Range", "bytes=" + startPosition + "-" + endPosition);
			// ʹ��java�е�RandomAccessFile���ļ����������д����
			fos = new RandomAccessFile(file, "rw");
			Log.i("xpf", "file.length=" + file.length());
			// ���ÿ�ʼд�ļ���λ��
			fos.seek(startPosition);
			bis = new BufferedInputStream(con.getInputStream());
			// ��ʼѭ����������ʽ��д�ļ�
			while (curPosition < endPosition) {
				//ֹͣ
				if(stop)
					break;
				
				int len = bis.read(buf, 0, BUFFER_SIZE);
				if (len == -1) {
					break;
				}
				if (curPosition + len < endPosition) {
					fos.write(buf, 0, len);
					curPosition = curPosition + len;
					downloadSize += len;
				} else {
					fos.write(buf, 0, endPosition - curPosition);
					this.finished = true;
					downloadSize += endPosition - curPosition ;
					curPosition = endPosition;
					break;
				}
			}

			// ���������Ϊtrue
			this.finished = true;
			bis.close();
			fos.close();
		} catch (IOException e) {
			Log.d(getName() + "Error:", e.getMessage());
		}
	}

	public boolean isFinished() {
		return finished;
	}

	public int getDownloadSize() {
		return downloadSize;
	}
	
	public void StopDownload(){
		stop = true;
	}
}
