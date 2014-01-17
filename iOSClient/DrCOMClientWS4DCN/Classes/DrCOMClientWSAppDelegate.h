//
//  DrCOMClientWSAppDelegate.h
//  DrCOMClientWS
//
//  Created by Keqin Su on 11-4-15.
//  Copyright 2011 City Hotspot Co., Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DrClientLib.h"
@class DrCOMClientWSViewController;


@interface DrCOMClientWSAppDelegate : NSObject <UIApplicationDelegate> {
    DrCOMClientWSViewController *_viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, assign) IBOutlet DrCOMClientWSViewController *viewController;

@end

