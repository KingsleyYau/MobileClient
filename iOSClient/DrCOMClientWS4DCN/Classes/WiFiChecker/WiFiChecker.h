//
//  WiFiChecker.h
//  DrPalm
//
//  Created by fgx_lion on 12-2-10.
//  Copyright (c) 2012年 DrCOM. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface WiFiChecker : NSObject
+ (BOOL)isWiFiEnable;
+ (BOOL)isNetWorkOK;
+ (NSString*)currentSSID;

@end
