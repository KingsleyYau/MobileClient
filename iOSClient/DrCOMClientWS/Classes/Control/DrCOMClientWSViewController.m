//
//  DrCOMClientWSViewController.m
//  DrCOMClientWS
//
//  Created by Keqin Su on 11-4-15.
//  Copyright 2011 City Hotspot Co., Ltd. All rights reserved.
//

#import "DrCOMClientWSViewController.h"
#import "SettingFileController.h"
#import "Utilities.h"
#import "WiFiChecker.h"

//add by Keqin Su for app store demo
#define DRCOM_Demo_User			@"dev.demo"
#define DRCOM_Demo_Pass			@"dev@dr.com"
#define DRCOM_Demo_Adress		@"210.21.59.58"

@interface DrCOMClientWSViewController() {
    
}
@property (nonatomic, retain) SettingFileController *fileController;

@property (nonatomic, retain) NSString *loginUsername;
@property (nonatomic, retain) NSString *loginPassword;
@property (nonatomic, retain) NSString *gwUrl;
@property (nonatomic, retain) NSTimer *timer;
@property (nonatomic, retain) NSString *errorMessage;
@property (nonatomic, retain) NSString *macList;

@property (nonatomic, assign) BOOL bSuccessed;
@property (nonatomic, assign) BOOL bSendUpdate;
@property (nonatomic, assign) NSInteger checkTime;

// 界面
- (void)startLoading;
- (void)stopLoading;
- (void)setInterface;
- (void)initFileSetting;
- (void)initDrClientLib;

// 逻辑处理
- (void)setViewControl:(BOOL)login;
- (void)startHttpStatusTimer;
- (BOOL)sendHttpStatus;
- (NSString *)errorMessageForCode:(NSInteger)errCode;

// 回调处理
- (void)loginSuccess;
- (void)loginFail;
- (void)logoutSuccess;
- (void)logoutFail;
- (void)statusFail;
// 升级
//- (NSString*)grantUpdateRequest;
//- (BOOL)sendUpdateRequest;
@end

@implementation DrCOMClientWSViewController
@synthesize versionLabel;
@synthesize nameField;
@synthesize passField;
@synthesize nameLabel;
@synthesize passLabel;
@synthesize loginButton;

@synthesize usedTimeLabel;
@synthesize usedFluxLabel;	
@synthesize timeLabel;
@synthesize fluxLabel;	
@synthesize minLabel;
@synthesize mbyteLabel;
@synthesize logoutButton;

@synthesize rememberLabel;
@synthesize signLabel;
@synthesize reconnectLabel;	

@synthesize rememberSwitch;
@synthesize signSwitch;
@synthesize reconnectSwitch;	

