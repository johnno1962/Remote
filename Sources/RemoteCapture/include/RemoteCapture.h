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

// Various wire formats used.
#define REMOTE_NOKEY 3 // Original format
#define REMOTE_VERSION 4 // Sends source file path for security check
#define MINICAP_VERSION 1 // https://github.com/openstf/minicap#usage
#define HYBRID_VERSION 2 // minicap but starting with "Remote" header

// May be used for security
#define REMOTE_KEY @__FILE__
#define REMOTE_XOR 0xc5

// Times coordinate-resolution to capture.
#ifndef REMOTE_OVERSAMPLE
#define REMOTE_OVERSAMPLE 1.0
#endif

#ifndef REMOTE_RETRIES
#define REMOTE_RETRIES 3
#endif

#ifdef REMOTE_HYBRID
// Wait for screen to settle before capture
#ifndef REMOTE_DEFER
#define REMOTE_DEFER 0.2
#endif

// Only wait this long for screen to settle
#ifndef REMOTE_MAXDEFER
#define REMOTE_MAXDEFER 0.1
#endif
#else
// Wait for screen to settle before capture
#ifndef REMOTE_DEFER
#define REMOTE_DEFER 0.0
#endif

// Only wait this long for screen to settle
#ifndef REMOTE_MAXDEFER
#define REMOTE_MAXDEFER 0.0
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

