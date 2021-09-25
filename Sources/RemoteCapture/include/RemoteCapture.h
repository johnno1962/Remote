//
//  RemoteCapture.h
//  Remote
//
//  Created by John Holdsworth on 14/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//
//  Repo: https://github.com/johnno1962/Remote
//  $Id: //depot/Remote/Sources/RemoteCapture/include/RemoteCapture.h#66 $
//
//  For historical reasons all the implementation is in this header file.
//  This was te easiest way for it to be distributed for Objective-C.
//  To use, #define REMOTE_IMPL and #import this file into a <Source>.m
//  and then call [RemoteCapture startCapture:@"hostname"] somewhere.
//
//  Copyright (c) 2014 John Holdsworth
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#import <sys/sysctl.h>
#import <netinet/tcp.h>
#import <sys/socket.h>
#import <arpa/inet.h>
#import <netdb.h>
#import <zlib.h>

#import "RemoteHeaders.h"

#ifndef REMOTE_PORT
#define INJECTION_PORT 31442
#define APPCODE_PORT 31444
#define XPROBE_PORT 31448
#define REMOTE_PORT 31449
#endif

#ifndef REMOTE_APPNAME
#define REMOTE_APPNAME RemoteCapture
#endif
#define REMOTE_MAGIC -141414141
#define REMOTE_MINDIFF (4*sizeof(rmencoded_t))
#define REMOTE_COMPRESSED_OFFSET 1000000000

// May be used for security
#define REMOTE_KEY @__FILE__
#define REMOTE_XOR 0xc5

// Times coordinate-resolution to capture.
#ifndef REMOTE_OVERSAMPLE
#define REMOTE_OVERSAMPLE (core.device.version == HYBRID_VERSION ? \
                           *(float *)core.device.remote.scale : 1.0)
#endif

#ifndef REMOTE_JPEGQUALITY
#define REMOTE_JPEGQUALITY 0.8
#endif

#ifndef REMOTE_RETRIES
#define REMOTE_RETRIES 3
#endif

#ifndef REMOTE_RETRYSLEEP
#define REMOTE_RETRYSLEEP 1.0
#endif

#ifdef REMOTE_HYBRID
// Wait for screen to settle before capture
#ifndef REMOTE_DEFER
#define REMOTE_DEFER 0.5
#endif

// Only wait this long for screen to settle
#ifndef REMOTE_MAXDEFER
#define REMOTE_MAXDEFER 0.1
#endif
#else
// Wait for screen to settle before capture
#ifndef REMOTE_DEFER
#define REMOTE_DEFER 0.5
#endif

// Only wait this long for screen to settle
#ifndef REMOTE_MAXDEFER
#define REMOTE_MAXDEFER 0.1
#endif
#endif

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

#ifndef CONNECTING_FORMAT
#define CONNECTING_FORMAT "%@: Connecting to: %s(%s):%d %@"
#endif

#define REMOTE_NOW [NSDate timeIntervalSinceReferenceDate]
#define RMBench if (params.benchmark) printf

// Various wire formats supported.
typedef NS_ENUM(int, RMFormat) {
    MINICAP_VERSION = 1, // https://github.com/openstf/minicap#usage
    HYBRID_VERSION = 2, // minicap but starting with "Remote" header
    REMOTE_NOKEY = 3, // Original Remote format
    REMOTE_VERSION = 4 // Sends source file path for security check
};

/// Parameters tunable after compilation.
static struct {
    /// Connection parameters:
    RMFormat format; /// Wire format to use
    in_port_t port; /// default socket port for connect
    BOOL benchmark; /// trace when capture is postponed
    int retries; /// Number of times Remote tries to connect
    NSTimeInterval retrySleep; /// seconds between tries
    /// Capture parameters:
    NSTimeInterval defer; /// seconds capture waits for screen to settle
    NSTimeInterval maxDefer; /// maximum seconds between captures (framerate)
    CGFloat jpegQuality; /// JPEG compression quality factor
} params = {
#ifdef REMOTE_MINICAP
    MINICAP_VERSION
#else
#ifdef REMOTE_HYBRID
    HYBRID_VERSION
#else
    REMOTE_VERSION
#endif
#endif
    , REMOTE_PORT,
#ifdef REMOTE_BENCHMARK
    TRUE,
#else
    FALSE,
#endif
    REMOTE_RETRIES, REMOTE_RETRYSLEEP,
    REMOTE_DEFER, REMOTE_MAXDEFER, REMOTE_JPEGQUALITY,
};

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#ifdef REMOTE_LEGACY
static BOOL remoteLegacy = TRUE;
#else
static BOOL remoteLegacy = FALSE;
#endif

/// Types...
typedef unsigned rmpixel_t;
typedef unsigned rmencoded_t;

/// Shaows UITouchPhase enum but available to Appkit code in server.
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
    RMTouchRegionExited,
    RMTouchTuneParameters = 98,
    RMTouchForceCapture = 99,
    RMTouchInsertText = 100
};

/// struct sent from client when it connects to rendering server
/// Can be either original Remote format or "minicap".
struct _rmdevice {
    char version;
    union {
        struct {
            char machine[24];
            char appname[64];
            char appvers[24];
            char hostname[63];
            char scale[4]; // float
            char isIPad[4]; // int
            char protocolVersion[4];// int
            char expansion[60];
            char magic[4]; // int
        } remote;
        struct {
            // See: https://github.com/openstf/minicap#usage
            char headerSize;
            char pid[4]; // int
            char realWidth[4];
            char realHeight[4];
            char virtualWidth[4];
            char virtualHeight[4];
            unsigned char orientation;
            unsigned char quirks;
        } minicap;
    };
};

