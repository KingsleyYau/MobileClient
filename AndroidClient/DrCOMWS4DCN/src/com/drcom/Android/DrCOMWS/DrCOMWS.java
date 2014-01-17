/*
 * File         : DrCOMWS.java
 * Date         : 2011-06-17
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : Android DrCOMWS
 */

package com.drcom.Android.DrCOMWS;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;

import com.drcom.drpalm.Tool.PushManager;
import com.drcom.drpalm.Tool.service.ConnectPushCallback;
import com.drcom.drpalm.Tool.service.DrServiceLog;

public class DrCOMWS extends Activity {
	private TextView m_txUsername;
	private TextView m_txPassword;
	private EditText m_edUsername;
	private EditText m_edPassword;
	private TextView m_txTime;
	private TextView m_txMin;
	private TextView m_txFlux;
	private TextView m_txMByte;
	private TextView m_txVersion;
	private CheckBox m_cbKeepPassword;
	private CheckBox m_cbSign;
	private Button m_btOK;
	private AlertDialog m_msgBox;
	private AlertDialog m_msgBoxWarm;
	private ProgressDialog progressDlg; 

	private String m_strUsername = new String("");
	private String m_strPassword = new String("");
	private String m_strMsg = new String("");
	private String m_strGWAddress = new String("");
	private boolean mConnectStatus = false;		//连接状态

	private DrCOMServiceInterface m_Service = null;
	private Intent intentDrCOMService = new Intent();
	private Intent intentProcess = new Intent();
	private SharedPreferences m_spfConf = null;
	private AppUpdateManagement mAppUpdateManagement;

	private Handler mHandler = new Handler(){
        public void handleMessage(Message msg) {
        	String strMsg = msg.getData().getString(Integer.toString(msg.what));
            switch (msg.what) {
	            case DrCOMDefine.iLoginResult:
	            	onResultMsg(true, strMsg);
	                break;
	            case DrCOMDefine.iLogoutResult:
	            	onResultMsg(false, strMsg);
	            	break;
	            case DrCOMDefine.iTimeResult:
	            	onResultInfo(true, strMsg);
	            	break;
	            case DrCOMDefine.iFluxResult:
	            	onResultInfo(false, strMsg);
	            	break;
	            case DrCOMDefine.iStatusResult:
	            	onResultStatusBreak(strMsg);
	            	break;
            }
        };
    };