#define REMOTE_NOW [NSDate timeIntervalSinceReferenceDate]
#ifdef REMOTE_BENCHMARK
#define RMBench printf
#else
#define RMBench while(0) printf
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
            char expansion[64];
            char magic[4]; // int
        } remote;
        struct {
            // See: https://github.com/openstf/minicap#usage
            char headerSize;
            char pid[4];
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

#if defined(REMOTE_IMPL) || \
    defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && defined(DEBUG)

#ifndef __IPHONE_OS_VERSION_MIN_REQUIRED
#import <Cocoa/Cocoa.h>
#else
#import <UIKit/UIKit.h>
#import <objc/runtime.h>

@interface REMOTE_APPNAME(Client)
+ (void)startCapture:(NSString *)addrs;
+ (void)shutdown;
@end

static NSTimeInterval timestamp0;
static UITouch *currentTouch;
static NSSet *currentTouches;
static BOOL lateJoiners;

@interface RCFakeEvent: UIEvent {
@public
    NSTimeInterval _timestamp;
}

@end

@implementation RCFakeEvent

- (instancetype)init {
    if ((self = [super init])) {
        _timestamp = REMOTE_NOW - timestamp0;
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
    return (UIEventType)0;
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
@implementation UITouch(Identifier)
- (void)_setTouchIdentifier:(unsigned int)ident {
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

static id<RemoteDelegate> remoteDelegate;
static NSMutableArray<NSValue *> *connections;
static char *connectionKey;

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
        in_port_t port = REMOTE_PORT;
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

    int32_t keylen = (int)strlen(connectionKey);
    for (NSValue *fp in newConnections) {
        FILE *writeFp = (FILE *)fp.pointerValue;
        int headerSize = 1 + (device.version == MINICAP_VERSION ?
            sizeof device.minicap : sizeof device.remote);
        if (fwrite(&device, 1, headerSize, writeFp) != headerSize)
            NSLog(@"%@: Could not write device info: %s", self, strerror(errno));
        else if (device.version == REMOTE_VERSION &&
                 fwrite(&keylen, 1, sizeof keylen, writeFp) != sizeof keylen)
            NSLog(@"%@: Could not write keylen: %s", self, strerror(errno));
        else if (device.version == REMOTE_VERSION &&
                 fwrite(connectionKey, 1, keylen, writeFp) != keylen)
            NSLog(@"%@: Could not write key: %s", self, strerror(errno));
        else
            [self performSelectorInBackground:@selector(processEvents:) withObject:fp];
    }

    dispatch_async(writeQueue, ^{
        [connections addObjectsFromArray:newConnections];
        [self queueCapture];
        lateJoiners = TRUE;
    });
    [remoteDelegate remoteConnected:TRUE];
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

    NSLog(@"%@: Attempting connection to: %s:%d", self, ipAddress, port);
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
        for (int retry = 0; retry<REMOTE_RETRIES; retry++) {
            if (retry)
                [NSThread sleepForTimeInterval:1.0];
            if (connect(remoteSocket, remoteAddr, remoteAddr->sa_len) >= 0)
                return remoteSocket;
        }

    NSLog(@"%@: Could not connect: %s", self, strerror(errno));
    close(remoteSocket);
    return 0;
}

static dispatch_queue_t writeQueue; // queue to synchronise outgoing writes
static struct _rmdevice device; // header sent to RemoteUI server on connect
static NSValue *inhibitEcho; // prevent events from server going back to server
static Class UIWindowLayer; // Use to filter for full window layer updates
static UITouch *realTouch; // An actual UITouch recycled for forging events
static CGSize bufferSize; // current size of off-screen image buffers

/// Initialse static viables for capture an swizzle in replacement
/// methods for intercepting screen updates and device events
/// Setup device header struct sent on opening the connection.
+ (void)initCapture {
    connections = [NSMutableArray new];
    timestamp0 = REMOTE_NOW;
    writeQueue = dispatch_queue_create("writeQueue", DISPATCH_QUEUE_SERIAL);
    UIWindowLayer = objc_getClass("UIWindowLayer");
    connectionKey = strdup(REMOTE_KEY.UTF8String);
    for (size_t i=0, keylen = (int)strlen(connectionKey); i<keylen; i++)
        connectionKey[i] ^= REMOTE_XOR;

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

#ifndef REMOTE_MINICAP
#if defined(REMOTE_HYBRID)
    device.version = HYBRID_VERSION;
#else
    device.version = REMOTE_VERSION;
#endif
    // prepare remote header
    *(int *)device.remote.magic = REMOTE_MAGIC;

    size_t size = sizeof device.remote.machine-1;
    sysctlbyname("hw.machine", device.remote.machine, &size, NULL, 0);
    device.remote.machine[size] = '\000';

    NSDictionary *infoDict = [NSBundle mainBundle].infoDictionary;
    strncpy(device.remote.appname, [infoDict[@"CFBundleIdentifier"] UTF8String]?:"", sizeof device.remote.appname-1);
    strncpy(device.remote.appvers, [infoDict[@"CFBundleShortVersionString"] UTF8String]?:"", sizeof device.remote.appvers-1);

    gethostname(device.remote.hostname, sizeof device.remote.hostname-1);

    *(float *)device.remote.scale = [screens[0] scale];
    *(int *)device.remote.isIPad =
        [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad;
#else // REMOTE_MINICAP
    // prepare minicap banner
    device.version = MINICAP_VERSION;
    device.minicap.headerSize = sizeof(device.version) + sizeof(device.minicap);
    *(uint32_t *)device.minicap.pid = getpid();
    *(uint32_t *)device.minicap.virtualWidth = screens[0].bounds.size.width;
    *(uint32_t *)device.minicap.virtualHeight = screens[0].bounds.size.width;
    *(uint32_t *)device.minicap.realWidth =
        *(uint32_t *)device.minicap.virtualWidth * [screens[0] scale];
    *(uint32_t *)device.minicap.realHeight =
        *(uint32_t *)device.minicap.virtualHeight * [screens[0] scale];
#endif
}

static int skipEcho; // Was to filter out layer commits during capture
static BOOL capturing; // Am in the middle of capturing
static NSTimeInterval mostRecentScreenUpdate; // last window layer update
static NSTimeInterval lastCaptureTime; // last time capture was forced
static NSArray *buffers; // off-screen buffers use in encoding images
static int frameno; // count of frames captured and transmmitted

/// Best effeort to get screen dimensions, even for iOS on M1 Mac
+ (CGRect)screenBounds {
    CGRect bounds = CGRectZero;
    while (TRUE) {
        for (UIWindow *window in [UIApplication sharedApplication].windows)  {
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
    RMDebug(@"capture: %f %f", timestamp, mostRecentScreenUpdate);
    UIScreen *screen = [UIScreen mainScreen];
    CGRect screenBounds = [self screenBounds];
    CGSize screenSize = screenBounds.size;
    CGFloat imageScale = device.version == MINICAP_VERSION ? 1. :
        *(int *)device.remote.isIPad || *(float *)device.remote.scale == 3. ? 1. : screen.scale;
    __block struct _rmframe frame = {REMOTE_NOW,
        {{(float)screenSize.width, (float)screenSize.height, (float)imageScale}}, 0};

    if (bufferSize.width != frame.width || bufferSize.height != frame.height) {
        buffers = nil;
        buffers = @[[[self alloc] initFrame:&frame],
                    [[self alloc] initFrame:&frame]];
        bufferSize = screenSize;
        frameno = 0;
    }

    REMOTE_APPNAME *buffer = buffers[frameno++&1];
    REMOTE_APPNAME *prevbuff = buffers[frameno&1];
    UIImage *screenshot;
    RMDebug(@"%@, %@ -- %@", buffers, buffer, prevbuff);

//    memset(buffer->buffer, 128, (char *)buffer->buffend - (char *)buffer->buffer);

    // The various ways to capture a screenshot over the years...
    if (remoteLegacy) {
        RMDebug(@"CAPTURE LEGACY");
        BOOL benchmark = FALSE;
        for (UIWindow *window in [[UIApplication sharedApplication] windows]) {
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
        skipEcho = 2;
    }
    else {
        capturing = TRUE;
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
        CGRect fullBounds = CGRectMake(0, 0,
                                       screenSize.width*REMOTE_OVERSAMPLE,
                                       screenSize.height*REMOTE_OVERSAMPLE);
        UIGraphicsBeginImageContext(fullBounds.size);
#if 01
        for (UIWindow *window in [UIApplication sharedApplication].windows)
            if (!window.isHidden)
                [window drawViewHierarchyInRect:fullBounds afterScreenUpdates:NO];
#else
        UIView *snap = [screen snapshotViewAfterScreenUpdates:NO];
        [snap drawViewHierarchyInRect:fullBounds afterScreenUpdates:NO];
#endif
        screenshot = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        skipEcho = 0;
#endif
        RMDebug(@"CAPTURE2 %@", [UIApplication sharedApplication].windows.lastObject);
        capturing = FALSE;
        RMBench("Captured #%d(%d), %.1fms %f\n", frameno, flush, ([NSDate
                  timeIntervalSinceReferenceDate]-start)*1000., timestamp);
    }

    dispatch_async(writeQueue, ^{
        if (timestamp < mostRecentScreenUpdate && !flush) {
            frameno--;
            return;
        }

        NSTimeInterval start = REMOTE_NOW;
        [self encodeAndTransmit:screenshot screenSize:screenSize
                          frame:frame buffer:buffer prevbuff:prevbuff];
        RMBench("Sent #%d(%d), %.1fms %f\n", frameno, flush, ([NSDate
                  timeIntervalSinceReferenceDate]-start)*1000., timestamp);
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
        if (device.version <= HYBRID_VERSION)
#ifdef REMOTE_PNGFORMAT
            encoded = UIImagePNGRepresentation(screenshot);
#else
            encoded = UIImageJPEGRepresentation(screenshot, 0.8);
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
                frameno--;
                return;
            }

            encoded = lateJoiners ? nil : [buffer subtractAndEncode:prevbuff];
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
                RMLog(@"%@: Delta image %d/%d %.1f%%", self, (int)encoded.length, frame.length, 100.*encoded.length/frame.length);
                frame.length = REMOTE_COMPRESSED_OFFSET + (int)encoded.length;
            }
#endif
        }

        for (NSValue *fp in connections) {
            FILE *writeFp = (FILE *)fp.pointerValue;
            uint32_t frameSize = (uint32_t)encoded.length;
            int frameHeaderSize = device.version <= HYBRID_VERSION ?
                                    sizeof frameSize : sizeof frame;
            if (fwrite(device.version <= HYBRID_VERSION ? (void *)&frameSize :
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

        if (rpevent.phase == RMTouchMoved && capturing)
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

            inhibitEcho = writeFp;

            switch (rpevent.phase) {

                case RMTouchBeganDouble:

                    currentTarget = nil;
                    for (UIWindow *window in [UIApplication sharedApplication].windows) {
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
                    [currentTouch2 _setTouchIdentifier:touchIdentifier];
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
                    for (UIWindow *window in [UIApplication sharedApplication].windows) {
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

                    if (!currentTouch)
                        currentTouch = realTouch ?: [UITouch new];
                    [currentTouch _setTouchIdentifier:touchIdentifier];

                    RMDebug(@"Target selected: %@ %d %d %d %d %lx\n%@",
                            currentTarget, isTextfield, isKeyboard, isButton,
                            currentTouch._touchIdentifier, currentTouch.hash,
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

                    [[UIApplication sharedApplication] in_sendEvent:event];
                    if (isButton)
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

                    [[UIApplication sharedApplication] in_sendEvent:event];
                    if (isButton)
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

                    [[UIApplication sharedApplication] in_sendEvent:event];
                    if (isButton)
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
                    NSLog(@"%@: Invalid Event: %d", self, rpevent.phase);
            }

            inhibitEcho = nil;
        });
    }

    NSLog(@"%@: processEvents: exits", self);
    fclose(readFp);

    [connections removeObject:writeFp];
    fclose((FILE *)writeFp.pointerValue);
    if (!connections.count)
        [self shutdown];
}

/// Stop capturing events
+ (void)shutdown {
    [remoteDelegate remoteConnected:FALSE];
    for (NSValue *writeFp in connections)
        fclose((FILE *)writeFp.pointerValue);
    connections = nil;
}

/// A delicate peice of code to work out when to request the capture of the screen
/// and transmission of it's representation to the RemoteUI server. Routed through
/// writeQueue to ensure that output does not back up on say, cellular connections.
+ (void)queueCapture {
    if (!connections.count)
        return;

    NSTimeInterval timestamp = REMOTE_NOW;
    mostRecentScreenUpdate = timestamp;
    BOOL flush = timestamp > lastCaptureTime + REMOTE_MAXDEFER;
    if (flush)
        lastCaptureTime = timestamp;
    dispatch_async(writeQueue, ^{
        int64_t delta = 0;
        if (!flush) {
            if (timestamp < mostRecentScreenUpdate)
                return;
#if 01
            [NSThread sleepForTimeInterval:REMOTE_DEFER];
#else
            delta = (int64_t)(REMOTE_DEFER * NSEC_PER_SEC);
#endif
        }

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delta), dispatch_get_main_queue(), ^{
            RMDebug(@"Capturing? %d %f", flush, mostRecentScreenUpdate);
            if ((timestamp < mostRecentScreenUpdate && !flush) ||
                timestamp < lastCaptureTime)
                return;
            [self capture:timestamp flush:flush];
        });
    });
}

@end

@implementation CALayer(REMOTE_APPNAME)

/// Twp methods that can be swizzled in to generate a stream of notifications the screen has been updated
- (void *)in_copyRenderLayer:(void *)a0 layerFlags:(unsigned)a1 commitFlags:(unsigned *)a2 {
    void *out = [self in_copyRenderLayer:a0 layerFlags:a1 commitFlags:a2];
    RMDebug(@"in_copyRenderLayer: %d %d %@ %lu", capturing, skipEcho, self,
            (unsigned long)[UIApplication sharedApplication].windows.count);
    if (self.class == UIWindowLayer)
        [REMOTE_APPNAME queueCapture];
    return out;
}

- (void)in_didCommitLayer:(void *)a0 {
    [self in_didCommitLayer:a0];
    RMDebug(@"in_didCommitLayer: %d %d %@ %lu", capturing, skipEcho, self,
            (unsigned long)[UIApplication sharedApplication].windows.count);
    if (self.class == UIWindowLayer)
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
    NSSet *touches = anEvent.allTouches;
    NSValue *incomingFp = inhibitEcho;
    realTouch = touches.anyObject;

#if 0
    RMLog(@"%@", anEvent);
    for (UITouch *t in touches)
        RMLog(@"Gestures: %@", t.gestureRecognizers);
#endif

    struct _rmframe header;
    header.timestamp = REMOTE_NOW;
    header.length = -(int)touches.count;

    NSMutableData *out = [NSMutableData new];
    if (device.version <= HYBRID_VERSION)
        [out appendBytes:&header.length length:sizeof header.length];

    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:touch.window];
        header.phase = (RMTouchPhase)touch.phase;
        header.x = loc.x;
        header.y = loc.y;
        [out appendBytes:&header length:sizeof header];
        header.length++;
    }

    dispatch_async(writeQueue, ^{
        for (NSValue *fp in connections) {
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