/// struct send before each iage frame sen to render server
struct _rmframe {
    NSTimeInterval timestamp;
    union {
        /// image paramters
        struct { float width, height, imageScale; };
        /// If length < 0, retails of a recorded touch
        struct { float x, y; RMTouchPhase phase; };
    };
    /// length of image data or if < 0 -touch number
    int length;
};

/// If image is compressed, it's uncompressed length is sent
struct _rmcompress {
    unsigned bytes; unsigned char data[1];
};

#define RMMAX_TOUCHES 2

/// Struct sent from UI of server to replay touches in the client
struct _rmevent {
    NSTimeInterval timestamp;
    RMTouchPhase phase;
    union {
        struct { float x, y; } touches[RMMAX_TOUCHES];
    };
    /* int padded; */
};

/// Internal buffers used for encoding
@interface REMOTE_APPNAME: NSObject {
@package
    rmpixel_t *buffer, *buffend;
    CGContextRef cg;
}
- (instancetype)initFrame:(const struct _rmframe *)frame;
- (NSData *)subtractAndEncode:(REMOTE_APPNAME *)prevbuff;
- (CGImageRef)cgImage;
@end

@protocol RemoteDelegate <NSObject>
@required
- (void)remoteConnected:(BOOL)status;
@end

/// Actual implementation starts here
#if defined(REMOTE_IMPL) || \
    defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && defined(DEBUG)

#ifndef __IPHONE_OS_VERSION_MIN_REQUIRED
#import <Cocoa/Cocoa.h>
#else
#import <UIKit/UIKit.h>
#import <objc/runtime.h>

/// Public Objective-C interface.
@interface REMOTE_APPNAME(Client)
/// Version of RemoteCapture.h
+ (NSString *)revision;
/// Connect to Remote server and start streaming screen captures.
/// @param addrs Space separated list of hostnames to connect to.
+ (void)startCapture:(NSString *)addrs;
/// Low level control of wire format (Called before startCapture)
/// @param format Wire format to use.
/// @param port Port number server is waiting on for connections.
/// @param retries Max number of retries to connect.
/// @param sleep Hold-off seconds between connection attempts.
+ (void)setFormat:(RMFormat)format port:(in_port_t)port
          retries:(int)retries sleep:(NSTimeInterval)sleep;
/// Real time control of capture parameters.
/// @param defer Seconds to defer sending capture to wait for screen to settle.
/// @param maxDefer Max seconds to defer between captures - controls framerate.
/// @param jpegQuality Passed to UIImageJPEGRepresentation()
/// @param benchmark Enable extra performance measureing parameters.
+ (void)setDefer:(NSTimeInterval)defer maxDefer:(NSTimeInterval)maxDefer
     jpegQuality:(double)jpegQuality benchmark:(BOOL)benchmark;
/// Stop capturing events
+ (void)shutdown;
@end

///  Event forging related touches
static struct {
    NSTimeInterval timestamp0;
    UITouch *currentTouch;
    NSSet *currentTouches;
} touches;

@interface RCFakeEvent: UIEvent {
@public
    NSTimeInterval _timestamp;
}

@end

@implementation RCFakeEvent

- (instancetype)init {
    if ((self = [super init])) {
        _timestamp = REMOTE_NOW - touches.timestamp0;
    }
    return self;
}

- (NSTimeInterval)timestamp {
    return _timestamp;
}

- (UITouch *)_firstTouchForView:(UIView *)view {
    RMDebug(@"_firstTouchForView: %@", view);
    return touches.currentTouch;
}

- (NSSet *)touchesForView:(UIView *)view {
    RMDebug(@"touchesForWindow:%@", view);
    return touches.currentTouches;
}

- (NSSet *)touchesForWindow:(UIWindow *)window {
    RMDebug(@"touchesForWindow:%@", window);
    return touches.currentTouches;
}

- (NSSet *)touchesForGestureRecognizer:(UIGestureRecognizer *)rec {
    RMDebug(@"touchesForGestureRecognizer:%@", rec);
    return touches.currentTouches;
}

- (void)_removeTouch:(UITouch *)touch fromGestureRecognizer:(UIGestureRecognizer *)rec {
    RMDebug(@"_removeTouch:%@ fromGestureRecognizer:%@", touch, rec);
}

- (NSSet *)allTouches {
    return touches.currentTouches;
}

- (void)_addWindowAwaitingLatentSystemGestureNotification:(id)a0 deliveredToEventWindow:(id)a1 {
    RMDebug(@"_addWindowAwaitingLatentSystemGestureNotification:%@ deliveredToEventWindow:%@", a0, a1);
}

- (NSUInteger)_buttonMask {
    return 0;
}

- (UIEventType)type {
    return (UIEventType)0;
}

@end

@interface UIView(Description)
- (NSString *)recursiveDescription;
@end

/// Methods swizzled to detect screen updates
@interface NSObject(ForwardReference)

- (void *)_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2;
- (void *)in_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2;

- (void)_didCommitLayer:(void *)a0;
- (void)in_didCommitLayer:(void *)a0;

@end

/// Method swizzled to intercept events
@interface UIApplication(ForwardReference)
- (void)in_sendEvent:(UIEvent *)event;
@end
@implementation UITouch(Identifier)
- (void)in_setTouchIdentifier:(unsigned int)ident {
    Ivar ivar = class_getInstanceVariable([self class], "_touchIdentifier");
    ptrdiff_t offset = ivar_getOffset(ivar);
    unsigned *iptr = (unsigned *)((char *)(__bridge void *)self + offset);
    *iptr = ident;
}
@end
#endif

/// The class defined by RemoteCapture is actually a buffer
/// used to work with the memory representation of screenshots
@implementation REMOTE_APPNAME