	private ServiceConnection m_Connection = new ServiceConnection() {
        public void onServiceDisconnected(ComponentName name) {
            m_Service = null;
        }

        public void onServiceConnected(ComponentName name, IBinder service) {
            m_Service = DrCOMServiceInterface.Stub.asInterface(service);
            // add listener
         	try {
				m_Service.addListener(m_Listener);
			} catch (RemoteException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}

			// connected auto
			if (!getPreferences(DrCOMDefine.DrCOMUrl).equals("")) {
				setConnectStatus(true);
				try {
					m_Service.setGetGWAddress(getPreferences(DrCOMDefine.DrCOMUrl));
					m_Service.setStatus(true);
					m_Service.getInfo(true);
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			else {
				if (m_cbSign.isChecked()) {
			    	onLogin();
			    }
			}
        }
    };

    private DrCOMServiceListener.Stub m_Listener = new DrCOMServiceListener.Stub() {
    	public void onLoginResult(String strError) {
    		sendMsg(DrCOMDefine.iLoginResult, strError);
    	}

    	public void onLogoutResult(String strError) {
    		sendMsg(DrCOMDefine.iLogoutResult, strError);
    	}

    	public void onRecvFlux(String strFlux) {
    		sendMsg(DrCOMDefine.iFluxResult, strFlux);
    	}

    	public void onRecvTime(String strTime) {
    		sendMsg(DrCOMDefine.iTimeResult, strTime);
    	}
    	public void onStatus(String strMessage) {
    		sendMsg(DrCOMDefine.iStatusResult, strMessage);
    	}

    	private boolean sendMsg(int iMsgType, String Msg) {
    		Message msg = new Message();
    		msg.what = iMsgType;
    		msg.getData().putString(Integer.toString(iMsgType), Msg);
    		return mHandler.sendMessage(msg);
    	}
    };

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	//for debug
    	//Jni auth = new Jni();
    	//String a = auth.getData();
    	//end debug

    	requestWindowFeature(Window.FEATURE_NO_TITLE);	//一定要放在第一行
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main1);

        m_txUsername = (TextView)this.findViewById(R.id.tx_username);
        m_txPassword = (TextView)this.findViewById(R.id.tx_password);
        m_edUsername = (EditText)this.findViewById(R.id.ed_username);
        m_edPassword = (EditText)this.findViewById(R.id.ed_password);
        m_txTime = (TextView)this.findViewById(R.id.tx_time);
    	m_txMin = (TextView)this.findViewById(R.id.tx_min);
    	m_txFlux = (TextView)this.findViewById(R.id.tx_flux);
    	m_txMByte = (TextView)this.findViewById(R.id.tx_mbyte);
    	m_txVersion = (TextView)this.findViewById(R.id.version);
        m_cbKeepPassword = (CheckBox)this.findViewById(R.id.cb_keep_password);
        m_cbSign = (CheckBox)this.findViewById(R.id.cb_sign);
        m_btOK = (Button)this.findViewById(R.id.bt_ok);
        m_msgBox = new AlertDialog.Builder(this).create();
        m_msgBox.setTitle(this.getString(R.string.Tips));

        m_msgBoxWarm = new AlertDialog.Builder(this).create();
        m_msgBoxWarm.setTitle(this.getString(R.string.Tips));

        intentDrCOMService.setClass(this, DrCOMService.class);
        intentProcess.setClass(this, Process.class);

        m_btOK.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				onLogin();
			}
        });

        m_cbKeepPassword.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				// TODO Auto-generated method stub
				if (m_cbKeepPassword.isChecked()) {
					putPreferences(DrCOMDefine.DrCOMRememberPass, DrCOMDefine.DrCOMYES);
				} else {
					m_cbSign.setChecked(false);
					putPreferences(DrCOMDefine.DrCOMSignIn, DrCOMDefine.DrCOMYNO);
					putPreferences(DrCOMDefine.DrCOMRememberPass, DrCOMDefine.DrCOMYNO);
				}
			}
        });

        m_cbSign.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				// TODO Auto-generated method stub
				if (m_cbSign.isChecked()) {
					m_cbKeepPassword.setChecked(true);
					putPreferences(DrCOMDefine.DrCOMRememberPass, DrCOMDefine.DrCOMYES);
					putPreferences(DrCOMDefine.DrCOMSignIn, DrCOMDefine.DrCOMYES);
				} else {
					putPreferences(DrCOMDefine.DrCOMSignIn, DrCOMDefine.DrCOMYNO);
				}
			}
        });

        m_msgBox.setButton(this.getString(R.string.OK), new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				m_msgBox.dismiss();
			}
		});

        m_msgBoxWarm.setButton(this.getString(R.string.OK), new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				if(m_Service != null){
		            try {
		            	if(mConnectStatus)
		            		m_Service.Logout();
//		    			if (m_Service.getStatus()) {
//		    				m_Service.Logout();
//		    			}
		            } catch (RemoteException e) {
		            	Log.e("DrCOMWS:onCreate:m_msgBoxWarm", e.toString());
		            }
		        }
				m_msgBoxWarm.dismiss();
				finish();
			}
		});

        m_msgBoxWarm.setButton2(this.getString(R.string.Cancel), new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				m_msgBoxWarm.dismiss();
			}
		});

        setConnectStatus(false);
        
        try {
        	bindService(intentDrCOMService, m_Connection, Context.BIND_AUTO_CREATE);
        }catch (SecurityException e) {
        	Log.e("DrCOMWS:onCreate", e.toString());
		}

        initUI();
        mAppUpdateManagement = new AppUpdateManagement(DrCOMWS.this);
        
        //测试PUSH
