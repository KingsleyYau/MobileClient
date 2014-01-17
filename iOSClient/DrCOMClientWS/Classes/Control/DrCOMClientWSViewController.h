//
//  DrCOMClientWSViewController.h
//  DrCOMClientWS
//
//  Created by Keqin Su on 11-4-15.
//  Copyright 2011 City Hotspot Co., Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DrCOMClientWSAppDelegate.h"
@interface DrCOMClientWSViewController : UIViewController {
    IDrCOMAuth *_drCOMAuth;
}
@property (nonatomic, retain) IBOutlet UILabel *versionLabel;
@property (nonatomic, retain) IBOutlet UITextField *nameField;
@property (nonatomic, retain) IBOutlet UITextField *passField;
@property (nonatomic, retain) IBOutlet UILabel *nameLabel;
@property (nonatomic, retain) IBOutlet UILabel *passLabel;
@property (nonatomic, retain) IBOutlet UIButton *loginButton;

@property (nonatomic, retain) IBOutlet UILabel *usedTimeLabel;
@property (nonatomic, retain) IBOutlet UILabel *usedFluxLabel;
@property (nonatomic, retain) IBOutlet UILabel *timeLabel;
@property (nonatomic, retain) IBOutlet UILabel *fluxLabel;
@property (nonatomic, retain) IBOutlet UILabel *minLabel;
@property (nonatomic, retain) IBOutlet UILabel *mbyteLabel;
@property (nonatomic, retain) IBOutlet UIButton *logoutButton;

@property (nonatomic, retain) IBOutlet UILabel *rememberLabel;
@property (nonatomic, retain) IBOutlet UILabel *signLabel;
@property (nonatomic, retain) IBOutlet UILabel *reconnectLabel;

@property (nonatomic, retain) IBOutlet UISwitch *rememberSwitch;
@property (nonatomic, retain) IBOutlet UISwitch *signSwitch;
@property (nonatomic, retain) IBOutlet UISwitch *reconnectSwitch;

@property (nonatomic, retain) IBOutlet UIView *markView;

- (IBAction)backgroundTap:(id)sender;
- (IBAction)onLogin;
- (IBAction)onLogout;
- (void)signChanged:(id)sender;
- (void)rememberChanged:(id)sender;

@end

