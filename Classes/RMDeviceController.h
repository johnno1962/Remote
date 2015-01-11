//
//  RPDeviceController.h
//  Remote
//
//  Created by John Holdsworth on 21/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RemoteCapture.h"

@class RMDeviceController;
@class RMImageView;

@protocol RMDeviceDelegate

@property (nonatomic) RMDeviceController *device;

- (void)logSet:(NSString *)html;
- (void)logAdd:(NSString *)html;

- (RMImageView *)imageView;
- (void)resize:(NSSize)size;
- (void)updateImage:(NSImage *)image;

- (void)reset;
- (NSTimeInterval)timeSinceLastEvent;

@end

@interface RMDeviceController : NSObject {
@public
    struct _rmdevice device;
    struct _rmframe frame;
}

- (instancetype)initSocket:(int)socket owner:(id<RMDeviceDelegate>)delegate;

- (void)sendEvent:(NSEvent *)theEvent phase:(RMTouchPhase)phase;
- (void)writeEvent:(const struct _rmevent *)event;

- (NSString *)snapshot:(RemoteCapture *)reference withFormat:(NSString *)format;
- (unsigned)differenceAgainst:(RemoteCapture *)snapshot;
- (RemoteCapture *)recoverBuffer:(NSString *)enc64;
- (NSImage *)recoverImage:(NSString *)enc64;

@end
