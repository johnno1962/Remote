//
//  RemoteCapture.h
//  Remote
//
//  Created by John Holdsworth on 14/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#include <sys/sysctl.h>
#import <netinet/tcp.h>
#import <sys/socket.h>
#import <arpa/inet.h>

#define INJECTION_PORT 31442
#define APPCODE_PORT 31444
#define XPROBE_PORT 31448
#define REMOTE_PORT 31449
#define REMOTE_APPNAME "Remote"
#define REMOTE_MINDIFF (3*sizeof(unsigned))

#ifdef DEBUG
#define RMLog NSLog
#else
#define RMLog while(0) NSLog
#endif

typedef NS_ENUM(int, RMTouchPhase) {
    RMTouchBeganDouble = -1,
    RMTouchBegan = 0,
    RMTouchMoved,
    RMTouchStationary,
    RMTouchEnded,
    RMTouchCancelled,
    RMTouchUseScale,
};

#define RMMAX_TOUCHES 2

struct _rmevent {
    RMTouchPhase phase;
    union {
        struct { float x, y; } touches[RMMAX_TOUCHES];
    };
};

struct _rmdevice {
    char machine[24];
    float scale;
    int isIPad;
};

struct _rmframe {
    union {
        struct { float width, height, imageScale; };
        struct { float x, y; RMTouchPhase phase; };
    };
    int length;
};

@interface RemoteCapture : NSObject {
@package
    unsigned *buffer, *buffend;
    CGContextRef cg;
}

@end

#if (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && defined(DEBUG) || \
    !defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && defined(REMOTE_IMPL))

#ifndef __IPHONE_OS_VERSION_MIN_REQUIRED
#import <Cocoa/Cocoa.h>
#else
#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import "RemoteHeaders.h"

static UITouch *currentTouch;
static NSSet *currentTouches;

@interface BCEvent : NSObject {
    NSTimeInterval _timestamp;
}

@end

@implementation BCEvent

- (instancetype)init {
    if ( (self = [super init]) ) {
        _timestamp = [NSDate timeIntervalSinceReferenceDate];
    }
    return self;
}

- (NSTimeInterval)timestamp {
    return _timestamp;
}

- (UITouch *)_firstTouchForView:(UIView *)view {
    RMLog( @"_firstTouchForView: %@", view );
    return currentTouch;
}

- (NSSet *)touchesForView:(UIView *)view {
    RMLog( @"touchesForWindow:%@", view );
    return currentTouches;
}

- (NSSet *)touchesForWindow:(UIWindow *)window {
    RMLog( @"touchesForWindow:%@", window );
    return currentTouches;
}

- (NSSet *)touchesForGestureRecognizer:(UIGestureRecognizer *)rec {
    RMLog( @"touchesForGestureRecognizer:%@", rec );
    return currentTouches;
}

- (void)_removeTouch:(UITouch *)touch fromGestureRecognizer:(UIGestureRecognizer *)rec {
    RMLog( @"_removeTouch:%@ fromGestureRecognizer:%@", touch, rec );
}

- (NSSet *)allTouches {
    return currentTouches;
}

- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 {
    RMLog( @"_addWindowAwaitingLatentSystemGestureNotification:%@ deliveredToEventWindow:%@", a0, a1 );
}

@end

@interface NSObject(ForwardReference)

- (void *)_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2;
- (void *)in_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2;

- (void)_didCommitLayer:(void *)a0;
- (void)in_didCommitLayer:(void *)a0;

@end

@interface UIApplication(ForwardReference)
- (void)in_sendEvent:(UIEvent *)event;
@end
#endif

@implementation RemoteCapture

- (instancetype)initFrame:(const struct _rmframe *)frame {
    if ( (self = [super init]) ) {
        CGSize size = { frame->width*frame->imageScale, frame->height*frame->imageScale };
        int bitsPerComponent = 8, bytesPerRow = bitsPerComponent/8*4 * (int)size.width;
        int bufferSize = bytesPerRow * (int)size.height;
        cg = CGBitmapContextCreate(NULL, size.width, size.height,
                                   bitsPerComponent, bytesPerRow, CGColorSpaceCreateDeviceRGB(),
                                   (CGBitmapInfo)kCGImageAlphaNoneSkipFirst);
        buffer = (unsigned *)CGBitmapContextGetData(cg);
        buffend = (unsigned *)((char *)buffer + bufferSize);
        CGContextTranslateCTM(cg, 0, size.height);
        CGContextScaleCTM(cg, frame->imageScale, -frame->imageScale);
    }
    return self;
}