@synthesize markView;
#pragma mark - 构造
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)init {
    self = [super init];
    if(self){
        //_drCOMAuth = (DrCOMAuth *)IDrCOMAuth::CreateDrCOMAuth();
    }
    return self;
}
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization

    }
    return self;
}
- (void)dealloc {
	[self.timer invalidate];
	self.timer = nil;
    
	self.loginUsername = nil;
	self.loginPassword = nil;
	self.gwUrl = nil;
	self.fileController = nil;
    self.macList = nil;
    IDrCOMAuth::ReleaseDrCOMAuth(_drCOMAuth);
    [super dealloc];
}
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
	[super viewDidLoad];
    [self initDrClientLib];
    srand(time(NULL));
    // TODO:网卡地址
    self.macList = [Utilities GetHardwareAddressList];
    // TODO:初始化界面
	[self setInterface];
	//reconnectSwitch.enabled = NO;
	[self setViewControl:NO];
    // TODO:初始化配置
	[self initFileSetting];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	// Release any cached data, images, etc that aren't in use.
}
#pragma mark - 协议逻辑
- (void)initDrClientLib {
    _drCOMAuth = (IDrCOMAuth *)IDrCOMAuth::CreateDrCOMAuth();
}
- (void)initFileSetting {
    // 读取配置文件
    self.fileController = [[[SettingFileController alloc] init] autorelease];
	self.nameField.text = [Utilities decodeString:[self.fileController readParamInSettingFile:DrCOMUsername] key:DrCOMClientWS];
	self.gwUrl = [Utilities decodeString:[self.fileController readParamInSettingFile:DrCOMUrl] key:DrCOMClientWS];
    if(!self.gwUrl) {
        self.gwUrl = @"";
    }

    // 自动登陆
	if ([[self.fileController readParamInSettingFile:DrCOMSignIn] isEqualToString:DrCOMYES]) {
		[rememberSwitch setOn:YES animated:YES];
		[signSwitch setOn:YES animated:YES];
	} else {
		[signSwitch setOn:NO animated:YES];
	}
    // 记住密码
	if ([[self.fileController readParamInSettingFile:DrCOMRememberPass] isEqualToString:DrCOMYES]) {
		[rememberSwitch setOn:YES animated:YES];
		passField.text = [Utilities decodeString:[self.fileController readParamInSettingFile:DrCOMPass] key:DrCOMClientWS];
	} else {
		[rememberSwitch setOn:NO animated:YES];
	}
    
    // 如果已经登录,显示状态
    if(self.gwUrl.length > 0) {
        [self setViewControl:YES];
        // 调用获取在线状态
        _drCOMAuth->SetGatewayAddress([self.gwUrl UTF8String]);
        [self startHttpStatusTimer];
    }
	else if ([[self.fileController readParamInSettingFile:DrCOMSignIn] isEqualToString:DrCOMYES] && [nameField.text length] > 0 && [passField.text length] >0)  {
        // 自动登陆
		[self onLogin];
	}
}
- (BOOL)sendHttpStatus {
    // 检测在线状态
    BOOL ret = _drCOMAuth->httpStatus();
    dispatch_async(dispatch_get_main_queue(), ^{
        if(ret) {
            // 在线
            self.timeLabel.text = [[[NSString alloc] initWithUTF8String:_drCOMAuth->getTimeStatus().c_str()] autorelease];
			self.fluxLabel.text = [[[NSString alloc] initWithUTF8String:_drCOMAuth->getFluxStatus().c_str()] autorelease];
            if (self.bSendUpdate) {
                self.bSendUpdate = NO;
                //send data to update server, because the app will be run in background immediately after login
            }
        }
        else {
            // 下线
            self.timeLabel.text = @"0";
			self.fluxLabel.text = @"0";

            self.errorMessage = NSLocalizedString(@"DrClient_StatusError", nil);
            [self statusFail];
        }
    });
    return ret;
}
- (NSString *)errorMessageForCode:(NSInteger)errCode {
    NSString *errMessage = nil;
    switch (errCode) {
        case DrClientHS_NetworkError:{
            errMessage = NSLocalizedString(@"DrClient_NetworkError", nil);
            break;
        }
        case DrClientHS_UsingNAT:{
            errMessage = NSLocalizedString(@"DrClient_UsingNAT", nil);
            break;
        }
        case DrClientHS_NotFoundDrCOM:{
            errMessage = NSLocalizedString(@"DrClient_NotFoundDrCOM", nil);
            break;
        }
        case DrClientHS_AlreadyOnline:{
            errMessage = NSLocalizedString(@"DrClient_AlreadyOnline", nil);
            break;
        }
        case DrClientHS_IpNotAllowLogin:{
            errMessage = NSLocalizedString(@"DrClient_IpNotAllowLogin", nil);
            break;
        }
        case DrClientHS_AccountNotAllow:{
            errMessage = NSLocalizedString(@"DrClient_AccountNotAllow", nil);
            break;
        }
        case DrClientHS_NotAllowChangePwd:{
            errMessage = NSLocalizedString(@"DrClient_NotAllowChangePwd", nil);
            break;
        }
        case DrClientHS_InvalidAccountOrPwd:{
            errMessage = NSLocalizedString(@"DrClient_InvalidAccountOrPwd", nil);
            break;
        }
        case DrClientHS_AccountTieUp:{
            NSString *errTips = NSLocalizedString(@"DrClient_AccountTieUp", nil);
            NSString *errFormatMessage = [NSString stringWithFormat:@"%@IP(%@) MAC(%@)", errTips, [NSString stringWithUTF8String:_drCOMAuth->getXip().c_str()], [NSString stringWithUTF8String:_drCOMAuth->getMac().c_str()]];
            errMessage = errFormatMessage;
            break;
        }
        case DrClientHS_UsingOnAppointedIp:{
            NSString *errTips = NSLocalizedString(@"DrClient_UsingOnAppointedIp", nil);
            NSString *errFormatMessage = [NSString stringWithFormat:@"%@IP(%@)", errTips, [NSString stringWithUTF8String:_drCOMAuth->getXip().c_str()]];
            errMessage = errFormatMessage;
            break;
        }
        case DrClientHS_ChargeOrFluxOver:{
            errMessage = NSLocalizedString(@"DrClient_ChargeOrFluxOver", nil);
            break;
        }
        case DrClientHS_AccountBreakOff:{
            errMessage = NSLocalizedString(@"DrClient_AccountBreakOff", nil);
            break;
        }
        case DrClientHS_SystemBufferFull:{
            errMessage = NSLocalizedString(@"DrClient_SystemBufferFull", nil);
            break;
        }
        case DrClientHS_TieUpCannotAmend:{
            errMessage = NSLocalizedString(@"DrClient_TieUpCannotAmend", nil);
            break;
        }
        case DrClientHS_NewAndConfirmPwdDiffer:{
            errMessage = NSLocalizedString(@"DrClient_NewAndConfirmPwdDiffer", nil);
            break;
        }
        case DrClientHS_PwdAmendSuccessed:{
            errMessage = NSLocalizedString(@"DrClient_PwdAmendSuccessed", nil);
            break;
        }
        case DrClientHS_UsingAppointedMac:{
            NSString *errTips = NSLocalizedString(@"DrClient_UsingAppointedMac", nil);
            NSString *errFormatMessage = [NSString stringWithFormat:@"%@MAC(%@)", errTips, [NSString stringWithUTF8String:_drCOMAuth->getMac().c_str()]];
            errMessage = errFormatMessage;
            break;
        }
        case DrClientHS_UndefineError:{
            errMessage = NSLocalizedString(@"DrClient_UndefineError", nil);//[[[NSString alloc] initWithUTF8String:_drCOMAuth->getUndefineError().c_str()] autorelease];
            break;
        }
        default:
            break;
    }
    return errMessage;
}
#pragma mark - 协议回调
- (void)loginSuccess {
    // 刷新界面
    [self stopLoading];
	[self setViewControl:YES];
    
    // 记录网关地址
    self.gwUrl = [[[NSString alloc] initWithUTF8String:_drCOMAuth->getGatewayAddress().c_str()] autorelease];
    [self.fileController writeParamInSettingFile:DrCOMUrl value:[Utilities encodeString:self.gwUrl key:DrCOMClientWS]];
    // 记录用户名
    [self.fileController writeParamInSettingFile:DrCOMUsername value:[Utilities encodeString:self.loginUsername key:DrCOMClientWS]];
    [self.fileController writeParamInSettingFile:DrCOMPass value:[Utilities encodeString:self.loginPassword key:DrCOMClientWS]];

    // 开始刷新在线状态
    [self startHttpStatusTimer];
}
- (void)loginFail {
    [self stopLoading];
    [self setViewControl:NO];
    [Utilities showDrAlert:self.errorMessage];
}
- (void)logoutSuccess {
    // 刷新界面
    [self stopLoading];
    [self setViewControl:NO];
    
    // 记录网关地址
    self.gwUrl = @"";
    [self.fileController writeParamInSettingFile:DrCOMUrl value:@""];
    
    // 修改配置
    if (![[self.fileController readParamInSettingFile:DrCOMRememberPass] isEqualToString:DrCOMYES]) {
        self.passField.text = @"";
    }
    self.gwUrl = @"";
    [self.fileController writeParamInSettingFile:DrCOMUrl value:self.gwUrl];
    
    // 停止在线检测
    [self.timer invalidate];
    self.timer = nil;
}
- (void)logoutFail {
    [self stopLoading];
    [Utilities showDrAlert:self.errorMessage];
}
- (void)statusFail {
    [self setViewControl:NO];
    [Utilities showDrAlert:self.errorMessage];
    // 停止在线检测
    [self.timer invalidate];
    self.timer = nil;
}
- (void)startHttpStatusTimer {
    // 开始每30秒刷新在线状态
    if (self.timer == nil) {
        self.timer = [NSTimer scheduledTimerWithTimeInterval:(30.0) target:self selector:@selector(timerFireMethod:) userInfo:nil repeats:YES];
        [self.timer fire];
    }
}
- (void)timerFireMethod:(NSTimer*)theTimer {
    [self sendHttpStatus];
}
#pragma mark - 界面逻辑
- (IBAction)backgroundTap:(id)sender {
    // 点击背景
	[nameField resignFirstResponder];
	[passField resignFirstResponder];
}
- (IBAction)onLogin {
    // 点击登录
	self.bSuccessed = NO;
	self.checkTime = 1;
	self.errorMessage = nil;

    self.loginUsername = nameField.text;
	
	self.loginPassword = passField.text;
	NSString *msg = nil;
	
    if(![WiFiChecker isWiFiEnable]) {
        // WiFi不可用
        msg = NSLocalizedString(@"DrClient_NetworkError", nil);
    }
	else if ([self.loginUsername length] < 1) {
		msg = NSLocalizedString(@"Please enter your username", nil);
	} else {
		if ([self.loginPassword length] < 1) {
			msg = NSLocalizedString(@"Please enter your password", nil);			
		}
	}
	
	if ([msg length] > 1) {
        // 界面输入错误
		[Utilities showDrAlert:msg];
	} else {
        [self startLoading];
//        dispatch_queue_t loginQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_queue_t requestQueue = dispatch_queue_create("com.drcom.com.login", NULL);        dispatch_async(requestQueue, ^{
            // 开始登录,阻塞
            NSString *strSSID = [WiFiChecker currentSSID];
            _drCOMAuth->SetSSID([strSSID UTF8String]);
            NSInteger ret = _drCOMAuth->httpLogin([self.gwUrl UTF8String], [self.loginUsername UTF8String], [self.loginPassword UTF8String]);
            ret = ret > 0? DrClientHS_Success:ret;
            // 登录完成
            dispatch_async(dispatch_get_main_queue(), ^{
                if(DrClientHS_Success == ret) {
                    // 登录成功
                    [self loginSuccess];
                }
                else {
                    // 登录失败
                    NSLog(@"login error code:%d", ret);
                    self.errorMessage = [self errorMessageForCode:ret];
                    [self loginFail];
                }
            });

        });
        dispatch_release(requestQueue);
    }
}
- (IBAction)onLogout {
    [self startLoading];
    dispatch_queue_t requestQueue = dispatch_queue_create("com.drcom.com.logout", NULL);
    dispatch_async(requestQueue, ^{
        // 开始注销
        NSInteger ret = _drCOMAuth->httpLogout();
        ret = ret > 0? DrClientHS_Success:ret;
        dispatch_async(dispatch_get_main_queue(), ^{
            // 注销完成
            if(DrClientHS_Success == ret) {
                // 注销成功
                [self logoutSuccess];
            }
            else {
                // 注销失败
                NSLog(@"logout error code:%d", ret);
                self.errorMessage = [self errorMessageForCode:ret];
                [self logoutFail];
            }
            
        });
    });
    dispatch_release(requestQueue);
}
- (void)signChanged:(id)sender {
    // 自动登录开关改变
	UISwitch *witchSwitch = (UISwitch*)sender;
	NSString *val = nil;
	
	if (witchSwitch.isOn) {
		val = DrCOMYES;
		[rememberSwitch setOn:YES animated:YES];
		[self.fileController writeParamInSettingFile:DrCOMRememberPass value:val];
	} else {
		val = DrCOMYNO;
	}
	[self.fileController writeParamInSettingFile:DrCOMSignIn value:val];
}
- (void)rememberChanged:(id)sender {
    // 记住密码开关改变
	UISwitch *witchSwitch = (UISwitch*)sender;
	NSString *val = nil;

	if (witchSwitch.isOn) {
		val = DrCOMYES;
	} else {
		val = DrCOMYNO;
		[signSwitch setOn:NO animated:YES];
		[self.fileController writeParamInSettingFile:DrCOMSignIn value:val];
	}
	[self.fileController writeParamInSettingFile:DrCOMRememberPass value:val];
}
- (void)setViewControl:(BOOL)login {
    // 启用/禁用控件
	if (login) {
		nameField.hidden = YES;
		passField.hidden = YES;
		nameLabel.hidden = YES;
		passLabel.hidden = YES;
		loginButton.hidden = YES;
		
		usedTimeLabel.hidden = NO;
		usedFluxLabel.hidden = NO;
		timeLabel.hidden = NO;
		fluxLabel.hidden = NO;
		minLabel.hidden = NO;
		mbyteLabel.hidden = NO;
		logoutButton.hidden = NO;
		
		rememberSwitch.enabled = NO;
		signSwitch.enabled = NO;
	} else {
		nameField.hidden = NO;
		passField.hidden = NO;
		nameLabel.hidden = NO;
		passLabel.hidden = NO;
		loginButton.hidden = NO;
		
		usedTimeLabel.hidden = YES;
		usedFluxLabel.hidden = YES;
		timeLabel.hidden = YES;
		fluxLabel.hidden = YES;
		minLabel.hidden = YES;
		mbyteLabel.hidden = YES;
		logoutButton.hidden = YES;
		
		rememberSwitch.enabled = YES;
		signSwitch.enabled = YES;
	}
}
- (void)setInterface {
    // 初始化界面
	self.nameField.placeholder = NSLocalizedString(@"username", nil);
	self.passField.placeholder = NSLocalizedString(@"password", nil);
	self.nameLabel.text = NSLocalizedString(@"Username:", nil);
	self.passLabel.text = NSLocalizedString(@"Password:", nil);
	[self.loginButton setTitle:NSLocalizedString(@"Login", nil) forState:UIControlStateNormal];
	self.usedTimeLabel.text = NSLocalizedString(@"Used time:", nil);
	self.usedFluxLabel.text = NSLocalizedString(@"Used flux:", nil);
	self.minLabel.text  = NSLocalizedString(@"Min", nil);
	self.mbyteLabel.text  = NSLocalizedString(@"MByte", nil);
	[self.logoutButton setTitle:NSLocalizedString(@"Logout", nil) forState:UIControlStateNormal];
	self.rememberLabel.text = NSLocalizedString(@"Remember my password", nil);
	self.signLabel.text = NSLocalizedString(@"Sign in Automatically", nil);
	self.reconnectLabel.text = NSLocalizedString(@"Reconnect Automatically", nil);
    // TODO:读取版本号
    NSString *verString = [NSString stringWithFormat:@"Ver %@", [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"]];
    self.versionLabel.text = verString;
}
- (void)startLoading{
	[self.view addSubview:markView];
	[(UIActivityIndicatorView *)[markView viewWithTag:202] startAnimating];
	
	markView.center = CGPointMake(markView.frame.size.width/2, 0 - markView.frame.size.height/2);
	markView.alpha = 0.0f;
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:0.3f];
	[UIView setAnimationCurve:UIViewAnimationCurveLinear];
	markView.center = CGPointMake(markView.frame.size.width/2, markView.frame.size.height/2);
	markView.alpha = 0.70f;
	[UIView commitAnimations];
}
- (void)stopLoading {
	[(UIActivityIndicatorView *)[markView viewWithTag:202] stopAnimating];
	[markView removeFromSuperview];
}
#pragma mark - 输入回调(UITextFieldDelegate)
- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
	if (range.location >= MaxTextFieldInputLen) {
		return NO;
	}
	return YES;
}
- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	[textField resignFirstResponder];
	return YES;
}

@end
