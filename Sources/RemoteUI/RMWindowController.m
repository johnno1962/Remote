//
//  RMWindowController.m
//  Remote
//
//  Created by John Holdsworth on 20/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//
//  Repo: https://github.com/johnno1962/Remote
//  $Id: //depot/Remote/Sources/RemoteUI/RMWindowController.m#4 $
//

#import "RMWindowController.h"

#import "RMDeviceController.h"
#import "RMMacroManager.h"
#import "RMImageView.h"

#include <sys/ioctl.h>
#include <net/if.h>

#ifndef REMOTE_TITLE
#define REMOTE_TITLE Remote
#endif
#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

@implementation NSTextCheckingResult(groups)

- (NSString *)groupAtIndex:(NSUInteger)index inString:(NSString *)string {
    NSRange range = [self rangeAtIndex:index];
    return range.location != NSNotFound ? [string substringWithRange:range] : nil;
}

@end

@interface RMWindowController () <NSWindowDelegate,RMDeviceDelegate> {
    IBOutlet RMMacroManager *manager;
    IBOutlet RMImageView *imageView;
    IBOutlet NSDrawer *drawer;
    IBOutlet NSView *logView;
    IBOutlet NSProgressIndicator *spinner;

    NSTimeInterval lastEvent;
    CGFloat aspect;
    BOOL cancel;
}

@end

RMWindowController *lastRMWindowController;
static NSMutableDictionary<NSString *,RMWindowController *> *connectionWindows;
static NSMenuItem *macroMenu;
static int serverSocket;

@implementation RMWindowController

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

+ (NSModalResponse)error:(NSString *)format, ... {
    va_list argp; va_start(argp, format);
    NSString *msg = [[NSString alloc] initWithFormat:format arguments:argp];
    return [[NSAlert alertWithMessageText:@"Remote Plugin:"
                            defaultButton:@"OK" alternateButton:nil otherButton:nil
                informativeTextWithFormat:@"%@", msg] runModal];
}

+ (void)startServer:(NSMenuItem *)mainMacroMenu {
    [self startServer:mainMacroMenu port:REMOTE_PORT];
}

+ (void)startServer:(NSMenuItem *)mainMacroMenu port:(in_port_t)port {
    connectionWindows = [NSMutableDictionary new];
    macroMenu = mainMacroMenu;

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    int optval = 1;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        [self error:@"Could not open service socket: %s", strerror(errno)];
    else if (fcntl(serverSocket, F_SETFD, FD_CLOEXEC) < 0)
        [self error:@"Could not set close exec: %s", strerror(errno)];
    else if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) < 0)
        [self error:@"Could not set socket option: %s", strerror(errno)];
    else if (setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval)) < 0)
        [self error:@"Could not set socket option: %s", strerror(errno)];
    else if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof serverAddr) < 0)
        [self error:@"Could not bind service socket: %s.", strerror(errno)];
    else if (listen(serverSocket, 5) < 0)
        [self error:@"Service socket would not listen: %s", strerror(errno)];
    else
        [self performSelectorInBackground:@selector(backgroundConnectionService) withObject:nil];
}

+ (void)stopServer {
    close(serverSocket);
    serverSocket = 0;
}

// accept connection from serverSocket
+ (void)backgroundConnectionService {

    NSLog(@"RMWindowController: Waiting for connections...");
    while (serverSocket) {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof clientAddr;

        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket > 0) {
            int optval = 1;
            if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval)) < 0)
                [self error:@"Set TCP_NODELAY %s", strerror(errno)];

            NSString *ipAddr = [NSString stringWithFormat:@"%s",//:%d", // can be unique to port
                                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)];
            NSLog(@"RMWindowController: Connection from %@", ipAddr);

            dispatch_sync(dispatch_get_main_queue(), ^{
                lastRMWindowController = connectionWindows[ipAddr];
                if (!lastRMWindowController)
                    connectionWindows[ipAddr] = lastRMWindowController = [[self class] new];
                [NSApp activateIgnoringOtherApps:YES];
                [lastRMWindowController showWindow:nil];
                lastRMWindowController.device = [[RMDeviceController alloc] initSocket:clientSocket owner:lastRMWindowController];
                lastRMWindowController.window.title =
                    [NSString stringWithFormat:@"%s: %s (%@)",
                     EXPAND_AND_QUOTE(REMOTE_TITLE),
                     lastRMWindowController.device->device.remote.hostname,
                     ipAddr];
            });
        }
        else
            [NSThread sleepForTimeInterval:.5];
    }
}

