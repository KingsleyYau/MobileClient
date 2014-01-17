//
//  WiFiChecker.m
//  DrPalm
//
//  Created by fgx_lion on 12-2-10.
//  Copyright (c) 2012年 DrCOM. All rights reserved.
//

#import "WiFiChecker.h"
#import "Reachability.h"
@implementation WiFiChecker
+ (BOOL)isWiFiEnable
{
    Reachability *reach = [Reachability sharedReachability];
    NetworkStatus status = [reach localWiFiConnectionStatus];
    return ReachableViaWiFiNetwork == status;
}
+ (BOOL)isNetWorkOK {
    Reachability *reach = [Reachability sharedReachability];
    NetworkStatus status = [reach internetConnectionStatus];
    return (NotReachable != status);
}
+ (NSString*)currentSSID
{
    Reachability *reach = [Reachability sharedReachability];
    return [reach currentSSID];
}

@end
