//
//  RemoteCapture.h
//  Remote
//
//  Created by John Holdsworth on 14/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//
//  Repo: https://github.com/johnno1962/Remote
//  $Id: //depot/Remote/Sources/RemoteCapture/include/RemoteCapture.h#29 $
//

#import <sys/sysctl.h>
#import <netinet/tcp.h>
#import <sys/socket.h>
#import <arpa/inet.h>
#import <netdb.h>
#import <zlib.h>

#import "RemoteHeaders.h"

#define REMOTE_MINICAP

#ifndef REMOTE_PORT
#define INJECTION_PORT 31442
#define APPCODE_PORT 31444
#define XPROBE_PORT 31448
#ifndef __IPHONE_OS_VERSION_MIN_REQUIRED
#define REMOTE_PORT 31449
#else
#define REMOTE_PORT 1313
#endif
#endif

#ifndef REMOTE_APPNAME
#define REMOTE_APPNAME "Remote"
#endif
#define REMOTE_MAGIC -141414141
#define REMOTE_MINDIFF (4*sizeof(rmencoded_t))
#define REMOTE_COMPRESSED_OFFSET 1000000000
#define REMOTE_VERSION 4
#define REMOTE_NOKEY 3
#define REMOTE_KEY @__FILE__
#define REMOTE_XOR 0xc5

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

#ifdef REMOTE_LEGACY
static BOOL remoteLegacy = TRUE;
#else
static BOOL remoteLegacy = FALSE;
#endif

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
    RMTouchRegionEntered,
    RMTouchRegionMoved,
    RMTouchRegionExited
};

#define RMMAX_TOUCHES 2

struct _rmcompress {
    unsigned bytes; unsigned char data[1];
};

struct _rmevent {
    NSTimeInterval timestamp;
    RMTouchPhase phase;
    union {
        struct { float x, y; } touches[RMMAX_TOUCHES];
    };
};

struct _rmdevice {
    char version;
#ifndef REMOTE_MINICAP
    char machine[24];
    char appname[64];
    char appvers[24];
    char hostname[63];
    float scale;
    int isIPad;
    char expansion[64];
    int magic;
#else // REMOTE_MINICAP
// See: https://github.com/openstf/minicap#usage
    char headerSize;
    char pid[4];
    char realWidth[4];
    char realHeight[4];
    char virtualWidth[4];
    char virtualHeight[4];
    unsigned char orientation;
    unsigned char quirks;
#endif
};