/// Setup internal buffer
/// @param frame info providing image size
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

/// Made-up image encoding format
/// @param prevbuff previous image
- (NSData *)subtractAndEncode:(REMOTE_APPNAME *)prevbuff {
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

/// Convert buffer into an image.
- (CGImageRef)cgImage {
    return CGBitmapContextCreateImage(cg);
}

- (void)dealloc {
    CGContextRelease(cg);
}

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED

/// Connection related state
static struct {
    id<RemoteDelegate> remoteDelegate;
    NSMutableArray<NSValue *> *connections;
} remote;

+ (NSString *)revision {
    return @"$Revision: #66 $";
}

#ifdef REMOTEPLUGIN_SERVERIPS
/// Auto-connect
+ (void)load {
    [self startCapture:@REMOTEPLUGIN_SERVERIPS];
}
#endif

/// Initiate screen capture and processing of events from RemoteUI server
/// @param addrs space separated list of IPV4 addresses or hostnames
+ (void)startCapture:(NSString *)addrs {
    [self performSelectorInBackground:@selector(backgroundConnect:)
                           withObject:addrs];
}

/// Connect in the backgrand rather than hold application up.
/// @param addrs space separate list of IPV4 addresses or hostnames
+ (BOOL)backgroundConnect:(NSString *)addrs {
    NSMutableArray *newConnections = [NSMutableArray new];
    for (NSString *addr in [addrs componentsSeparatedByString:@" "]) {
        NSArray<NSString *> *parts = [addr componentsSeparatedByString:@":"];
        NSString *inaddr = parts[0];
        in_port_t port = params.port;
        if (parts.count > 1)
            port = (in_port_t)parts[1].intValue;
        int remoteSocket = [self connectIPV4:inaddr.UTF8String port:port];
        if (remoteSocket) {
            NSLog(@"%@: Connected to %@:%d.", self, inaddr, port);
            FILE *writeFp = fdopen(remoteSocket, "w");
            [newConnections addObject:[NSValue valueWithPointer:writeFp]];
        }
    }
    if (!newConnections.count)
        return FALSE;

    static dispatch_once_t once;
    dispatch_once(&once, ^{
        [self initCapture];
    });

    int32_t keylen = (int)strlen(core.connectionKey);
    for (NSValue *fp in newConnections) {
        FILE *writeFp = (FILE *)fp.pointerValue;
        int headerSize = sizeof core.device.version +
            (core.device.version == MINICAP_VERSION ?
             sizeof core.device.minicap : sizeof core.device.remote);
        if (fwrite(&core.device, 1, headerSize, writeFp) != headerSize)
            NSLog(@"%@: Could not write device info: %s", self, strerror(errno));
        else if (core.device.version == REMOTE_VERSION &&
                 fwrite(&keylen, 1, sizeof keylen, writeFp) != sizeof keylen)
            NSLog(@"%@: Could not write keylen: %s", self, strerror(errno));
        else if (core.device.version == REMOTE_VERSION &&
                 fwrite(core.connectionKey, 1, keylen, writeFp) != keylen)
            NSLog(@"%@: Could not write key: %s", self, strerror(errno));
        else
            [self performSelectorInBackground:@selector(processEvents:)
                                   withObject:fp];
        fflush(writeFp);
    }

    dispatch_async(core.writeQueue, ^{
        [remote.connections addObjectsFromArray:newConnections];
        [self queueCapture];
        core.lateJoiners = TRUE;
    });
    [remote.remoteDelegate remoteConnected:TRUE];
    return TRUE;
}

/// Parse addres and attempt to connect to a "RemoteUI" server
/// @param ipAddress IPV4 address or hostname
/// @param port well known port for remote server
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
            NSLog(@"%@: Could not look up host '%s'", self, ipAddress);
            return 0;
        }
    }

    NSLog(@CONNECTING_FORMAT,
          self, ipAddress, inet_ntoa(remoteAddr.sin_addr),
          ntohs(remoteAddr.sin_port), [self revision]);
    return [self connectAddr:(struct sockaddr *)&remoteAddr];
}

/// Try to connect to specified internet address
/// @param remoteAddr parse/looked-up address
+ (int)connectAddr:(struct sockaddr *)remoteAddr {
    int remoteSocket, optval = 1;
    if ((remoteSocket = socket(remoteAddr->sa_family, SOCK_STREAM, 0)) < 0)
        NSLog(@"%@: Could not open socket for injection: %s", self, strerror(errno));
    else if (setsockopt(remoteSocket, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval)) < 0)
        NSLog(@"%@: Could not set TCP_NODELAY: %s", self, strerror(errno));
    else
        for (int retry = 0; retry<params.retries; retry++) {
            if (retry)
                [NSThread sleepForTimeInterval:params.retrySleep];
            if (connect(remoteSocket, remoteAddr, remoteAddr->sa_len) >= 0)
                return remoteSocket;
        }

    NSLog(@"%@: Could not connect fd #%d: '%s'",
          self, remoteSocket, strerror(errno));
    NSLog(@"%@: Are you running a %@ server at that address?", self, self);
    close(remoteSocket);
    return 0;
}

/// Initialised once state
static struct {
    dispatch_queue_t writeQueue; /// queue to synchronise outgoing writes
    struct _rmdevice device; /// header sent to RemoteUI server on connect
    Class UIWindowLayer; /// Use to filter for full window layer updates
    char *connectionKey; /// Can be used to vet connections
    BOOL lateJoiners; /// Used to flag late connections
} core;

