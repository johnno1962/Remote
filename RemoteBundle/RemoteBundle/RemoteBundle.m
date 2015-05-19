//
//  RemoteBundle.m
//  Remote
//
//  Created by John Holdsworth on 18/05/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import "RemoteCapture.h"

@interface BundleInjection : NSObject
+ (void)loadedNotify:(int)notify hook:(void *)hook;
+ (const char *)connectedAddress;
@end

@implementation RemoteCapture(Autoload)

+ (void)load {
    Class injectionLoader = NSClassFromString(@"BundleInjection");
#if TARGET_IPHONE_SIMULATOR
    [RemoteCapture startCapture:@"127.0.0.1"];
#else
    if ( [injectionLoader respondsToSelector:@selector(connectedAddress)] )
        [RemoteCapture startCapture:[NSString stringWithUTF8String:[injectionLoader connectedAddress]]];
    else
        NSLog( @"Remote requires BundleInjection class to work on device" );
#endif
    [injectionLoader loadedNotify:0 hook:NULL];
}

@end