- (NSData *)subtractAndEncode:(RemoteCapture *)prevbuff {
    unsigned tmpsize = 16*1024;
    unsigned *tmp = (unsigned *)malloc(tmpsize*sizeof *tmp), *end = tmp + tmpsize-4;

    unsigned *out = tmp, count = 0, expectedDiff = 0, check = 0;
    for ( register unsigned
             *curr = buffer,
             *prev = prevbuff ? prevbuff->buffer : NULL ;
             curr < buffend ; check += *curr, curr++ ) {
        unsigned ref = (prev ? *prev++ : 0), diff = *curr - ref - expectedDiff;
        if ( !diff && curr != buffer )
            count++;
        else {
            if ( count ) {
                if ( count < 0xff )
                    out[-1] |= count;
                else {
                    out[-1] |= 0xff;
                    *out++ = count;
                }
                count = 0;
            }

            *out++ = diff & 0xffffff00;
        }

        expectedDiff = *curr - ref;

        if ( out >= end ) {
            size_t ptr = out - tmp;
            tmpsize *= 1.1;
            tmp = (unsigned *)realloc(tmp, tmpsize * sizeof *tmp);
            out = tmp + ptr;
            end = tmp + tmpsize-4;
        }
    }

    if ( count ) {
        if ( count < 0xff )
            out[-1] |= count;
        else {
            out[-1] |= 0xff;
            *out++ = count;
        }
    }

    *out++ = check;

    return [NSData dataWithBytesNoCopy:tmp length:(char *)out-(char *)tmp freeWhenDone:YES];
}

- (CGImageRef)cgImage {
    return CGBitmapContextCreateImage( cg );
}

- (void)dealloc {
    CGContextRelease(cg);
}

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED

static struct _rmdevice device;
static int connectionSocket;
static Class UIWindowLayer;
static NSArray *screens;

#ifdef REMOTEPLUGIN_SERVERIPS
+ (void)load {
    [self performSelectorInBackground:@selector(startCapture:) withObject:@REMOTEPLUGIN_SERVERIPS];
}
#endif

+ (void)startCapture:(NSString *)addrs {
#if TARGET_IPHONE_SIMULATOR
    addrs = @"127.0.0.1";
#endif
    UIWindowLayer = objc_getClass("UIWindowLayer");
    for ( NSString *addr in [addrs componentsSeparatedByString:@" "] )
        if ( (connectionSocket = [self connectTo:[addr UTF8String]]) )
            break;

    if ( !connectionSocket )
        return;
    else
        NSLog( @"RemoteCapture: Connected." );

    while ( !(screens = [UIScreen screens]).count )
        [NSThread sleepForTimeInterval:.5];

#if 1
    method_exchangeImplementations(class_getInstanceMethod(objc_getClass("CALayer"), @selector(_copyRenderLayer:layerFlags:commitFlags:)),
                                   class_getInstanceMethod([NSObject class], @selector(in_copyRenderLayer:layerFlags:commitFlags:)));
#else
    method_exchangeImplementations(class_getInstanceMethod(objc_getClass("CALayer"), @selector(_didCommitLayer:)),
                                   class_getInstanceMethod([NSObject class], @selector(in_didCommitLayer:)));
#endif

    method_exchangeImplementations(class_getInstanceMethod([UIApplication class], @selector(sendEvent:)),
                                   class_getInstanceMethod([UIApplication class], @selector(in_sendEvent:)));

    size_t size = sizeof device.machine;
    sysctlbyname("hw.machine", device.machine, &size, NULL, 0);
    device.machine[size] = '\000';

    UIScreen *screen = screens[0];
    device.scale = [screen scale];
    device.isIPad = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;

    if ( write( connectionSocket, &device, sizeof device ) != sizeof device )
        NSLog( @"RemoteCapture: Could not write device info" );

    [self performSelectorInBackground:@selector(processEvents) withObject:nil];
    [self performSelectorOnMainThread:@selector(capture) withObject:nil waitUntilDone:NO];
}

+ (int)connectTo:(const char *)ipAddress {
    struct sockaddr_in loaderAddr;

    loaderAddr.sin_family = AF_INET;
    inet_aton( ipAddress, &loaderAddr.sin_addr );
    loaderAddr.sin_port = htons(REMOTE_PORT);

    NSLog( @"RemoteCapture: %s attempting connection to: %s:%d", REMOTE_APPNAME, ipAddress, REMOTE_PORT );

    int loaderSocket, optval = 1;
    if ( (loaderSocket = socket(loaderAddr.sin_family, SOCK_STREAM, 0)) < 0 )
        NSLog( @"RemoteCapture: %s: Could not open socket for injection: %s", REMOTE_APPNAME, strerror( errno ) );
    else if ( setsockopt( loaderSocket, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval)) < 0 )
        NSLog( @"RemoteCapture: %s: Could not set TCP_NODELAY: %s", REMOTE_APPNAME, strerror( errno ) );
    else if ( connect( loaderSocket, (struct sockaddr *)&loaderAddr, sizeof loaderAddr ) >= 0 )
        return loaderSocket;

    NSLog( @"RemoteCapture: %s: Could not connect: %s", REMOTE_APPNAME, strerror( errno ) );
    close( loaderSocket );
    return 0;
}