//        initPush();
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
	    if (keyCode == KeyEvent.KEYCODE_BACK) {
	    	m_msgBoxWarm.setMessage(this.getString(R.string.Do_you_want_to_exist));
	    	m_msgBoxWarm.show();
	    	return true;
	    }
	    return super.onKeyDown(keyCode, event);
	}

    public void onLogin() {
    	m_strMsg = "";
    	String strMessage = "";
    	m_strUsername = m_edUsername.getText().toString();
    	m_strPassword = m_edPassword.getText().toString();

    	boolean bConnectStatus = false;
    	try {
    		bConnectStatus = m_Service.getStatus();
		} catch (RemoteException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

		if(!bConnectStatus) {
			// 非在线
	    	if (m_strUsername.length() < 1) {
	    		strMessage = this.getString(R.string.Please_enter_your_username);
	    	} else if (m_strPassword.length() < 1) {
	    		strMessage = this.getString(R.string.Please_enter_your_password);
	    	}
		}

    	if (strMessage.length() > 1) {
    		showMsg(strMessage);
    	} else {
    		showAnimation(true);
    		try {
    			if (!bConnectStatus) {
    			//if(m_strGWAddress.length() == 0) {
    				m_Service.Login(m_strGWAddress, m_strUsername, m_strPassword);
    			} else {
    				m_Service.Logout();
    			}
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				Log.e("DrCOMWS:onLogin", e.toString());
			}
    	}
    }

    public void showMsg(String str) {
    	try{
    		showAnimation(false);
    		m_msgBox.setMessage(str);
        	m_msgBox.show();
    	}catch (Exception e) {
			// TODO: handle exception
		}
    	
    }

    public void onDestroy() {
    	//
    	try {
        	m_Service.rmtListener(m_Listener);
        } catch (RemoteException e) {
        	Log.e("DrCOMWS:onDestroy", e.toString());
        }
        
        try{
        	stopService(intentDrCOMService);
        	unbindService(m_Connection);
        }catch (Exception e) {
			// TODO: handle exception
		}
        
    	mAppUpdateManagement.Destory();
    	super.onDestroy();
    }

    private void showAnimation(boolean bShow) {
    	try {
	    	if (bShow) {
//	    		startActivityForResult(intentProcess, 0);
	    		ShowLoadingDialog() ;
	    	} else {
//	    		finishActivity(0);
	    		HideLoadingDialog();
	    	}
    	} catch (Exception e) {
    		Log.e("DrCOMWS:showAnimation", e.toString());
    	}
    }

    /*
	 * when bFlag is ture means Login resault, otherwise means Logout
	 */
    public void onResultMsg(boolean bFlag, String strMsg) {
    	showAnimation(false);
		if (strMsg.length() > 0) {
			if (!m_strMsg.equals(strMsg)) {
				m_strMsg = strMsg;
				showMsg(strMsg);
			}
		} else {
			try {
				if (bFlag) {
					m_Service.getInfo(true);
					setConnectStatus(true);
					putPreferences(DrCOMDefine.DrCOMUsername, m_strUsername);
					putPreferences(DrCOMDefine.DrCOMPass, m_strPassword);

					// mark gateway address
					m_strGWAddress = m_Service.getGWAddress();
					putPreferences(DrCOMDefine.DrCOMUrl, m_strGWAddress);
					
//					Toast.makeText(this, "注册PUSH", Toast.LENGTH_SHORT).show();
					initPush();
					
//					if (m_Service.getStatus()) {
//						// mark gateway address
//						m_strGWAddress = m_Service.getGWAddress();
//						putPreferences(DrCOMDefine.DrCOMUrl, m_strGWAddress);
//					}
				} else {
					m_Service.getInfo(false);
					setConnectStatus(false);
//					if (getPreferences(DrCOMDefine.DrCOMRememberPass).equals(DrCOMDefine.DrCOMYNO)) {
						// reset gateway address
						putPreferences(DrCOMDefine.DrCOMUrl, "");
//					}
				}
			} catch (RemoteException e) {
				// TODO Auto-generated catch block
				Log.e("DrCOMWS:onResultMsg", e.toString());
			}
		}
		try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

    /*
	 * when bFlag is ture means time info, otherwise means flux info
	 */
    public void onResultInfo(boolean bFlag, String strInfo) {
    	if (bFlag) {
    		m_txTime.setText(strInfo);
    	} else {
    		m_txFlux.setText(strInfo);
    	}
    }
    public void onResultStatusBreak(String strMsg) {
		try {
			if(strMsg.length() > 0) {
				showMsg(strMsg);
				m_Service.setStatus(false);
				m_Service.getInfo(false);
				setConnectStatus(false);
			}
		} catch (RemoteException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		setConnectStatus(false);
    }

    private void setConnectStatus(boolean bConnected) {
    	mConnectStatus = bConnected;
    	if (bConnected) {
    		m_txUsername.setText(R.string.Used_time);
	        m_txPassword.setText(R.string.Used_flux);
	        m_edUsername.setVisibility(View.GONE);
	        m_edPassword.setVisibility(View.GONE);

    		m_txTime.setVisibility(View.VISIBLE);
	        m_txMin.setVisibility(View.VISIBLE);
	        m_txFlux.setVisibility(View.VISIBLE);
	        m_txMByte.setVisibility(View.VISIBLE);

	        m_btOK.setText(R.string.logout);
	        m_btOK.setTextColor(this.getResources().getColor(R.color.light_red));

	        m_cbKeepPassword.setEnabled(false);
	        m_cbSign.setEnabled(false);
    	} else {
    		m_txUsername.setText(R.string.Username);
	        m_txPassword.setText(R.string.Password);
	        m_edUsername.setVisibility(View.VISIBLE);
	        m_edPassword.setVisibility(View.VISIBLE);

	    	m_txTime.setVisibility(View.GONE);
	        m_txMin.setVisibility(View.GONE);
	        m_txFlux.setVisibility(View.GONE);
	        m_txMByte.setVisibility(View.GONE);

	        m_btOK.setText(R.string.login);
	        m_btOK.setTextColor(this.getResources().getColor(R.color.light_green));

	        m_cbKeepPassword.setEnabled(true);
	        m_cbSign.setEnabled(true);
    	}
    }

    private void initUI() {
    	m_spfConf = getSharedPreferences(DrCOMDefine.DrCOMClientWS, 0);
        m_edUsername.setText(getPreferences(DrCOMDefine.DrCOMUsername));
        m_strGWAddress = getPreferences(DrCOMDefine.DrCOMUrl);
        if (getPreferences(DrCOMDefine.DrCOMSignIn).equals(DrCOMDefine.DrCOMYES)) {
        	m_cbSign.setChecked(true);
        	m_cbKeepPassword.setChecked(true);
        } else {
        	m_cbSign.setChecked(false);
        }
        if (getPreferences(DrCOMDefine.DrCOMRememberPass).equals(DrCOMDefine.DrCOMYES)) {
        	m_cbKeepPassword.setChecked(true);
        	m_edPassword.setText(getPreferences(DrCOMDefine.DrCOMPass));
        } else {
        	m_cbKeepPassword.setChecked(false);
        }
        
        //版本号
        PackageManager packageManager = getPackageManager();
		// getPackageName()是你当前类的包名，0代表是获取版本信息
		PackageInfo packInfo = null;
		try {
			packInfo = packageManager.getPackageInfo(getPackageName(), 0);
			m_txVersion.setText("Ver " + packInfo.versionName);
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
    }

    private String getPreferences(String strKey) {
    	String strValue = m_spfConf.getString(strKey, "");
    	if (strValue.length() > 0) {
    		try {
				strValue = DrAES128.decrypt(DrCOMDefine.DrCOMClientWS, strValue);
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
    	}
    	return strValue;
    }

    private boolean putPreferences(String strKey, String strValue) {
    	String strValueTmp = "";
    	try {
			strValueTmp = DrAES128.encrypt(DrCOMDefine.DrCOMClientWS, strValue);
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	SharedPreferences.Editor editor = m_spfConf.edit();
    	editor.putString(strKey, strValueTmp);
    	return editor.commit();
    }

    static {
        System.loadLibrary("DrClientLib");
    }

    private void initPush(){
    	//初始化PUSH服务Log
		DrServiceLog.getInstance(this);
		//初始化PUSH服务并注册
		PushManager.init(getApplicationContext(), true , new ConnectPushCallback() {

			@Override
			public void onSuccess() {
				// TODO Auto-generated method stub
				try {
//					PushManager.Register("9532", "a8Ca7rzh7g9532jQiunuFHK6mRAKMhKh");
					PushManager.Register("DrCOMWS", "2013-08-29");
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			
			@Override
			public void onError(String err) {
				// TODO Auto-generated method stub
				
			}
		});
    }
    
	/**
	 * 弹出LOADING对话框
	 */
	public void ShowLoadingDialog() {
		progressDlg = new ProgressDialog(DrCOMWS.this);
		progressDlg.setMessage(getString(R.string.pleasewait));
		progressDlg.setCancelable(false);
		progressDlg.show();
	}
	
	/**
	 * 隐藏LOADING对话框
	 */
	public void HideLoadingDialog(){
		if(progressDlg != null){
			progressDlg.dismiss();
			progressDlg = null;
		}
		
	}
}