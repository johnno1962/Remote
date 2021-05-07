//
//  RMDeviceController.h
//  Remote
//
//  Created by John Holdsworth on 21/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//
//  $Id: //depot/Remote/Sources/RemoteUI/include/RMDeviceController.h#3 $
//

#import <Cocoa/Cocoa.h>

#undef XPROBE_PORT
#define REMOTE_LEGACY
#import "RemoteCapture.h"

@class RMDeviceController;
@class RMImageView;

@protocol RMDeviceDelegate <NSObject>

@property (nonatomic) RMDeviceController *device;

- (void)logSet:(NSString *)html;
- (void)logAdd:(NSString *)html;

- (RMImageView *)imageView;
- (void)resize:(NSSize)size;
- (void)updateImage:(CGImageRef)img;
- (void)loading:(BOOL)active;

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
- (void)sendText:(NSString *)text;
- (void)shutdown;

- (NSString *)snapshot:(RemoteCapture *)reference withFormat:(NSString *)format;
- (unsigned)differenceAgainst:(RemoteCapture *)snapshot;
- (RemoteCapture *)recoverBuffer:(NSString *)enc64;
- (NSImage *)recoverImage:(NSString *)enc64;

@end