static dispatch_queue_t writeQueue;
static int skipEcho, pending;

+ (void)capture {
    UIScreen *screen = screens[0];
    CGRect bounds = screen.bounds;
    CGFloat imageScale = device.isIPad || device.scale == 3. ? 1. : screen.scale;

    struct _rmframe frame = { bounds.size.width, bounds.size.height, imageScale, 0 };

    static NSArray *buffers;
    static CGSize size;
    static int frameno;

    if ( size.width != frame.width || size.height != frame.height ) {
        buffers = nil;
        buffers = @[[[RemoteCapture alloc] initFrame:&frame],
                    [[RemoteCapture alloc] initFrame:&frame]];
        size = bounds.size;
        frameno = 0;
    }

    RemoteCapture *buffer = buffers[frameno++%2];
    RemoteCapture *prevbuff = buffers[frameno%2];

    BOOL benchmark = FALSE;
    for ( UIWindow *window in [[UIApplication sharedApplication] windows] ) {
        NSTimeInterval start = [NSDate timeIntervalSinceReferenceDate];
#if 0
        UIView *snap = [window snapshotViewAfterScreenUpdates:YES];
        [snap.layer renderInContext:buffer->cg];
#else
        [[window layer] renderInContext:buffer->cg];
#endif
        if( benchmark )
            RMLog( @"%@ %f", NSStringFromCGRect(window.bounds),
                  [NSDate timeIntervalSinceReferenceDate]-start );
    }

    pending = 0;
    skipEcho = 3;

    NSTimeInterval start = [NSDate timeIntervalSinceReferenceDate];
    NSData *out = [buffer subtractAndEncode:prevbuff];
    frame.length = (unsigned)[out length];

    if ( benchmark )
        RMLog( @"== %f", [NSDate timeIntervalSinceReferenceDate]-start );

    if ( !writeQueue )
        writeQueue = dispatch_queue_create("writeQueue", DISPATCH_QUEUE_SERIAL);

    if ( frame.length != REMOTE_MINDIFF )
        dispatch_async(writeQueue, ^{
            if ( write(connectionSocket, &frame, sizeof frame) != sizeof frame )
                NSLog( @"RemoteCapture: Could not write bounds" );
            else if ( write(connectionSocket, [out bytes], frame.length) != frame.length )
                NSLog( @"RemoteCapture: Could not write out" );
        });
}