+ (NSArray *)serverAddresses {
    NSMutableArray *addrs = [NSMutableArray new];
    char buffer[1024];
    struct ifconf ifc;
    ifc.ifc_len = sizeof buffer;
    ifc.ifc_buf = buffer;

    if (ioctl(serverSocket, SIOCGIFCONF, &ifc) < 0)
        [[self class] error:@"ioctl error %s", strerror(errno)];
    else
        for (char *ptr = buffer; ptr < buffer + ifc.ifc_len;) {
            struct ifreq *ifr = (struct ifreq *)ptr;
            size_t len = MAX(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
            ptr += sizeof(ifr->ifr_name) + len;	// for next one in buffer

            if (ifr->ifr_addr.sa_family != AF_INET)
                continue;	// ignore if not desired address family

            struct sockaddr_in *iaddr = (struct sockaddr_in *)&ifr->ifr_addr;
            [addrs addObject:[NSString stringWithUTF8String:inet_ntoa(iaddr->sin_addr)]];
        }
    
    return addrs;
}

- (instancetype)init {
    if ((self = [super initWithWindowNibName:self.className])) {
        [self loadWindow];
        [manager updateLoaderItems];
    }
    return self;
}

- (void)awakeFromNib {
    [manager updateLoaderItems];
}

- (void)showWindow:(id)sender {
    [super showWindow:sender];
    [drawer setContentSize:logView.frame.size];
    drawer.contentView = logView;
    [drawer openOnEdge:NSMaxXEdge];
}

- (RMImageView *)imageView {
    return imageView;
}

- (NSMenuItem *)macroMenu {
    return macroMenu;
}

- (void)updateImage:(CGImageRef)img {
    if (!self.device) return;
    struct _rmframe *framep = &self.device->frame;
    NSSize size = {framep->width*framep->imageScale, framep->height*framep->imageScale};
    NSImage *image = [[NSImage alloc] initWithCGImage:img size:size];
    [imageView performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:NO];
    if (framep->imageScale != 1.)
        image = [[NSImage alloc] initWithCGImage:img size:NSMakeSize(framep->width,framep->height)];
    [manager performSelectorOnMainThread:@selector(recordImage:) withObject:image waitUntilDone:NO];
    CGImageRelease(img);
}

- (void)loading:(BOOL)active {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (active)
            [spinner startAnimation:self];
        else
            [spinner stopAnimation:self];
    });
}

- (void)setDevice:(RMDeviceController *)device {
    if (!(_device = device)) {
        NSBundle *bundle = [NSBundle bundleForClass:[self class]];
        NSString *pngPath = [bundle pathForResource:@"iphone" ofType:@"png"];
        NSData *data = [NSData dataWithContentsOfFile:pngPath];
        NSImage *image = [[NSImage alloc] initWithData:data];
        [imageView performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:NO];
    }
}

static CGFloat windowTitleHeight = 22.;

- (void)resize:(NSSize)size {
    NSRect windowFrame = self.window.frame;
    CGFloat editableBorder = imageView.isEditable ? 3.0 * 2 : 0.;
    windowFrame.size = NSMakeSize(size.width + editableBorder,
                                  size.height + editableBorder + windowTitleHeight);

    aspect = 0.;

    [self.window setFrame:windowFrame display:YES animate:NO];

    if (self.window.frame.size.height != windowFrame.size.height) {
        CGFloat windowClipScale =
            (self.window.frame.size.height-windowTitleHeight)/
            (windowFrame.size.height-windowTitleHeight);
        windowFrame.size.width *= windowClipScale;
        windowFrame.size.height = self.window.frame.size.height;
        [self.window setFrame:windowFrame display:YES animate:YES];
    }

    aspect = size.height/size.width;
}

