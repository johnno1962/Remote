//
//  RemoteCapture.h
//  Remote
//
//  Created by John Holdsworth on 14/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//
//  Repo: https://github.com/johnno1962/Remote
//  $Id: //depot/Remote/Classes/RemoteCapture.h#58 $
//  

#import <sys/sysctl.h>
#import <netinet/tcp.h>
#import <sys/socket.h>
#import <arpa/inet.h>
#import <netdb.h>
#import <zlib.h>

#ifndef REMOTE_PORT
#define INJECTION_PORT 31442
#define APPCODE_PORT 31444
#define XPROBE_PORT 31448
#define REMOTE_PORT 31449
#endif

#define REMOTE_APPNAME "Remote"
#define REMOTE_MAGIC -141414141
#define REMOTE_MINDIFF (3*sizeof(rmencoded_t))
#define REMOTE_COMPRESSED_OFFSET 1000000000

#ifdef DEBUG
#define RMLog NSLog
#else
#define RMLog while(0) NSLog
#endif

#ifdef REMOTE_DEBUG
#define RMDebug NSLog
#else
#define RMDebug while(0) NSLog
#endif

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

BOOL remoteLegacy = FALSE;

typedef unsigned rmpixel_t;
typedef unsigned rmencoded_t;

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

struct _rmcompress {
    unsigned bytes; unsigned char data[1];
};

struct _rmevent {
    RMTouchPhase phase;
    union {
        struct { float x, y; } touches[RMMAX_TOUCHES];
    };
};

struct _rmdevice {
    char version;
    char machine[24];
    char appname[64];
    char appvers[24];
    char hostname[63];
    float scale;
    int isIPad;
    char expansion[64];
    int magic;
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
    rmpixel_t *buffer, *buffend;
    CGContextRef cg;
}

+ (BOOL)startCapture:(NSString *)addrs;

@end

@protocol RemoteDelegate <NSObject>
@required
- (void)remoteConnected:(BOOL)status;
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
    if ((self = [super init])) {
        _timestamp = [NSDate timeIntervalSinceReferenceDate];
    }
    return self;
}

- (NSTimeInterval)timestamp {
    return _timestamp;
}

- (UITouch *)_firstTouchForView:(UIView *)view {
    RMLog(@"_firstTouchForView: %@", view);
    return currentTouch;
}

- (NSSet *)touchesForView:(UIView *)view {
    RMLog(@"touchesForWindow:%@", view);
    return currentTouches;
}

- (NSSet *)touchesForWindow:(UIWindow *)window {
    RMLog(@"touchesForWindow:%@", window);
    return currentTouches;
}

- (NSSet *)touchesForGestureRecognizer:(UIGestureRecognizer *)rec {
    RMLog(@"touchesForGestureRecognizer:%@", rec);
    return currentTouches;
}

- (void)_removeTouch:(UITouch *)touch fromGestureRecognizer:(UIGestureRecognizer *)rec {
    RMLog(@"_removeTouch:%@ fromGestureRecognizer:%@", touch, rec);
}

- (NSSet *)allTouches {
    return currentTouches;
}

- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 {
    RMLog(@"_addWindowAwaitingLatentSystemGestureNotification:%@ deliveredToEventWindow:%@", a0, a1);
}

- (NSUInteger)_buttonMask {
    return 0;
}

- (long)type {
    return 0;
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
    if ((self = [super init])) {
        CGSize size = { frame->width*frame->imageScale, frame->height*frame->imageScale };
        int bitsPerComponent = 8, bytesPerRow = bitsPerComponent/8*4 * (int)size.width;
        int bufferSize = bytesPerRow * (int)size.height;
        cg = CGBitmapContextCreate(NULL, size.width, size.height,
                                   bitsPerComponent, bytesPerRow, CGColorSpaceCreateDeviceRGB(),
                                   (CGBitmapInfo)kCGImageAlphaNoneSkipFirst);
        buffer = (rmpixel_t *)CGBitmapContextGetData(cg);
        buffend = (rmpixel_t *)((char *)buffer + bufferSize);
        if (remoteLegacy) {
            CGContextTranslateCTM(cg, 0, size.height);
            CGContextScaleCTM(cg, frame->imageScale, -frame->imageScale);
        }
        else
            CGContextScaleCTM(cg, frame->imageScale, frame->imageScale);
    }
    return self;
}