+ (void)processEvents {
    FILE *eventStream = fdopen(connectionSocket, "r");

    struct _rmevent rpevent;
    while ( fread(&rpevent, 1, sizeof rpevent, eventStream) == sizeof rpevent ) {

        RMLog( @"Remote Event: %f %f %d", rpevent.touches[0].x, rpevent.touches[0].y, rpevent.phase );
        dispatch_sync(dispatch_get_main_queue(), ^{
            CGPoint location = {rpevent.touches[0].x, rpevent.touches[0].y},
                location2 = {rpevent.touches[1].x, rpevent.touches[1].y};
            UIEvent *fakeEvent = (UIEvent *)[[BCEvent alloc] init];
            static UITextAutocorrectionType saveAuto;
            static UITouch *currentTouch2;
            static UIView *currentTarget;
            static UITouchesEvent *event;

            switch ( rpevent.phase ) {

                case RMTouchBeganDouble:

                    currentTarget = nil;
                    for ( UIWindow *window in [[UIApplication sharedApplication] windows] ) {
                        UIView *found = [window hitTest:location2 withEvent:nil];
                        if ( found )
                            currentTarget = found;
                    }

                    RMLog( @"Double Target selected: %@", currentTarget );

                    currentTouch2 = [[UITouch alloc] init];

                    [currentTouch2 setWindow:currentTarget.window];
                    [currentTouch2 setView:currentTarget];

                    [currentTouch2 setIsTap:1];
                    [currentTouch2 setTapCount:1];
                    //[currentTouch2 _setIsFirstTouchForView:1];

                    [currentTouch2 setPhase:UITouchPhaseBegan];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:[NSDate timeIntervalSinceReferenceDate]];

                case RMTouchBegan:
                    currentTarget = nil;
                    for ( UIWindow *window in [[UIApplication sharedApplication] windows] ) {
                        UIView *found = [window hitTest:location withEvent:nil];
                        if ( found )
                            currentTarget = found;
                    }

                    RMLog( @"Target selected: %@", currentTarget );
                    if ( [currentTarget respondsToSelector:@selector(setAutocorrectionType:)] ) {
                        UITextField *textField = (UITextField *)currentTarget;
                        saveAuto = textField.autocorrectionType;
                        textField.autocorrectionType = UITextAutocorrectionTypeNo;
                    }

                    currentTouch = [[UITouch alloc] init];

                    [currentTouch setWindow:currentTarget.window];
                    [currentTouch setView:currentTarget];

                    [currentTouch setIsTap:1];
                    [currentTouch setTapCount:1];
                    //[self _setIsFirstTouchForView:1];

                    [currentTouch setPhase:UITouchPhaseBegan];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:[NSDate timeIntervalSinceReferenceDate]];

                    currentTouches = [NSSet setWithObjects:currentTouch, currentTouch2, nil];

                    if ( !event )
                        event = [[objc_getClass("UITouchesEvent") alloc] _init];
                    [event _clearTouches];
                    [event _addTouch:currentTouch forDelayedDelivery:NO];
                    if ( currentTouch2 )
                        [event _addTouch:currentTouch2 forDelayedDelivery:NO];

                    [[UIApplication sharedApplication] in_sendEvent:(UIEvent *)event];
                    [currentTarget touchesBegan:currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchMoved:
                case RMTouchStationary:
                    [currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:[NSDate timeIntervalSinceReferenceDate]];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:[NSDate timeIntervalSinceReferenceDate]];

                    [event _clearTouches];
                    [event _addTouch:currentTouch forDelayedDelivery:NO];
                    if ( currentTouch2 )
                        [event _addTouch:currentTouch2 forDelayedDelivery:NO];

                    [[UIApplication sharedApplication] in_sendEvent:(UIEvent *)event];
                    [currentTarget touchesMoved:currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchEnded:
                case RMTouchCancelled:
                    [currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:[NSDate timeIntervalSinceReferenceDate]];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:[NSDate timeIntervalSinceReferenceDate]];

                    [event _clearTouches];
                    [event _addTouch:currentTouch forDelayedDelivery:NO];
                    if ( currentTouch2 )
                        [event _addTouch:currentTouch2 forDelayedDelivery:NO];

                    [[UIApplication sharedApplication] in_sendEvent:(UIEvent *)event];
                    [currentTarget touchesEnded:currentTouches withEvent:fakeEvent];

                    if ( [currentTarget respondsToSelector:@selector(setAutocorrectionType:)] ) {
                        UITextField *textField = (UITextField *)currentTarget;
                        textField.autocorrectionType = saveAuto;
                    }

                    currentTouch = currentTouch2 = nil;
                    currentTarget = nil;
                    break;
                    
                default:
                    NSLog( @"RemoteCapture: Invalid Event: %d", rpevent.phase );
            }
        });
    }
    
    NSLog( @"RemoteCapture: processEvents exits" );
    fclose( eventStream );
    connectionSocket = 0;
}

@end

@implementation NSObject(RemoteCapture)

- (void *)in_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2 {
    void *out = [self in_copyRenderLayer:a0 layerFlags:a1 commitFlags:a2];
    if ( connectionSocket && [self isKindOfClass:UIWindowLayer] && --skipEcho<0 && !pending++ )
        [RemoteCapture performSelectorOnMainThread:@selector(capture) withObject:nil waitUntilDone:NO];
    return  out;
}

- (void)in_didCommitLayer:(void *)a0 {
    [self in_didCommitLayer:a0];
    if ( connectionSocket && [self isKindOfClass:UIWindowLayer] && --skipEcho<0 && !pending++ )
        [RemoteCapture performSelectorOnMainThread:@selector(capture) withObject:nil waitUntilDone:NO];
}

@end

@implementation UIApplication(RemoteCapture)

- (void)in_sendEvent:(UIEvent *)event {
    [self in_sendEvent:event];
    NSSet *touches = event.allTouches;
#if 0
    RMLog( @"%@", event );
    for ( UITouch *t in touches )
        RMLog( @"Gestures: %@", t.gestureRecognizers );
#endif

    struct _rmframe header;
    header.length = (int)-touches.count;

    for ( UITouch *touch in touches ) {
        CGPoint loc = [touch _locationInWindow:touch.window];
        header.phase = (RMTouchPhase)touch.phase;
        header.x = loc.x;
        header.y = loc.y;
        dispatch_async(writeQueue, ^{
            write(connectionSocket, &header, sizeof header);
        });
        header.length++;
    }
}

#endif
@end
#endif