struct _rmframe {
    NSTimeInterval timestamp;
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
- (instancetype)initFrame:(const struct _rmframe *)frame;
- (NSData *)subtractAndEncode:(RemoteCapture *)prevbuff;
- (CGImageRef)cgImage;
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

@interface RemoteCapture(Client)
+ (void)startCapture:(NSString *)addrs;
+ (void)shutdown;
@end

static NSTimeInterval timestamp0;
static UITouch *currentTouch;
static NSSet *currentTouches;
static BOOL lateJoiners;

@interface BCEvent : UIEvent {
@public
    NSTimeInterval _timestamp;
}

@end

@implementation BCEvent

- (instancetype)init {
    if ((self = [super init])) {
        _timestamp = [NSDate timeIntervalSinceReferenceDate] - timestamp0;
    }
    return self;
}

- (NSTimeInterval)timestamp {
    return _timestamp;
}

- (UITouch *)_firstTouchForView:(UIView *)view {
    RMDebug(@"_firstTouchForView: %@", view);
    return currentTouch;
}

- (NSSet *)touchesForView:(UIView *)view {
    RMDebug(@"touchesForWindow:%@", view);
    return currentTouches;
}

- (NSSet *)touchesForWindow:(UIWindow *)window {
    RMDebug(@"touchesForWindow:%@", window);
    return currentTouches;
}

- (NSSet *)touchesForGestureRecognizer:(UIGestureRecognizer *)rec {
    RMDebug(@"touchesForGestureRecognizer:%@", rec);
    return currentTouches;
}

- (void)_removeTouch:(UITouch *)touch fromGestureRecognizer:(UIGestureRecognizer *)rec {
    RMDebug(@"_removeTouch:%@ fromGestureRecognizer:%@", touch, rec);
}

- (NSSet *)allTouches {
    return currentTouches;
}

- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 {
    RMDebug(@"_addWindowAwaitingLatentSystemGestureNotification:%@ deliveredToEventWindow:%@", a0, a1);
}

- (NSUInteger)_buttonMask {
    return 0;
}

- (UIEventType)type {
    return 0;
}

@end

@interface UIView(Description)
- (NSString *)recursiveDescription;
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
        CGSize size = {frame->width*frame->imageScale, frame->height*frame->imageScale};
        int bitsPerComponent = 8, bytesPerRow = bitsPerComponent/8*4 * (int)size.width;
        int bufferSize = bytesPerRow * (int)size.height;
        cg = CGBitmapContextCreate(NULL, size.width, size.height, bitsPerComponent,
                                   bytesPerRow, CGColorSpaceCreateDeviceRGB(),
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
    unsigned tmpsize = 64*1024;
    rmencoded_t *tmp = (rmencoded_t *)malloc(tmpsize * sizeof *tmp), *end = tmp + tmpsize;

    rmencoded_t *out = tmp, count = 0, expectedDiff = 0, check = 0;
    *out++ = prevbuff == nil;

    for (const rmpixel_t *curr = buffer, *prev = prevbuff ? prevbuff->buffer : NULL;
             curr < buffend; check += *curr, curr++) {
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

    return [NSData dataWithBytesNoCopy:tmp length:(char *)out - (char *)tmp freeWhenDone:YES];
}

- (CGImageRef)cgImage {
    return CGBitmapContextCreateImage(cg);
}

- (void)dealloc {
    CGContextRelease(cg);
}

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED

static id<RemoteDelegate> remoteDelegate;
static NSMutableArray<NSValue *> *connections;
static char *connectionKey;

#ifdef REMOTEPLUGIN_SERVERIPS
+ (void)load {
    [self startCapture:@REMOTEPLUGIN_SERVERIPS];
}
#endif

+ (void)startCapture:(NSString *)addrs {
    [self performSelectorInBackground:@selector(backgroundConnect:)
                           withObject:addrs];
}

+ (BOOL)backgroundConnect:(NSString *)addrs {
    NSMutableArray *newConnections = [NSMutableArray new];
    for (NSString *addr in [addrs componentsSeparatedByString:@" "]) {
        NSArray<NSString *> *parts = [addr componentsSeparatedByString:@":"];
        NSString *inaddr = parts[0];
        in_port_t port = REMOTE_PORT;
        if (parts.count > 1)
            port = (in_port_t)parts[1].intValue;
        int remoteSocket = [self connectIPV4:inaddr.UTF8String port:port];
        if (remoteSocket) {
            NSLog(@"RemoteCapture: Connected to %@:%d.", inaddr, port);
            FILE *writeFp = fdopen(remoteSocket, "w");
            setbuf(writeFp, NULL);
            [newConnections addObject:[NSValue valueWithPointer:writeFp]];
        }
    }
    if (!newConnections.count)
        return FALSE;

    static dispatch_once_t once;
    dispatch_once(&once, ^{
        [self initCapture];
    });

    int32_t keylen = (int)strlen(connectionKey);
    for (NSValue *fp in newConnections) {
        if (fwrite(&device, 1, sizeof device, fp.pointerValue) != sizeof device)
            NSLog(@"%@: Could not write device info: %s", self, strerror(errno));
#ifndef REMOTE_MINICAP
        else if (fwrite(&keylen, 1, sizeof keylen, fp.pointerValue) != sizeof keylen)
            NSLog(@"%@: Could not write keylen: %s", self, strerror(errno));
        else if (fwrite(connectionKey, 1, keylen, fp.pointerValue) != keylen)
            NSLog(@"%@: Could not write key: %s", self, strerror(errno));
#endif
        else
            [self performSelectorInBackground:@selector(processEvents:) withObject:fp];
    }

    dispatch_async(writeQueue, ^{
        [connections addObjectsFromArray:newConnections];
        [self queueCapture];
        lateJoiners = TRUE;
    });
    return TRUE;
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

    NSLog(@"RemoteCapture: Attempting connection to: %s:%d", ipAddress, port);
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
static struct _rmdevice device;
static NSValue *inhibitEcho;
static Class UIWindowLayer;
static UITouch *realTouch;
static CGSize bufferSize;

+ (void)initCapture {
    connections = [NSMutableArray new];
    timestamp0 = [NSDate timeIntervalSinceReferenceDate];
    connectionKey = strdup(REMOTE_KEY.UTF8String);
    for (size_t i=0, keylen = (int)strlen(connectionKey); i<keylen; i++)
        connectionKey[i] ^= REMOTE_XOR;

    while (!(screens = [UIScreen screens]).count)
        [NSThread sleepForTimeInterval:.5];

    UIWindowLayer = objc_getClass("UIWindowLayer");
#if 01
    method_exchangeImplementations(
        class_getInstanceMethod(CALayer.class, @selector(_copyRenderLayer:layerFlags:commitFlags:)),
        class_getInstanceMethod(CALayer.class, @selector(in_copyRenderLayer:layerFlags:commitFlags:)));
#else
    method_exchangeImplementations(
        class_getInstanceMethod(CALayer.class, @selector(_didCommitLayer:)),
        class_getInstanceMethod(CALayer.class, @selector(in_didCommitLayer:)));
#endif
#ifndef REMOTE_MINICAP
    method_exchangeImplementations(
        class_getInstanceMethod(UIApplication.class, @selector(sendEvent:)),
        class_getInstanceMethod(UIApplication.class, @selector(in_sendEvent:)));

    // set up device description struct
    device.magic = REMOTE_MAGIC;
    device.version = REMOTE_VERSION;

    size_t size = sizeof device.machine-1;
    sysctlbyname("hw.machine", device.machine, &size, NULL, 0);
    device.machine[size] = '\000';

    NSDictionary *infoDict = [NSBundle mainBundle].infoDictionary;
    strncpy(device.appname, [infoDict[@"CFBundleIdentifier"] UTF8String]?:"", sizeof device.appname-1);
    strncpy(device.appvers, [infoDict[@"CFBundleShortVersionString"] UTF8String]?:"", sizeof device.appvers-1);

    gethostname(device.hostname, sizeof device.hostname-1);

    device.scale = [screens[0] scale];
    device.isIPad = [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad;
#else
    device.version = 1;
    device.headerSize = sizeof(device);
    *(uint32_t *)device.pid = getpid();
    *(uint32_t *)device.virtualWidth = screens[0].bounds.size.width;
    *(uint32_t *)device.virtualHeight = screens[0].bounds.size.width;
    *(uint32_t *)device.realWidth = *(uint32_t *)device.virtualWidth * [screens[0] scale];
    *(uint32_t *)device.realHeight = *(uint32_t *)device.virtualHeight * [screens[0] scale];
#endif
    writeQueue = dispatch_queue_create("writeQueue", DISPATCH_QUEUE_SERIAL);
    [remoteDelegate remoteConnected:TRUE];
}

+ (void)processEvents:(NSValue *)writeFp {
    FILE *readFp = fdopen(fileno(writeFp.pointerValue), "r");

    struct _rmevent rpevent;
    while (fread(&rpevent, 1, sizeof rpevent, readFp) == sizeof rpevent) {

        RMLog(@"Remote Event: %f %f %d", rpevent.touches[0].x, rpevent.touches[0].y, rpevent.phase);

        if (rpevent.phase == RMTouchMoved && capturing)
            continue;

        NSTimeInterval timestamp = rpevent.timestamp;
        BCEvent *fakeEvent = [BCEvent new];
        fakeEvent->_timestamp = timestamp;

        dispatch_sync(dispatch_get_main_queue(), ^{
            CGPoint location = {rpevent.touches[0].x, rpevent.touches[0].y},
                location2 = {rpevent.touches[1].x, rpevent.touches[1].y};
            static UITextAutocorrectionType saveAuto;
            static BOOL isTextfield, isKeyboard;
            static UITextField *textField;
            static UITouch *currentTouch2;
            static UIView *currentTarget;
//            static unsigned touchIdentifier = 100000;

            static UITouchesEvent *event;
            if (!event)
                event = [[objc_getClass("UITouchesEvent") alloc] _init];

            inhibitEcho = writeFp;

            switch (rpevent.phase) {

                case RMTouchBeganDouble:

                    currentTarget = nil;
                    for (UIWindow *window in [[UIApplication sharedApplication] windows]) {
                        UIView *found = [window hitTest:location2 withEvent:nil];
                        if (found)
                            currentTarget = found;
                    }

                    RMDebug(@"Double Target selected: %@", currentTarget);

                    currentTouch2 = [UITouch new];

                    [currentTouch2 setTimestamp:timestamp];
                    [currentTouch2 setInitialTouchTimestamp:timestamp];
                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
//                    [currentTouch2 setSentTouchesEnded:false];
                    [currentTouch2 setWindow:currentTarget.window];
//                    [currentTouch2 _setDisplacement:CGSizeMake(0.0, 0.0)];
//                    [currentTouch2 _setWindowServerHitTestWindow:currentTarget.window];
//                    [currentTouch2 _setTouchIdentifier:touchIdentifier];
                    [currentTouch2 _setPathIndex:1];
                    [currentTouch2 _setPathIdentity:2];
//                    [currentTouch2 setMajorRadius:20.0];
//                    [currentTouch2 setMajorRadiusTolerance:5.0];
                    [currentTouch2 _setType:0];
//                    [currentTouch2 _setNeedsForceUpdate:false];
//                    [currentTouch2 _setHasForceUpdate:false];
//                    [currentTouch2 _setForceCorrelationToken:0];
                    [currentTouch2 _setSenderID:778835616971358211];
                    [currentTouch2 _setZGradient:0.0];
//                    [currentTouch2 _setMaximumPossiblePressure:0.0];
                    [currentTouch2 _setEdgeType:0];
                    [currentTouch2 _setEdgeAim:0];

                    //[self _setIsFirstTouchForView:1];
                    [currentTouch2 setView:currentTarget];
//                    [currentTouch2 _setLocation:location preciseLocation:location inWindowResetPreviousLocation:true];
//                    [currentTouch2 _setPressure:0.0 resetPrevious:true];
                    //[UITouch _updateWithChildEvent:0x600001ff81c0];
//                    [currentTouch2 setIsTap:true];
                    [currentTouch2 setTapCount:1];
//                    [currentTouch2 _setIsFirstTouchForView:true];

                    [currentTouch2 _setLocationInWindow:location resetPrevious:YES];

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

                    RMDebug(@"Target selected: %@ %d %d\n%@", currentTarget,
                          isTextfield, isKeyboard, [currentTarget recursiveDescription]);

                    if (currentTarget.superview.class ==
                        objc_getClass("UIKeyboardEmojiCollectionViewCell")) {
                        NSString *emoji = [(UILabel *)currentTarget.subviews[0] text];
                        textField.text = [textField.text stringByAppendingString:emoji];
                    }

                    if (isTextfield) {
                        textField = (UITextField *)currentTarget;
                        saveAuto = textField.autocorrectionType;
                        textField.autocorrectionType = UITextAutocorrectionTypeNo;
                    }

                    if (!currentTouch)
                        currentTouch = realTouch ?: [UITouch new];

                    [currentTouch setTimestamp:timestamp];
                    [currentTouch setInitialTouchTimestamp:timestamp];
                    [currentTouch setPhase:(UITouchPhase)rpevent.phase];
//                    [currentTouch setSentTouchesEnded:false];
                    [currentTouch setWindow:currentTarget.window];
//                    [currentTouch _setDisplacement:CGSizeMake(0.0, 0.0)];
//                    [currentTouch _setWindowServerHitTestWindow:currentTarget.window];
//                    [currentTouch _setTouchIdentifier:touchIdentifier];
                    [currentTouch _setPathIndex:1];
                    [currentTouch _setPathIdentity:2];
//                    [currentTouch setMajorRadius:20.0];
//                    [currentTouch setMajorRadiusTolerance:5.0];
                    [currentTouch _setType:0];
//                    [currentTouch _setNeedsForceUpdate:false];
//                    [currentTouch _setHasForceUpdate:false];
//                    [currentTouch _setForceCorrelationToken:0];
                    [currentTouch _setSenderID:778835616971358211];
                    [currentTouch _setZGradient:0.0];
//                    [currentTouch _setMaximumPossiblePressure:0.0];
                    [currentTouch _setEdgeType:0];
                    [currentTouch _setEdgeAim:0];

                    //[self _setIsFirstTouchForView:1];
                    [currentTouch setView:currentTarget];
//                    [currentTouch _setLocation:location preciseLocation:location inWindowResetPreviousLocation:true];
//                    [currentTouch _setPressure:0.0 resetPrevious:true];
                    //[UITouch _updateWithChildEvent:0x600001ff81c0];
//                    [currentTouch setIsTap:true];
                    [currentTouch setTapCount:1];
//                    [currentTouch _setIsFirstTouchForView:true];

                    [currentTouch _setLocationInWindow:location resetPrevious:YES];

                    currentTouches = [NSSet setWithObjects:currentTouch, currentTouch2, nil];

                    [event _clearTouches];
                    [event _addTouch:currentTouch forDelayedDelivery:NO];
                    if (currentTouch2)
                        [event _addTouch:currentTouch2 forDelayedDelivery:NO];

                    [[UIApplication sharedApplication] sendEvent:(UIEvent *)event];
//                    if (!isKeyboard)
                        [currentTarget touchesBegan:currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchMoved:
                case RMTouchStationary:
                    [currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:timestamp];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:timestamp];

                    [[UIApplication sharedApplication] sendEvent:(UIEvent *)event];
//                    if (!isKeyboard)
                        [currentTarget touchesMoved:currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchEnded:
                case RMTouchCancelled:
                    [currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [currentTouch setTimestamp:timestamp];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:timestamp];

                    [[UIApplication sharedApplication] sendEvent:(UIEvent *)event];
//                    if (!isKeyboard)
                        [currentTarget touchesEnded:currentTouches withEvent:fakeEvent];

                    if (isTextfield) {
                        UITextField *textField = (UITextField *)currentTarget;
                        textField.autocorrectionType = saveAuto;
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

            inhibitEcho = nil;
        });
    }

    NSLog(@"RemoteCapture: processEvents exits");
    fclose(readFp);

    [connections removeObject:writeFp];
    fclose(writeFp.pointerValue);
    if (!connections.count)
        [self shutdown];
}

+ (void)shutdown {
    [remoteDelegate remoteConnected:FALSE];
    for (NSValue *writeFp in connections)
        fclose(writeFp.pointerValue);
    connections = nil;
}

static int skipEcho;
static BOOL capturing;
static NSTimeInterval mostRecentScreenUpdate;

+ (CGRect)screenBounds {
    CGRect bounds = CGRectZero;
    while (TRUE) {
        for (UIWindow *window in UIApplication.sharedApplication.windows)  {
            if (window.bounds.size.height > bounds.size.height)
                bounds = window.bounds;
        }
        if (bounds.size.height)
            break;
        else
            [[NSRunLoop mainRunLoop]
             runUntilDate:[NSDate dateWithTimeIntervalSinceNow:.2]];
    }
    return bounds;
}

+ (void)capture:(NSNumber *)timestamp {
//    RMDebug(@"capture: %f %f", timestamp.doubleValue, mostRecentScreenUpdate);
    if (timestamp.doubleValue < mostRecentScreenUpdate)
        return;
    UIScreen *screen = [UIScreen mainScreen];
    CGRect screenBounds = [self screenBounds];
    CGSize screenSize = screenBounds.size;
#ifndef REMOTE_MINICAP
    CGFloat imageScale = device.isIPad || device.scale == 3. ? 1. : screen.scale;
#else
    CGFloat imageScale = 1.0;
#endif
    __block struct _rmframe frame = {[NSDate timeIntervalSinceReferenceDate],
        {{(float)screenSize.width, (float)screenSize.height, (float)imageScale}}, 0};

    static NSArray *buffers;
    static int frameno;

    if (bufferSize.width != frame.width || bufferSize.height != frame.height) {
        buffers = nil;
        buffers = @[[[RemoteCapture alloc] initFrame:&frame],
                    [[RemoteCapture alloc] initFrame:&frame]];
        bufferSize = screenSize;
        frameno = 0;
    }

    RemoteCapture *buffer = buffers[frameno++&1];
    RemoteCapture *prevbuff = buffers[frameno&1];
    UIImage *screenshot;
    RMDebug(@"%@, %@ -- %@", buffers, buffer, prevbuff);

//    memset(buffer->buffer, 128, (char *)buffer->buffend - (char *)buffer->buffer);

    // The various ways to capture a screenshot over the years...
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
        CGRect screenBounds = [self screenBounds];
        CGSize screenSize = screenBounds.size;
#if 00
        UIView *snapshotView = [keyWindow snapshotViewAfterScreenUpdates:YES];
        RMDebug(@"CAPTURE1");
        UIGraphicsBeginImageContextWithOptions(screenSize, YES, 0);
        [snapshotView drawViewHierarchyInRect:snapshotView.bounds afterScreenUpdates:NO];
        screenshot = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        BOOL displayingKeyboard = [[UIApplication sharedApplication].windows.lastObject
                                   isKindOfClass:objc_getClass("UIRemoteKeyboardWindow")];
        static BOOL displayedKeyboard;
        if (displayingKeyboard)
            displayedKeyboard = TRUE;
        skipEcho = displayedKeyboard ? displayingKeyboard ? 10 : 8 : 6;
#else
//        extern CGImageRef UIGetScreenImage(void);
//        CGImageRef screenshot = UIGetScreenImage();
//        CGContextDrawImage(buffer->cg, CGRectMake(0, 0, screenSize.width, screenSize.height), screenshot);
        UIGraphicsBeginImageContext(screenSize);
        for (UIWindow *window in [UIApplication sharedApplication].windows)
            if (!window.isHidden)
                [window drawViewHierarchyInRect:screenBounds afterScreenUpdates:NO];
        screenshot = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        skipEcho = 0;
#endif
        RMDebug(@"CAPTURE2 %@", [UIApplication sharedApplication].windows.lastObject);
        capturing = FALSE;
    }

    dispatch_async(writeQueue, ^{
        if (timestamp.doubleValue < mostRecentScreenUpdate) {
            frameno--;
            return;
        }

#ifndef REMOTE_MINICAP
        if (screenshot)
            CGContextDrawImage(buffer->cg, CGRectMake(0, 0,
                            screenSize.width, screenSize.height), screenshot.CGImage);

        BOOL blankImage = TRUE;
        for (const rmpixel_t *curr = buffer->buffer; curr < buffer->buffend; curr++)
            if (*curr & 0xffffff00) {
                blankImage = FALSE;
                break;
            }
        if (blankImage) {
            [self performSelector:@selector(queueCapture) withObject:nil afterDelay:0.1];
            frameno--;
            return;
        }

        NSData *encoded = lateJoiners ? nil : [buffer subtractAndEncode:prevbuff];
        NSData *keyframe = [buffer subtractAndEncode:nil];
        if (lateJoiners || keyframe.length < encoded.length)
            encoded = keyframe;
        lateJoiners = FALSE;

        frame.length = (unsigned)encoded.length;
        if (frame.length <= REMOTE_MINDIFF) {
            frameno--;
            return;
        }

#ifdef REMOTE_COMPRESSION
        struct _rmcompress *buff = malloc(sizeof buff->bytes + encoded.length + 100);
        uLongf clen = buff->bytes = (unsigned)encoded.length;
        if (compress2(buff->data, &clen, (const Bytef *)encoded.bytes,
                      buff->bytes, Z_BEST_SPEED) == Z_OK && clen < encoded.length) {
            encoded = [NSMutableData dataWithBytesNoCopy:buff length:sizeof buff->bytes+clen freeWhenDone:YES];
            RMLog(@"Remote: Delta image %d/%d %.1f%%", (int)encoded.length, frame.length, 100.*encoded.length/frame.length);
            frame.length = REMOTE_COMPRESSED_OFFSET + (int)encoded.length;
        }
#endif
#else
        NSData *encoded = UIImageJPEGRepresentation(screenshot, 0.8);
#endif

        for (NSValue *writeFp in connections) {
            FILE *fp = (FILE *)writeFp.pointerValue;
#ifdef REMOTE_MINICAP
            uint32_t frame = (uint32_t)encoded.length;
#endif
            if (fwrite(&frame, 1, sizeof frame, fp) != sizeof frame)
                NSLog(@"RemoteCapture: Could not write bounds: %s", strerror(errno));
            else if (fwrite(encoded.bytes, 1, encoded.length, fp) != encoded.length)
                NSLog(@"RemoteCapture: Could not write encoded: %s", strerror(errno));
        }
    });
}

+ (void)captureSynchronized:(NSNumber *)timestamp {
    dispatch_async(writeQueue, ^{
        [RemoteCapture performSelectorOnMainThread:@selector(capture:) withObject:timestamp waitUntilDone:NO];
    });
}

+ (void)queueCapture {
    RMDebug(@"queueCapture");
    [RemoteCapture performSelectorOnMainThread:@selector(captureSynchronized:)
        withObject:[NSNumber numberWithDouble:mostRecentScreenUpdate =
                    [NSDate timeIntervalSinceReferenceDate]] waitUntilDone:NO];
}

@end

@implementation CALayer(RemoteCapture)

- (void)queueCapture {
    if (connections.count && !capturing && self.class == UIWindowLayer && --skipEcho<0)
        [RemoteCapture queueCapture];
    else {
        [RemoteCapture cancelPreviousPerformRequestsWithTarget:RemoteCapture.class];
        [RemoteCapture performSelector:@selector(queueCapture) withObject:nil afterDelay:0.8];
    }
}

- (void *)in_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2 {
    void *out = [self in_copyRenderLayer:a0 layerFlags:a1 commitFlags:a2];
    RMDebug(@"in_copyRenderLayer: %d %d %@ %lu", capturing, skipEcho, self,
            (unsigned long)[UIApplication sharedApplication].windows.count);
    [self queueCapture];
    return out;
}

- (void)in_didCommitLayer:(void *)a0 {
    [self in_didCommitLayer:a0];
    RMDebug(@"in_didCommitLayer: %d %d %@ %lu", capturing, skipEcho, self,
            (unsigned long)[UIApplication sharedApplication].windows.count);
    [self queueCapture];
}

@end

@implementation UIApplication(RemoteCapture)

- (void)in_sendEvent:(UIEvent *)anEvent {
    [self in_sendEvent:anEvent];
    NSSet *touches = anEvent.allTouches;
    NSValue *incomingFp = inhibitEcho;

#if 0
    RMLog(@"%@", anEvent);
    for (UITouch *t in touches)
        RMLog(@"Gestures: %@", t.gestureRecognizers);
#endif

    struct _rmframe header;
    header.timestamp = [NSDate timeIntervalSinceReferenceDate];
    header.length = (int)-touches.count;

    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:touch.window];
        header.phase = (RMTouchPhase)touch.phase;
        header.x = loc.x;
        header.y = loc.y;
        NSData *out = [NSData dataWithBytes:&header length:sizeof header];
        dispatch_async(writeQueue, ^{
            for (NSValue *fp in connections) {
                if (fp != incomingFp &&
                    fwrite(out.bytes, 1, out.length, fp.pointerValue) != out.length)
                    NSLog(@"RemoteCapture: Could not write event: %s", strerror(errno));
            }
        });
        header.length++;
        realTouch = touch;
    }
}

#endif
@end
#endif