- (NSSize)windowWillResize:(NSWindow *)sender
                    toSize:(NSSize)frameSize {
    if (aspect)
        frameSize.height = frameSize.width*aspect+windowTitleHeight;
    return frameSize;
}

- (void)windowWillClose:(NSNotification *)notification {
    if (notification.object == self.window)
        [self.device shutdown];
}

- (void)logSet:(NSString *)macroHTML {
    [manager callJS:@"logSet" with:macroHTML];
}

- (void)logAdd:(NSString *)entry {
    NSLog(@"logAdd: %@", entry);
    [manager callJS:@"logAdd" with:entry];
}

- (void)replay:(NSString *)html {
    __block BOOL completed = YES;
    cancel = NO;

    static NSRegularExpression *parser;
    if (!parser)
        parser = [NSRegularExpression regularExpressionWithPattern:@"<div id=\"([^\"]+)\"[^>]*>\\s*("
                  "(Began|Moved|Ended) (?:t:)?([\\d.]+) (?:x:)?([\\d.]+) (?:y:)?([\\d.]+)( (?:x:)?([\\d.]+) (?:y:)?([\\d.]+))?|"
                  "Expect timeout:([\\d.]+) tolerance:(\\d+).+?"
                  "<span style=\"display:none\">([^<]+)</span>|"
                  "Device (?:w:)?(\\d+) (?:h:)?(\\d+) (?:iscale:)?(\\d+) (?:scale:)?(\\d+))" options:0 error:NULL];

    [parser enumerateMatchesInString:html options:0 range:NSMakeRange(0,html.length)
                          usingBlock:^(NSTextCheckingResult *result, NSMatchingFlags flags, BOOL *stop) {
      NSString *divID = [result groupAtIndex:1 inString:html];
      NSString *phase = [result groupAtIndex:3 inString:html];
      NSString *enc64 = [result groupAtIndex:12 inString:html];

      [manager performSelectorOnMainThread:@selector(logAnimate:) withObject:divID waitUntilDone:NO];

      if (phase) {
          struct _rmevent event;
          switch ([phase characterAtIndex:0]) {
              case 'B':
                  event.phase = [result groupAtIndex:7 inString:html] ? RMTouchBeganDouble : RMTouchBegan;
                  break;
              case 'M':
                  event.phase = RMTouchMoved;
                  break;
              case 'S':
                  event.phase = RMTouchStationary;
                  break;
              case 'E':
                  event.phase = RMTouchEnded;
                  break;
              case 'C':
                  event.phase = RMTouchCancelled;
                  break;
              default:
                  NSLog(@"Remote: Invalid event phase %@", phase);
          }

          NSTimeInterval wait = [result groupAtIndex:4 inString:html].doubleValue;
          while (wait && !cancel) {
              NSTimeInterval sleep = MIN(0.5, wait);
              [NSThread sleepForTimeInterval:sleep];
              wait -= sleep;
          }

          if (cancel)
              dispatch_sync(dispatch_get_main_queue(), ^{
                  switch ([[NSAlert alertWithMessageText:@"Replay Cancelled:" defaultButton:@"Cancel Replay"
                                          alternateButton:@"Cancel Wait Only" otherButton:nil
                                informativeTextWithFormat:@"Replay cancelled, do you wish to continue?"] runModal]) {

                      case NSAlertAlternateReturn:
                          cancel = FALSE;
                          break;
                  }
              });

          event.touches[0].x = [result groupAtIndex:5 inString:html].floatValue;
          event.touches[0].y = [result groupAtIndex:6 inString:html].floatValue;

          event.touches[1].x = [result groupAtIndex:8 inString:html].floatValue;
          event.touches[1].y = [result groupAtIndex:9 inString:html].floatValue;

          [self.device writeEvent:cancel?NULL:&event];
      }
      else if (enc64) {
          NSTimeInterval timeout = [result groupAtIndex:10 inString:html].doubleValue;
          unsigned tolerance = [result groupAtIndex:11 inString:html].intValue;
          RemoteCapture *snapshot = [self.device recoverBuffer:enc64];

          completed = NO;
          unsigned difference = ~0;
          NSTimeInterval start = [NSDate timeIntervalSinceReferenceDate];
          while (!cancel && [NSDate timeIntervalSinceReferenceDate] - start < timeout) {
              difference = [self.device differenceAgainst:snapshot];
              if (difference <= tolerance) {
                  completed = YES;
                  break;
              }
              [NSThread sleepForTimeInterval:.5];
          }

          if (!completed && !cancel) {
              dispatch_sync(dispatch_get_main_queue(), ^{
                  switch ([[NSAlert alertWithMessageText:@"Replay Expect Failed:" defaultButton:@"Cancel Replay"
                                          alternateButton:@"Update tolerance" otherButton:@"Update snapshot"
                                informativeTextWithFormat:@"Difference against reference of %u compared with of %u and timeout expired", difference, tolerance] runModal]) {

                      case NSAlertDefaultReturn:
                          cancel = TRUE;
                          break;

                      case NSAlertAlternateReturn:
                          [manager updateDivWithID:divID withInnerHTML:[self snapshot:snapshot tolerance:difference]];
                          break;

                      case NSAlertOtherReturn:
                          [manager updateDivWithID:divID withInnerHTML:[self snapshot:nil tolerance:tolerance]];
                          break;
                  }
              });
          }
      }
      else {
          float width = [result groupAtIndex:13 inString:html].floatValue;
          float height = [result groupAtIndex:14 inString:html].floatValue;
          float imageScale = [result groupAtIndex:15 inString:html].floatValue;

          dispatch_sync(dispatch_get_main_queue(), ^{
              if (self.device && (width != self.device->frame.width || height != self.device->frame.height))
                  switch ([[NSAlert alertWithMessageText:@"Replay Device Mismatch:" defaultButton:@"OK"
                                          alternateButton:@"Cancel" otherButton:nil
                                informativeTextWithFormat:@"Macro recorded on a device with a different screen resolution. Events will not match up with the interface. Proceed?"] runModal]) {
                      case NSAlertAlternateReturn:
                          cancel = TRUE;
                          break;
                  }
              else if (self.device && imageScale != self.device->frame.imageScale)
                  switch ([[NSAlert alertWithMessageText:@"Replay Resolution Mismatch:" defaultButton:@"OK"
                                          alternateButton:@"Cancel" otherButton:nil
                                informativeTextWithFormat:@"Macro recorded on a device with a different screen resolution (scale). Snapshots will not compare correctly. Proceed?"] runModal]) {
                      case NSAlertAlternateReturn:
                          cancel = TRUE;
                          break;
                  }
          });
      }
      
      *stop = cancel;
    }];

    [manager performSelectorOnMainThread:@selector(logAnimate:) withObject:@"" waitUntilDone:NO];
    
    NSString *msg = completed ? cancel ? @"Replay Cancelled" : @"Replay Complete" : @"Replay Failed";
    [self performSelectorOnMainThread:@selector(logAdd:) withObject:msg waitUntilDone:NO];
}

- (void)cancelReplay {
    cancel = TRUE;
}

- (void)reset {
    lastEvent = 0.;
}

- (NSTimeInterval)timeSinceLastEvent {
    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    if (!lastEvent)
        lastEvent = now;
    NSTimeInterval dt = now-lastEvent;
    lastEvent = now;
    return dt;
}

- (NSString *)takeSnapshot {
    return [self snapshot:nil tolerance:0];
}

- (NSString *)snapshot:(RemoteCapture *)buffer tolerance:(unsigned)tolerance {
    float defaultTimeout = 10;
    NSString *format = [NSString stringWithFormat:@" Expect timeout:%g tolerance:%d<br>"
                        "<img class=\"snapshot\" src=\"data:image/png;base64,%%@\" onclick=\"showSnapshot(this)\">"
                        "<span style=\"display:none\">%%@</span><p>",
                        defaultTimeout, tolerance];
    return [self.device snapshot:buffer withFormat:format];
}

- (NSImage *)recoverImage:(NSString *)enc64 {
    return [self.device recoverImage:enc64];
}

- (void)replayMacro:(NSString *)name {
    [manager replayMacro:name];
}

@end