/// Initialse static viables for capture an swizzle in replacement
/// methods for intercepting screen updates and device events
/// Setup device header struct sent on opening the connection.
+ (void)initCapture {
    remote.connections = [NSMutableArray new];
    touches.timestamp0 = REMOTE_NOW;
    core.writeQueue = dispatch_queue_create("writeQueue", DISPATCH_QUEUE_SERIAL);
    core.UIWindowLayer = objc_getClass("UIWindowLayer");
    core.connectionKey = strdup(REMOTE_KEY.UTF8String);
    for (size_t i=0, keylen = (int)strlen(core.connectionKey); i<keylen; i++)
        core.connectionKey[i] ^= REMOTE_XOR;

    __block NSArray<UIScreen *> *screens;
    do {
        dispatch_sync(dispatch_get_main_queue(), ^{
            screens = [UIScreen screens];
        });
        if (!screens.count)
            [NSThread sleepForTimeInterval:.5];
    } while (!screens.count);

#if 01
    method_exchangeImplementations(
        class_getInstanceMethod(CALayer.class, @selector(_copyRenderLayer:layerFlags:commitFlags:)),
        class_getInstanceMethod(CALayer.class, @selector(in_copyRenderLayer:layerFlags:commitFlags:)));
#else
    method_exchangeImplementations(
        class_getInstanceMethod(CALayer.class, @selector(_didCommitLayer:)),
        class_getInstanceMethod(CALayer.class, @selector(in_didCommitLayer:)));
#endif
    method_exchangeImplementations(
        class_getInstanceMethod(UIApplication.class, @selector(sendEvent:)),
        class_getInstanceMethod(UIApplication.class, @selector(in_sendEvent:)));

    core.device.version = params.format;
    if (params.format != MINICAP_VERSION) {
        // prepare remote header
        *(int *)core.device.remote.magic = REMOTE_MAGIC;

        size_t size = sizeof core.device.remote.machine-1;
        sysctlbyname("hw.machine", core.device.remote.machine, &size, NULL, 0);
        core.device.remote.machine[size] = '\000';

        NSDictionary *infoDict = [NSBundle mainBundle].infoDictionary;
        strncpy(core.device.remote.appname,
                [infoDict[@"CFBundleIdentifier"] UTF8String]?:"",
                sizeof core.device.remote.appname-1);
        strncpy(core.device.remote.appvers,
                [infoDict[@"CFBundleShortVersionString"] UTF8String]?:"",
                sizeof core.device.remote.appvers-1);

        gethostname(core.device.remote.hostname, sizeof core.device.remote.hostname-1);

        *(float *)core.device.remote.scale = [screens[0] scale];
        *(int *)core.device.remote.isIPad =
            [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad;
        *(int *)core.device.remote.protocolVersion = 115;
    }
    else {
        // prepare minicap banner
        core.device.version = MINICAP_VERSION;
        core.device.minicap.headerSize = sizeof(core.device.version) + sizeof(core.device.minicap);
        *(uint32_t *)core.device.minicap.pid = getpid();
        *(uint32_t *)core.device.minicap.virtualWidth = screens[0].bounds.size.width;
        *(uint32_t *)core.device.minicap.virtualHeight = screens[0].bounds.size.width;
        *(uint32_t *)core.device.minicap.realWidth =
            *(uint32_t *)core.device.minicap.virtualWidth * [screens[0] scale];
        *(uint32_t *)core.device.minicap.realHeight =
            *(uint32_t *)core.device.minicap.virtualHeight * [screens[0] scale];
    }
}

/// Time varying state
static struct {
    int skipEcho; /// Was to filter out layer commits during capture
    BOOL capturing; /// Am in the middle of capturing
    NSTimeInterval mostRecentScreenUpdate; /// last window layer update
    NSTimeInterval lastCaptureTime; /// last time capture was forced
    NSValue *inhibitEcho; /// prevent events from server going back to server
    UITouch *realTouch; /// An actual UITouch recycled for forging events
    CGSize bufferSize; /// current size of off-screen image buffers
    NSArray *buffers; /// off-screen buffers use in encoding images
    int frameno; /// count of frames captured and transmmitted
} state;

+ (UIApplication *)sharedApplication {
    return [[UIApplication class] performSelector:@selector(sharedApplication)];
}

/// Best effort to get screen dimensions, including iOS on M1 Mac
+ (CGRect)screenBounds {
    CGRect bounds = CGRectZero;
    while (TRUE) {
        for (UIWindow *window in [self sharedApplication].windows)  {
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

/// Capture device's screen (and discard it if there is a more recent update to the screen on the way)
/// @param timestamp Time update to the screen was notified
/// @param flush force transmission of screen update reguardless of timestamp
+ (void)capture:(NSTimeInterval)timestamp flush:(BOOL)flush {
    RMDebug(@"capture: %f %f", timestamp, state.mostRecentScreenUpdate);
    UIScreen *screen = [UIScreen mainScreen];
    CGRect screenBounds = [self screenBounds];
    CGSize screenSize = screenBounds.size;
    CGFloat imageScale = core.device.version == MINICAP_VERSION ? 1. :
        *(int *)core.device.remote.isIPad ||
        *(float *)core.device.remote.scale == 3. ? 1. : screen.scale;
    __block struct _rmframe frame = {REMOTE_NOW,
        {{(float)screenSize.width, (float)screenSize.height, (float)imageScale}}, 0};

    if (state.bufferSize.width != frame.width || state.bufferSize.height != frame.height) {
        state.buffers = nil;
        state.buffers = @[[[self alloc] initFrame:&frame],
                          [[self alloc] initFrame:&frame]];
        state.bufferSize = screenSize;
        state.frameno = 0;
    }

    REMOTE_APPNAME *buffer = state.buffers[state.frameno++&1];
    REMOTE_APPNAME *prevbuff = state.buffers[state.frameno&1];
    UIImage *screenshot;
    RMDebug(@"%@, %@ -- %@", state.buffers, buffer, prevbuff);

//    memset(buffer->buffer, 128, (char *)buffer->buffend - (char *)buffer->buffer);

    // The various ways to capture a screenshot over the years...
    if (remoteLegacy) {
        RMDebug(@"CAPTURE LEGACY");
        BOOL benchmark = FALSE;
        for (UIWindow *window in [[self sharedApplication] windows]) {
            NSTimeInterval start = REMOTE_NOW;
#if 0
            UIView *snap = [window snapshotViewAfterScreenUpdates:YES];
            [snap.layer renderInContext:buffer->cg];
#else
            [[window layer] renderInContext:buffer->cg];
#endif
            if(benchmark)
                RMLog(@"%@ %f", NSStringFromCGRect(window.bounds),
                      REMOTE_NOW-start);
        }
        state.skipEcho = 2;
    }
    else {
        state.capturing = TRUE;
        RMDebug(@"CAPTURE0");
        NSTimeInterval start = REMOTE_NOW;
        CGRect screenBounds = [self screenBounds];
        CGSize screenSize = screenBounds.size;
#if 00
        UIView *snapshotView = [keyWindow snapshotViewAfterScreenUpdates:YES];
        RMDebug(@"CAPTURE1");
        UIGraphicsBeginImageContextWithOptions(screenSize, YES, 0);
        [snapshotView drawViewHierarchyInRect:snapshotView.bounds afterScreenUpdates:NO];
        screenshot = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        BOOL displayingKeyboard = [[self sharedApplication].windows.lastObject
                                   isKindOfClass:objc_getClass("UIRemoteKeyboardWindow")];
        static BOOL displayedKeyboard;
        if (displayingKeyboard)
            displayedKeyboard = TRUE;
        skipEcho = displayedKeyboard ? displayingKeyboard ? 10 : 8 : 6;
#else
//        extern CGImageRef UIGetScreenImage(void);
//        CGImageRef screenimage = UIGetScreenImage();
//        CGContextDrawImage(buffer->cg, CGRectMake(0, 0, screenSize.width, screenSize.height), screenshot);
        CGRect fullBounds = CGRectMake(0, 0,
                                       screenSize.width*REMOTE_OVERSAMPLE,
                                       screenSize.height*REMOTE_OVERSAMPLE);
#if 01
        UIGraphicsBeginImageContext(fullBounds.size);
        for (UIWindow *window in [self sharedApplication].windows)
            if (!window.isHidden)
#if 01
                [window drawViewHierarchyInRect:fullBounds afterScreenUpdates:NO];
#else
                [window.layer renderInContext:UIGraphicsGetCurrentContext()];
#endif
        screenshot = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
#else // Using snapshotViewAfterScreenUpdates (not faster and loops)
        static UIView *screenshotView;
        if (!screenshotView || 1)
            screenshotView = [[UIScreen mainScreen] snapshotViewAfterScreenUpdates:NO];
        RMBench("Sanpshot #%d(%d), %.1fms %f\n", frameno, flush, (REMOTE_NOW-start)*1000., timestamp);

        dispatch_async(dispatch_get_main_queue(), ^{
            NSTimeInterval start = REMOTE_NOW;
            UIGraphicsBeginImageContext(fullBounds.size);
            [screenshotView drawViewHierarchyInRect:fullBounds afterScreenUpdates:YES];
//            [screenshotView.layer renderInContext:UIGraphicsGetCurrentContext()];
            UIImage *screenshot = UIGraphicsGetImageFromCurrentImageContext();
            UIGraphicsEndImageContext();
            RMBench("Render #%d(%d), %.1fms %f\n", frameno, flush,
                    (REMOTE_NOW-start)*1000., timestamp);
            capturing = FALSE;
            skipEcho = 0;

            dispatch_async(writeQueue, ^{
                if (timestamp < mostRecentScreenUpdate && !flush) {
                    frameno--;
                    return;
                }

                NSTimeInterval start = REMOTE_NOW;
                [self encodeAndTransmit:screenshot screenSize:screenSize
                                  frame:frame buffer:buffer prevbuff:prevbuff];
                RMBench("Sent #%d(%d), %.1fms %f\n", frameno, flush,
                        (REMOTE_NOW-start)*1000., timestamp);
            });
        });
        return;
#endif
        state.skipEcho = 0;
#endif
        RMDebug(@"CAPTURE2 %@", [self sharedApplication].windows.lastObject);
        state.capturing = FALSE;
        RMBench("Captured #%d(%d), %.1fms %f\n", state.frameno, flush,
                (REMOTE_NOW-start)*1000., timestamp);
    }

    dispatch_async(core.writeQueue, ^{
        if (timestamp < state.mostRecentScreenUpdate && !flush) {
            RMBench("Discard 3 #%d\n", state.frameno);
            state.frameno--;
            return;
        }

        NSTimeInterval start = REMOTE_NOW;
        [self encodeAndTransmit:screenshot screenSize:screenSize
                          frame:frame buffer:buffer prevbuff:prevbuff];
        RMBench("Sent #%d(%d), %.1fms %f\n",
                state.frameno, flush, (REMOTE_NOW-start)*1000., timestamp);
    });
}

/// Encode image either using Remote's run-length encoded format relaive to previous capture
/// or in minicap format (int32_t length + jpeg image format)
/// @param screenshot UIImage containing screen contents
/// @param screenSize Best extimate of screen size
/// @param frame struct  to be transmitted to server when using Remote's native format
/// @param buffer Buffer to contain most recent screenshot
/// @param prevbuff Buffer containing previous screenshot to relative encode
+ (void)encodeAndTransmit:(UIImage *)screenshot
               screenSize:(CGSize)screenSize frame:(struct _rmframe)frame
       buffer:(REMOTE_APPNAME *)buffer prevbuff:(REMOTE_APPNAME *)prevbuff
{
        NSData *encoded;
        if (core.device.version <= HYBRID_VERSION)
#ifdef REMOTE_PNGFORMAT
            encoded = UIImagePNGRepresentation(screenshot);
#else
            encoded = UIImageJPEGRepresentation(screenshot, params.jpegQuality);
#endif
        else {
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
                RMBench("Discard 4 #%d\n", state.frameno);
                state.frameno--;
                return;
            }

            encoded = core.lateJoiners ? nil : [buffer subtractAndEncode:prevbuff];
            NSData *keyframe = [buffer subtractAndEncode:nil];
            if (core.lateJoiners || keyframe.length < encoded.length)
                encoded = keyframe;

            frame.length = (unsigned)encoded.length;
            if (!core.lateJoiners && frame.length <= REMOTE_MINDIFF) {
                RMBench("Discard 5 #%d\n", state.frameno);
                state.frameno--;
                return;
            }

#ifdef REMOTE_COMPRESSION
            struct _rmcompress *buff = malloc(sizeof buff->bytes + encoded.length + 100);
            uLongf clen = buff->bytes = (unsigned)encoded.length;
            if (compress2(buff->data, &clen, (const Bytef *)encoded.bytes,
                          buff->bytes, Z_BEST_SPEED) == Z_OK && clen < encoded.length) {
                encoded = [NSMutableData dataWithBytesNoCopy:buff length:sizeof buff->bytes+clen freeWhenDone:YES];
                RMLog(@"%@: Delta image %d/%d %.1f%%", self, (int)encoded.length, frame.length, 100.*encoded.length/frame.length);
                frame.length = REMOTE_COMPRESSED_OFFSET + (int)encoded.length;
            }
#endif
        }

        core.lateJoiners = FALSE;
        for (NSValue *fp in remote.connections) {
            FILE *writeFp = (FILE *)fp.pointerValue;
            uint32_t frameSize = (uint32_t)encoded.length;
            int frameHeaderSize = core.device.version <= HYBRID_VERSION ?
                                    sizeof frameSize : sizeof frame;
            if (fwrite(core.device.version <= HYBRID_VERSION ? (void *)&frameSize :
                       (void *)&frame, 1, frameHeaderSize, writeFp) != frameHeaderSize)
                NSLog(@"%@: Could not write frame: %s", self, strerror(errno));
            else if (fwrite(encoded.bytes, 1, encoded.length, writeFp) != encoded.length)
                NSLog(@"%@: Could not write encoded: %s", self, strerror(errno));
            else
                fflush(writeFp);
        }
}

/// Run in backgrount to process event structs coming from user interface in order to forge them
/// @param writeFp Connection to RemoteUI server
+ (void)processEvents:(NSValue *)writeFp {
    FILE *readFp = fdopen(fileno((FILE *)writeFp.pointerValue), "r");

    struct _rmevent rpevent;
    while (fread(&rpevent, 1, sizeof rpevent, readFp) == sizeof rpevent) {

        RMLog(@"%@ Event: %f %f %d", self,
              rpevent.touches[0].x, rpevent.touches[0].y, rpevent.phase);

        if (rpevent.phase == RMTouchForceCapture) {
            [self queueCapture];
            continue;
        }

        if (rpevent.phase == RMTouchTuneParameters) {
            [self setDefer:rpevent.touches[0].x
                  maxDefer:rpevent.touches[0].y
               jpegQuality:params.jpegQuality
                 benchmark:params.benchmark];
            continue;
        }

        if (rpevent.phase == RMTouchMoved && state.capturing)
            continue;

        NSTimeInterval timestamp = rpevent.timestamp;

        NSString *sentText;
        if (rpevent.phase >= RMTouchInsertText) {
            size_t textSize = rpevent.phase - RMTouchInsertText;
            char *buffer = (char *)malloc(textSize + 1);
            textSize = fread(buffer, 1, textSize, readFp);
            buffer[textSize] = '\000';
            sentText = [NSString stringWithUTF8String:buffer];
            free(buffer);
        }

        dispatch_sync(dispatch_get_main_queue(), ^{
            CGPoint location = {rpevent.touches[0].x, rpevent.touches[0].y},
                location2 = {rpevent.touches[1].x, rpevent.touches[1].y};
            static UITextAutocorrectionType saveAuto;
            static BOOL isTextfield, isKeyboard, isButton;
            static UITextField *textField;
            static UITouch *currentTouch2;
            static UIView *currentTarget;
            static unsigned touchIdentifier = 120;
            touchIdentifier++;

            static UITouchesEvent *event;
            if (!event)
                event = [[objc_getClass("UITouchesEvent") alloc] _init];
            RCFakeEvent *fakeEvent = [RCFakeEvent new];
            fakeEvent->_timestamp = timestamp;

            if (sentText) {
                [textField insertText:sentText];
                return;
            }

            state.inhibitEcho = writeFp;

            switch (rpevent.phase) {

                case RMTouchBeganDouble:

                    currentTarget = nil;
                    for (UIWindow *window in [self sharedApplication].windows) {
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
                    [currentTouch2 in_setTouchIdentifier:touchIdentifier];
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
                    for (UIWindow *window in [self sharedApplication].windows) {
                        UIView *found = [window hitTest:location withEvent:fakeEvent];
                        if (found)
                            currentTarget = found;
                    }

                    isTextfield = [currentTarget
                                   respondsToSelector:@selector(setAutocorrectionType:)];
                    isKeyboard = [currentTarget
                                  isKindOfClass:objc_getClass("UIKeyboardLayoutStar")];

                    static NSArray *needsTouch;
                    if (!needsTouch) needsTouch = @[
                        objc_getClass("UIButton"),
                        objc_getClass("UITabBarButton"),
                        objc_getClass("_UIButtonBarButton"),
                        objc_getClass("UISegmentedControl"),
                        objc_getClass("UITableViewCellEditControl"),
                        objc_getClass("UISwipeActionStandardButton"),
                    ];

                    isButton = [currentTarget class] == [UIView class];
                    for (Class buttonClass in needsTouch)
                        if ([currentTarget isKindOfClass:buttonClass])
                            isButton = true;

                    if (!touches.currentTouch)
                        touches.currentTouch = state.realTouch ?: [UITouch new];
                    [touches.currentTouch in_setTouchIdentifier:touchIdentifier];

                    RMDebug(@"Target selected: %@ %d %d %d %d %lx\n%@",
                            currentTarget, isTextfield, isKeyboard, isButton,
                            touches.currentTouch._touchIdentifier,
                            touches.currentTouch.hash,
                            [currentTarget recursiveDescription]);

                    if (currentTarget.superview.class ==
                        objc_getClass("UIKeyboardEmojiCollectionViewCell")) {
                        NSString *emoji = [(UILabel *)currentTarget.subviews[0] text];
                        [textField insertText:emoji];
                    }

                    if (isTextfield) {
                        textField = (UITextField *)currentTarget;
                        saveAuto = textField.autocorrectionType;
                        textField.autocorrectionType = UITextAutocorrectionTypeNo;
                    }

                    [touches.currentTouch setTimestamp:timestamp];
                    [touches.currentTouch setInitialTouchTimestamp:timestamp];
                    [touches.currentTouch setPhase:(UITouchPhase)rpevent.phase];
//                    [currentTouch setSentTouchesEnded:false];
                    [touches.currentTouch setWindow:currentTarget.window];
//                    [currentTouch _setDisplacement:CGSizeMake(0.0, 0.0)];
//                    [currentTouch _setWindowServerHitTestWindow:currentTarget.window];
//                    [currentTouch _setTouchIdentifier:touchIdentifier];
                    [touches.currentTouch _setPathIndex:1];
                    [touches.currentTouch _setPathIdentity:2];
//                    [currentTouch setMajorRadius:20.0];
//                    [currentTouch setMajorRadiusTolerance:5.0];
                    [touches.currentTouch _setType:0];
//                    [currentTouch _setNeedsForceUpdate:false];
//                    [currentTouch _setHasForceUpdate:false];
//                    [currentTouch _setForceCorrelationToken:0];
                    [touches.currentTouch _setSenderID:778835616971358211];
                    [touches.currentTouch _setZGradient:0.0];
//                    [currentTouch _setMaximumPossiblePressure:0.0];
                    [touches.currentTouch _setEdgeType:0];
                    [touches.currentTouch _setEdgeAim:0];

                    //[self _setIsFirstTouchForView:1];
                    [touches.currentTouch setView:currentTarget];
//                    [currentTouch _setLocation:location preciseLocation:location inWindowResetPreviousLocation:true];
//                    [currentTouch _setPressure:0.0 resetPrevious:true];
                    //[UITouch _updateWithChildEvent:0x600001ff81c0];
//                    [currentTouch setIsTap:true];
                    [touches.currentTouch setTapCount:1];
//                    [currentTouch _setIsFirstTouchForView:true];

                    [touches.currentTouch _setLocationInWindow:location resetPrevious:YES];

                    touches.currentTouches = [NSSet setWithObjects:touches.currentTouch, currentTouch2, nil];

                    [event _clearTouches];
                    [event _addTouch:touches.currentTouch forDelayedDelivery:NO];
                    if (currentTouch2)
                        [event _addTouch:currentTouch2 forDelayedDelivery:NO];

                    [[self sharedApplication] in_sendEvent:event];
                    if (isButton)
                        [currentTarget touchesBegan:touches.currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchMoved:
                case RMTouchStationary:
                    [touches.currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [touches.currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [touches.currentTouch setTimestamp:timestamp];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:timestamp];

                    [[self sharedApplication] in_sendEvent:event];
                    if (isButton)
                        [currentTarget touchesMoved:touches.currentTouches withEvent:fakeEvent];
                    break;

                case RMTouchEnded:
                case RMTouchCancelled:
                    [touches.currentTouch setPhase:(UITouchPhase)rpevent.phase];
                    [touches.currentTouch _setLocationInWindow:location resetPrevious:YES];
                    [touches.currentTouch setTimestamp:timestamp];

                    [currentTouch2 setPhase:(UITouchPhase)rpevent.phase];
                    [currentTouch2 _setLocationInWindow:location2 resetPrevious:YES];
                    [currentTouch2 setTimestamp:timestamp];

                    [[self sharedApplication] in_sendEvent:event];
                    if (isButton)
                        [currentTarget touchesEnded:touches.currentTouches withEvent:fakeEvent];

                    if (isTextfield) {
                        UITextField *textField = (UITextField *)currentTarget;
                        textField.autocorrectionType = saveAuto;
                    }

                    touches.currentTouches = nil;
                    currentTarget = nil;
                    currentTouch2 = nil;
                    touches.currentTouch = nil;
                    event = nil;
                    break;

                default:
                    NSLog(@"%@: Invalid Event: %d", self, rpevent.phase);
            }

            state.inhibitEcho = nil;
        });
    }

    NSLog(@"%@: processEvents: exits", self);
    fclose(readFp);

    [remote.connections removeObject:writeFp];
    fclose((FILE *)writeFp.pointerValue);
    if (!remote.connections.count)
        [self shutdown];
}

/// Low level control of wire format.
/// @param format Wire format to use.
/// @param port Port number server is waiting on for connections.
/// @param retries Max number of retries to connect.
/// @param sleep Hold-off seconds between connection attempts.
+ (void)setFormat:(RMFormat)format port:(in_port_t)port
          retries:(int)retries sleep:(NSTimeInterval)sleep {
    if (remote.connections)
        NSLog(@"%@: Connection parameters can not be changed"
              " after you've connected.", self);
    params.format = format;
    params.port = port;
    params.retries = retries;
    params.retrySleep = sleep;
}

/// Real time control of capture parameters.
/// @param defer Seconds to defer sending capture to wait for screen to settle.
/// @param maxDefer Max seconds to defer between captures - controls framerate.
/// @param jpegQuality Passed to UIImageJPEGRepresentation()
/// @param benchmark Enable extra performance measureing parameters.
+ (void)setDefer:(NSTimeInterval)defer maxDefer:(NSTimeInterval)maxDefer
     jpegQuality:(double)jpegQuality benchmark:(BOOL)benchmark {
    params.defer = defer;
    params.maxDefer = maxDefer;
    params.jpegQuality = jpegQuality ?: REMOTE_JPEGQUALITY;
    params.benchmark = benchmark;
}

/// Stop capturing events
+ (void)shutdown {
    [remote.remoteDelegate remoteConnected:FALSE];
    for (NSValue *writeFp in remote.connections)
        fclose((FILE *)writeFp.pointerValue);
    [remote.connections removeAllObjects];
}

/// A delicate piece of code to work out when to request the capture of the screen
/// and transmission of it's representation to the RemoteUI server. Routed through
/// writeQueue to ensure that output does not back up on say, cellular connections.
+ (void)queueCapture {
    if (!remote.connections.count)
        return;

    NSTimeInterval timestamp =
    state.mostRecentScreenUpdate = REMOTE_NOW;
    BOOL flush = timestamp > state.lastCaptureTime + params.maxDefer;
    if (flush)
        state.lastCaptureTime = timestamp;
    dispatch_async(core.writeQueue, ^{
        int64_t delta = 0;
        if (!flush) {
            if (timestamp < state.mostRecentScreenUpdate) {
                RMBench("Discard 1\n");
                return;
            }
#if 0
            [NSThread sleepForTimeInterval:params.defer];
#else
            delta = (int64_t)(params.defer * NSEC_PER_SEC);
#endif
        }

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delta), dispatch_get_main_queue(), ^{
            RMDebug(@"Capturing? %d %f", flush, state.mostRecentScreenUpdate);
            if (timestamp < (flush ? state.lastCaptureTime :
                             state.mostRecentScreenUpdate)) {
                RMBench("Discard 2 %d\n", flush);
                return;
            }
            [self capture:timestamp flush:flush];
//            lastCaptureTime = timestamp;
        });
    });
}

@end

@implementation CALayer(REMOTE_APPNAME)

/// Twp methods that can be swizzled in to generate a stream of notifications the screen has been updated
- (void *)in_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2 {
    void *out = [self in_copyRenderLayer:a0 layerFlags:a1 commitFlags:a2];
    RMDebug(@"in_copyRenderLayer: %d %d %@ %lu", state.capturing, state.skipEcho,
            self, (unsigned long)[REMOTE_APPNAME sharedApplication].windows.count);
    if (self.class == core.UIWindowLayer)
        [REMOTE_APPNAME queueCapture];
    return out;
}

- (void)in_didCommitLayer:(void *)a0 {
    [self in_didCommitLayer:a0];
    RMDebug(@"in_didCommitLayer: %d %d %@ %lu", state.capturing, state.skipEcho,
            self, (unsigned long)[REMOTE_APPNAME sharedApplication].windows.count);
    if (self.class == core.UIWindowLayer)
        [REMOTE_APPNAME queueCapture];
}

@end

@implementation UIApplication(REMOTE_APPNAME)

/// Swizzled in to capture device events and transmit them to the RemoteUI server
/// so they can be recorded and played back using processEvents: above.
/// Events encoded as a fake frame with negative length rather than image & size.
/// @param anEvent actual UIEvent which contains the UITouches
- (void)in_sendEvent:(UIEvent *)anEvent {
    [self in_sendEvent:anEvent];
    NSValue *incomingFp = state.inhibitEcho;
    NSSet *touches = anEvent.allTouches;
    state.realTouch = touches.anyObject;

#if 0
    RMLog(@"%@", anEvent);
    for (UITouch *t in touches)
        RMLog(@"Gestures: %@", t.gestureRecognizers);
#endif

    struct _rmframe header;
    header.timestamp = REMOTE_NOW;
    header.length = -(int)touches.count;

    NSMutableData *out = [NSMutableData new];
    if (core.device.version <= HYBRID_VERSION)
        [out appendBytes:&header.length length:sizeof header.length];

    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:touch.window];
        header.phase = (RMTouchPhase)touch.phase;
        header.x = loc.x;
        header.y = loc.y;
        [out appendBytes:&header length:sizeof header];
        header.length++;
    }

    dispatch_async(core.writeQueue, ^{
        for (NSValue *fp in remote.connections) {
            if (fp == incomingFp)
                continue;
            FILE *writeFp = (FILE *)fp.pointerValue;
            if (fwrite(out.bytes, 1, out.length, writeFp) != out.length)
                NSLog(@"%@: Could not write event: %s", REMOTE_APPNAME.class, strerror(errno));
            else
                fflush(writeFp);
        }
    });
}

#endif
@end
#endif