- (NSData *)subtractAndEncode:(RemoteCapture *)prevbuff {
    unsigned tmpsize = 16*1024;
    rmencoded_t *tmp = (rmencoded_t *)malloc(tmpsize*sizeof *tmp), *end = tmp + tmpsize;

    rmencoded_t *out = tmp, count = 0, expectedDiff = 0, check = 0;
    for (rmpixel_t *curr = buffer, *prev = prevbuff ? prevbuff->buffer : NULL ;
             curr < buffend ; check += *curr, curr++) {
        rmpixel_t ref = (prev ? *prev++ : 0), diff = *curr - ref - expectedDiff;
        if (!diff && curr != buffer)
            count++;
        else {
            if (count) {
                if (count < 0xff)
                    out[-1] |= count;
                else {
                    out[-1] |= 0xff;
                    *out++ = count;
                }
                count = 0;
            }

            *out++ = diff & 0xffffff00;

            if (out + 4 >= end) {
                size_t ptr = out - tmp;
                tmpsize *= 1.5;
                tmp = (rmencoded_t *)realloc(tmp, tmpsize * sizeof *tmp);
                out = tmp + ptr;
                end = tmp + tmpsize;
            }
        }

        expectedDiff = *curr - ref;
    }

    if (count) {
        if (count < 0xff)
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
    return CGBitmapContextCreateImage(cg);
}

- (void)dealloc {
    CGContextRelease(cg);
}

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED

static id<RemoteDelegate> remoteDelegate;

#ifdef REMOTEPLUGIN_SERVERIPS
+ (void)load {
    [self performSelectorInBackground:@selector(startCapture:) withObject:@REMOTEPLUGIN_SERVERIPS];
}
#endif

+ (BOOL)startCapture:(NSString *)addrs {
    for (NSString *addr in [addrs componentsSeparatedByString:@" "]) {
        NSArray<NSString *> *parts = [addr componentsSeparatedByString:@":"];
        NSString *inaddr = parts[0];
        in_port_t port = REMOTE_PORT;
        if (parts.count > 1)
            port = (in_port_t)parts[1].intValue;
        int remoteSocket = [self connectIPV4:[inaddr UTF8String] port:port];
        if (remoteSocket) {
            NSLog(@"RemoteCapture: Connected.");
            [self runCaptureOnSocket:remoteSocket];
            return TRUE;
        }
    }
    return FALSE;
}

+ (int)connectIPV4:(const char *)ipAddress port:(in_port_t)port {
    static struct sockaddr_in remoteAddr;

    remoteAddr.sin_len = sizeof remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(port);

    if (isdigit(ipAddress[0]))
        inet_aton(ipAddress, &remoteAddr.sin_addr);
    else {
        struct hostent *ent = gethostbyname2(ipAddress, remoteAddr.sin_family);
        if (ent)
            memcpy(&remoteAddr.sin_addr, ent->h_addr_list[0], sizeof remoteAddr.sin_addr);
        else {
            NSLog(@"RemoteCapture: Could not look up host '%s'", ipAddress);
            return 0;
        }
    }

    NSLog(@"RemoteCapture: %s attempting connection to: %s:%d", REMOTE_APPNAME, ipAddress, port);
    return [self connectAddr:(struct sockaddr *)&remoteAddr];
}

+ (int)connectAddr:(struct sockaddr *)remoteAddr {
    int remoteSocket, optval = 1;
    if ((remoteSocket = socket(remoteAddr->sa_family, SOCK_STREAM, 0)) < 0)
        NSLog(@"RemoteCapture: %s: Could not open socket for injection: %s", REMOTE_APPNAME, strerror(errno));
    else if (setsockopt(remoteSocket, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval)) < 0)
        NSLog(@"RemoteCapture: %s: Could not set TCP_NODELAY: %s", REMOTE_APPNAME, strerror(errno));
    else if (connect(remoteSocket, remoteAddr, remoteAddr->sa_len) >= 0)
        return remoteSocket;

    NSLog(@"RemoteCapture: %s: Could not connect: %s", REMOTE_APPNAME, strerror(errno));
    close(remoteSocket);
    return 0;
}

static NSArray<UIScreen *> *screens;
static dispatch_queue_t writeQueue;
static NSTimeInterval timeStamp0;
static struct _rmdevice device;
static int connectionSocket;
static Class UIWindowLayer;
static CGSize bufferSize;

+ (void)runCaptureOnSocket:(int)remoteSocket {
    connectionSocket = remoteSocket;
    bufferSize.width = 0.0;

    while (!(screens = [UIScreen screens]).count)
        [NSThread sleepForTimeInterval:.5];

    if (!UIWindowLayer) {
        UIWindowLayer = objc_getClass("UIWindowLayer");
#if 01
        method_exchangeImplementations(class_getInstanceMethod(objc_getClass("CALayer"), @selector(_copyRenderLayer:layerFlags:commitFlags:)),
                                       class_getInstanceMethod([NSObject class], @selector(in_copyRenderLayer:layerFlags:commitFlags:)));
#else
        method_exchangeImplementations(class_getInstanceMethod(objc_getClass("CALayer"), @selector(_didCommitLayer:)),
                                       class_getInstanceMethod([NSObject class], @selector(in_didCommitLayer:)));
#endif

        method_exchangeImplementations(class_getInstanceMethod([UIApplication class], @selector(sendEvent:)),
                                       class_getInstanceMethod([UIApplication class], @selector(in_sendEvent:)));
    }

    device.version = 1;

    size_t size = sizeof device.machine-1;
    sysctlbyname("hw.machine", device.machine, &size, NULL, 0);
    device.machine[size] = '\000';

    NSDictionary *infoDict = [NSBundle mainBundle].infoDictionary;
    strncpy(device.appname, [infoDict[@"CFBundleIdentifier"] UTF8String]?:"", sizeof device.appname-1);
    strncpy(device.appvers, [infoDict[@"CFBundleShortVersionString"] UTF8String]?:"", sizeof device.appvers-1);

    gethostname(device.hostname, sizeof device.hostname-1);

    device.scale = [screens[0] scale];
    device.isIPad = [[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad;
    device.magic = REMOTE_MAGIC;

    timeStamp0 = [NSDate timeIntervalSinceReferenceDate];

    if (write(connectionSocket, &device, sizeof device) != sizeof device)
        NSLog(@"RemoteCapture: Could not write device info");

    if (!writeQueue)
        writeQueue = dispatch_queue_create("writeQueue", DISPATCH_QUEUE_SERIAL);

    NSOperatingSystemVersion minimumVersion = {13, 0, 0};
    if (![[NSProcessInfo new] isOperatingSystemAtLeastVersion:minimumVersion])
        remoteLegacy = TRUE;

    [remoteDelegate remoteConnected:TRUE];
    [self performSelectorInBackground:@selector(processEvents) withObject:nil];
    [self performSelectorOnMainThread:@selector(capture:) withObject:nil waitUntilDone:NO];
}

static int skipEcho, pending;
static BOOL capturing, displayedKeyboard;
static NSTimeInterval mostRecentScreenUpdate;

+ (void)capture:(NSNumber *)timestamp {
    RMDebug(@"capture: %f %f", timestamp.doubleValue, mostRecentScreenUpdate);
    if (timestamp.doubleValue < mostRecentScreenUpdate)
        return;
    UIScreen *screen = screens[0];
    CGRect bounds = screen.bounds;
    CGFloat imageScale = device.isIPad || device.scale == 3. ? 1. : screen.scale;
    __block struct _rmframe frame = { (float)bounds.size.width, (float)bounds.size.height, (float)imageScale, 0 };

    static NSArray *buffers;
    static int frameno;

    if (bufferSize.width != frame.width || bufferSize.height != frame.height) {
        buffers = nil;
        buffers = @[[[RemoteCapture alloc] initFrame:&frame],
                    [[RemoteCapture alloc] initFrame:&frame]];
        bufferSize = bounds.size;
        frameno = 0;
    }

    RemoteCapture *buffer = buffers[frameno++%2];
    RemoteCapture *prevbuff = buffers[frameno%2];

//    memset(buffer->buffer, 128, (char *)buffer->buffend - (char *)buffer->buffer);

    if (remoteLegacy) {
        RMDebug(@"CAPTURE LEGACY");
        BOOL benchmark = FALSE;
        for (UIWindow *window in [[UIApplication sharedApplication] windows]) {
            NSTimeInterval start = [NSDate timeIntervalSinceReferenceDate];
#if 0
            UIView *snap = [window snapshotViewAfterScreenUpdates:YES];
            [snap.layer renderInContext:buffer->cg];
#else
            [[window layer] renderInContext:buffer->cg];
#endif
            if(benchmark)
                RMLog(@"%@ %f", NSStringFromCGRect(window.bounds),
                      [NSDate timeIntervalSinceReferenceDate]-start);
        }
        skipEcho = 2;
    }
    else {
        capturing = TRUE;
        RMDebug(@"CAPTURE0");
        UIScreen *mainScreen = [UIScreen mainScreen];
        CGSize screenSize = mainScreen.bounds.size;
        UIView *snapshotView = [mainScreen snapshotViewAfterScreenUpdates:YES];
        RMDebug(@"CAPTURE1");
        UIGraphicsBeginImageContextWithOptions(screenSize, YES, 0);
        [snapshotView drawViewHierarchyInRect:snapshotView.bounds afterScreenUpdates:YES];
        UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        RMDebug(@"CAPTURE2 %@", [UIApplication sharedApplication].windows.lastObject);
        CGContextDrawImage(buffer->cg, CGRectMake(0, 0, screenSize.width, screenSize.height), image.CGImage);
        BOOL displayingKeyboard = [[UIApplication sharedApplication].windows.lastObject
                                   isKindOfClass:objc_getClass("UIRemoteKeyboardWindow")];
        if (displayingKeyboard)
            displayedKeyboard = TRUE;
        skipEcho = displayedKeyboard ? displayingKeyboard ? 10 : 8 : 6;
        capturing = FALSE;

        BOOL emptyImage = TRUE;
        for (rmpixel_t *curr = buffer->buffer; curr < buffer->buffend; curr++)
            if (curr[0] & 0xffffff00) {
                emptyImage = FALSE;
                break;
            }
        if (emptyImage) {
            frameno--;
            [self performSelector:@selector(capture:) withObject:timestamp afterDelay:0.1];
            return;
        }
    }

    pending = 0;
    unsigned thisFrameno = frameno;

    dispatch_async(writeQueue, ^{
        if (thisFrameno < frameno)
            return;
        if (timestamp.doubleValue < mostRecentScreenUpdate) {
            frameno--;
            return;
        }
        NSData *out = [buffer subtractAndEncode:prevbuff];
        frame.length = (unsigned)out.length;
        if (frame.length <= REMOTE_MINDIFF)
            return;

#ifdef REMOTE_COMPRESSION
        struct _rmcompress *buff = malloc(sizeof buff->bytes+out.length+100);
        uLongf clen = buff->bytes = (unsigned)out.length;
        if (compress2(buff->data, &clen,
                       (const Bytef *)out.bytes, buff->bytes, Z_BEST_SPEED) == Z_OK && clen < out.length) {
            out = [NSMutableData dataWithBytesNoCopy:buff length:sizeof buff->bytes+clen freeWhenDone:YES];
            RMLog(@"Remote: Delta image %d/%d %.1f%%", (int)out.length, frame.length, 100.*out.length/frame.length);
            frame.length = REMOTE_COMPRESSED_OFFSET + (int)out.length;
        }
#endif
        if (write(connectionSocket, &frame, sizeof frame) != sizeof frame)
            NSLog(@"RemoteCapture: Could not write bounds");
        else if (write(connectionSocket, out.bytes, out.length) != out.length)
            NSLog(@"RemoteCapture: Could not write out");
    });
}

//static UITouchesEvent *realEvent;

+ (void)processEvents {
    FILE *eventStream = fdopen(connectionSocket, "r");

    struct _rmevent rpevent;
    while (fread(&rpevent, 1, sizeof rpevent, eventStream) == sizeof rpevent) {

        RMLog(@"Remote Event: %f %f %d", rpevent.touches[0].x, rpevent.touches[0].y, rpevent.phase);

        if (rpevent.phase == RMTouchMoved && capturing)
            continue;

        dispatch_sync(dispatch_get_main_queue(), ^{
            CGPoint location = {rpevent.touches[0].x, rpevent.touches[0].y},
                location2 = {rpevent.touches[1].x, rpevent.touches[1].y};
            UIEvent *fakeEvent = (UIEvent *)[[BCEvent alloc] init];
            static UITextAutocorrectionType saveAuto;
            static BOOL isTextfield, isKeyboard;
            static UITouch *currentTouch2;
            static UIView *currentTarget;

            static UITouchesEvent *event = nil;
            if (!event) {
                event = [[objc_getClass("UITouchesEvent") alloc] _init];
//                static char aPointer[1000];
//                [event _setHIDEvent:(__IOHIDEvent *)aPointer];
            }

            NSTimeInterval timeStamp = [NSDate
                                        timeIntervalSinceReferenceDate];

            switch (rpevent.phase) {

                case RMTouchBeganDouble:

                    currentTarget = nil;
                    for (UIWindow *window in [[UIApplication sharedApplication] windows]) {
                        UIView *found = [window hitTest:location2 withEvent:nil];
                        if (found)
                            currentTarget = found;
                    }

                    RMLog(@"Double Target selected: %@", currentTarget);

                    currentTouch2 = [[UITouch alloc] init];

                    [currentTouch2 setWindow:currentTarget.window];
                    [currentTouch2 setView:currentTarget];

                    [currentTouch2 setIsTap:1];
                    [currentTouch2 setTapCount:1];
                    //[currentTouch2 _setIsFirstTouchForView:1];

                    [currentTouch2 setPhase:UITouchPhaseBegan];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:timeStamp];

                case RMTouchBegan:
                    currentTarget = nil;
                    for (UIWindow *window in [[UIApplication sharedApplication] windows]) {
                        UIView *found = [window hitTest:location withEvent:nil];
                        if (found)
                            currentTarget = found;
                    }

                    isTextfield = [currentTarget
                                   respondsToSelector:@selector(setAutocorrectionType:)];
                    isKeyboard = [currentTarget
                                  isKindOfClass:objc_getClass("UIKeyboardLayoutStar")];

                    RMLog(@"Target selected: %@ %d %d",
                          currentTarget, isTextfield, isKeyboard);
                    if (isTextfield) {
                        UITextField *textField = (UITextField *)currentTarget;
                        saveAuto = textField.autocorrectionType;
                        textField.autocorrectionType = UITextAutocorrectionTypeNo;
                    }

                    if (!currentTouch) {
                        NSOperatingSystemVersion minimumVersion = {8, 4, 0};
                        if ([[NSProcessInfo new] isOperatingSystemAtLeastVersion:minimumVersion])
                            NSLog(@"RemoteCapture: *** Initial event from device required for iOS 8.4+ ***");
                        currentTouch = [[UITouch alloc] init];
                    }

                    [currentTouch setWindow:currentTarget.window];
                    [currentTouch setView:currentTarget];

                    [currentTouch setIsTap:1];
                    [currentTouch setTapCount:1];
                    //[self _setIsFirstTouchForView:1];

                    [currentTouch setPhase:UITouchPhaseBegan];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:timeStamp];

                    currentTouches = [NSSet setWithObjects:currentTouch, currentTouch2, nil];

                    [event _clearTouches];
                    [event _addTouch:currentTouch forDelayedDelivery:NO];
                    if (currentTouch2)
                        [event _addTouch:currentTouch2 forDelayedDelivery:NO];

                    [[UIApplication sharedApplication] in_sendEvent:(UIEvent *)event];
                    if (!isKeyboard)
                        [currentTarget touchesBegan:currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchMoved:
                case RMTouchStationary:
                    [currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:timeStamp];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:timeStamp];

                    [[UIApplication sharedApplication] in_sendEvent:(UIEvent *)event];
                    if (!isKeyboard)
                        [currentTarget touchesMoved:currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchEnded:
                case RMTouchCancelled:
                    [currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:timeStamp];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:timeStamp];

                    [[UIApplication sharedApplication] in_sendEvent:(UIEvent *)event];
                    if (!isKeyboard)
                        [currentTarget touchesEnded:currentTouches withEvent:fakeEvent];

                    if (saveAuto) {
                        UITextField *textField = (UITextField *)currentTarget;
                        textField.autocorrectionType = saveAuto;
                        saveAuto = 0;
                    }

                    currentTouches = nil;
                    currentTarget = nil;
                    currentTouch2 = nil;
                    currentTouch = nil;
                    event = nil;
                    break;
                    
                default:
                    NSLog(@"RemoteCapture: Invalid Event: %d", rpevent.phase);
            }
        });
    }
    
    NSLog(@"RemoteCapture: processEvents exits");
    fclose(eventStream);
    [self shutdown];
}

+ (void)shutdown {
    if (connectionSocket) {
        [remoteDelegate remoteConnected:FALSE];
        close(connectionSocket);
        connectionSocket = 0;
    }
}

+ (void)capture0:(NSNumber *)timestamp {
//    if (!pending++)
        dispatch_async(writeQueue, ^{
            [RemoteCapture performSelectorOnMainThread:@selector(capture:) withObject:timestamp waitUntilDone:NO];
        });
}

@end

@implementation NSObject(RemoteCapture)

- (void *)in_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2 {
    void *out = [self in_copyRenderLayer:a0 layerFlags:a1 commitFlags:a2];
    RMDebug(@"in_copyRenderLayer: %d %d %@ %lu", capturing, skipEcho, self,
            (unsigned long)[UIApplication sharedApplication].windows.count);
    if (connectionSocket && !capturing && self.class == UIWindowLayer && --skipEcho<0) {
        [RemoteCapture performSelectorOnMainThread:@selector(capture0:)
            withObject:[NSNumber numberWithDouble:mostRecentScreenUpdate =
                        [NSDate timeIntervalSinceReferenceDate]] waitUntilDone:NO];
    }
    return out;
}

- (void)in_didCommitLayer:(void *)a0 {
    [self in_didCommitLayer:a0];
    RMDebug(@"in_didCommitLayer: %d %d %@ %lu", capturing, skipEcho, self,
            (unsigned long)[UIApplication sharedApplication].windows.count);
    if (connectionSocket && !capturing && self.class == UIWindowLayer && --skipEcho<0)
        [RemoteCapture performSelectorOnMainThread:@selector(capture0:)
            withObject:[NSNumber numberWithDouble:mostRecentScreenUpdate =
                        [NSDate timeIntervalSinceReferenceDate]] waitUntilDone:NO];
}

@end

@implementation UIApplication(RemoteCapture)

- (void)in_sendEvent:(UIEvent *)anEvent {
    [self in_sendEvent:anEvent];
    NSSet *touches = anEvent.allTouches;

//    if ([anEvent isKindOfClass:objc_getClass("UITouchesEvent")]) {
////        realEvent = (UITouchesEvent *)anEvent;
//    //    if (!currentTouch)
////            currentTouch = [touches anyObject];
//    }

#if 0
    RMLog(@"%@", anEvent);
    for (UITouch *t in touches)
        RMLog(@"Gestures: %@", t.gestureRecognizers);
#endif

    struct _rmframe header;
    header.length = (int)-touches.count;

    for (UITouch *touch in touches) {
        CGPoint loc = [touch _locationInWindow:touch.window];
        header.phase = (RMTouchPhase)touch.phase;
        header.x = loc.x;
        header.y = loc.y;
        NSData *out = [NSData dataWithBytes:&header length:sizeof header];
        dispatch_async(writeQueue, ^{
            if (write(connectionSocket, out.bytes, out.length) != out.length)
                NSLog(@"RemoteCapture: Could not write event");
        });
        header.length++;
    }
}

#endif
@end
#endif
