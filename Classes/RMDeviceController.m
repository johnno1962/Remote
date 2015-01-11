//
//  RPDeviceController.m
//  Replay
//
//  Created by John Holdsworth on 21/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#define REMOTE_IMPL
#import "RMDeviceController.h"
#import "RMImageView.h"

#import <zlib.h>

#define COMPRESS_SNAPSHOT
struct _rmcompress { uLongf bytes; Bytef data[1]; };

@implementation  RemoteCapture(Recover)

// run length encoding used to only transmit differrences between frames
- (void)recover:(const void *)tmp against:(RemoteCapture *)prevbuff {

    register unsigned expectedDiff = 0, check = 0, *data = (unsigned *)tmp;
    for ( register unsigned
         *curr = self->buffer,
         *prev = prevbuff->buffer;
         curr < self->buffend ; ) {

        unsigned diff = *data++;
        unsigned count = diff & 0xff;

        diff &= 0xffffff00;
        check += *curr++ = (*prev++ + diff + expectedDiff) | 0x000000ff;
        if ( count ) {
            if ( count == 0xff )
                count = *data++;
            for ( unsigned i=0 ; i<count && curr < self->buffend ; i++ )
                check += *curr++ = (*prev++ + diff + expectedDiff) | 0x000000ff;
        }

        expectedDiff = curr[-1] - prev[-1];
    }

    if ( check != *data )
        NSLog( @"RemoteCapture: recover problem" );
}

@end

@implementation RMDeviceController {
    __weak id<RMDeviceDelegate> owner;
    int clientSocket;

    RemoteCapture *currentBuffer;
}

- (instancetype)initSocket:(int)socket owner:(id<RMDeviceDelegate>)theOwner {
    if ( (self = [super init]) ) {
        [owner = theOwner reset];
        clientSocket = socket;
        [self performSelectorInBackground:@selector(renderService) withObject:nil];
    }
    return self;
}

// process a connection
- (void)renderService {
    FILE *renderStream = fdopen( clientSocket, "r" );
    NSArray *buffers;
    int frameno = 0;

    if( fread(&device, 1, sizeof device, renderStream) != sizeof device )
        NSLog( @"Could not read device info" );

    NSString *deviceString = [NSString stringWithFormat:@"<div>Hardware %s</div>", device.machine];
    [(NSObject *)owner performSelectorOnMainThread:@selector(logSet:) withObject:deviceString waitUntilDone:NO];

    // loop through frames
    struct _rmframe newFrame;
    while ( fread(&newFrame, 1, sizeof newFrame, renderStream) == sizeof newFrame ) {

        // event from device
        if ( newFrame.length < 0 ) {
            int touchCount = -newFrame.length;

            struct _rmevent event;

            BOOL isMutipleStart = newFrame.phase == RMTouchBegan && touchCount > 1;
            event.phase = isMutipleStart ? RMTouchBeganDouble : newFrame.phase;

            NSMutableString *arg = [self startEvent:newFrame.phase];

            do {
                int touchno = newFrame.length + touchCount;
                if ( touchno < RMMAX_TOUCHES ) {
                    event.touches[touchno].x = newFrame.x;
                    event.touches[touchno].y = newFrame.y;
                }
                [arg appendFormat:@" %.1f %.1f", newFrame.x, newFrame.y];
            }
            while ( newFrame.length != -1 &&
                   fread(&newFrame, 1, sizeof newFrame, renderStream) == sizeof newFrame );

            [owner.imageView event:&event];

            [(NSObject *)owner performSelectorOnMainThread:@selector(logAdd:)
                                    withObject:arg waitUntilDone:NO];
            continue;
        }

        // resize display window/NSImageView
        NSSize  newSize = NSMakeSize(newFrame.width, newFrame.height);
        if ( !buffers || newFrame.imageScale != frame.imageScale ||
                newSize.width != frame.width || newSize.height != frame.height ) {

            dispatch_sync(dispatch_get_main_queue(), ^{
                [owner resize:newSize];
            });

            NSString *deviceString = [NSString stringWithFormat:@"Device %g %g %g %g",
                                      newFrame.width, newFrame.height, newFrame.imageScale, device.scale];
            [(NSObject *)owner performSelectorOnMainThread:@selector(logAdd:)
                                                withObject:deviceString waitUntilDone:NO];

            // buffer current and previous frame to only render differences
            buffers = nil;
            buffers = @[[[RemoteCapture alloc] initFrame:&newFrame],
                        [[RemoteCapture alloc] initFrame:&newFrame]];
            frameno = 0;
        }

        frame = newFrame;

        //NSLog( @"Incoming bytes from client: %u", frame.length );
        void *tmp = malloc(frame.length);
        if ( fread(tmp, 1, frame.length, renderStream) != frame.length )
            break;

        // alternate buffers
        RemoteCapture *buffer = buffers[frameno++%2];
        RemoteCapture *prevbuff = buffers[frameno%2];
        
        [buffer recover:tmp against:prevbuff];
        free(tmp);

        currentBuffer = buffer;

        // update image in display window
        CGImageRef img = [currentBuffer cgImage];
        NSImage *image = [[NSImage alloc] initWithCGImage:img size:newSize];
        CGImageRelease(img);
        [owner.imageView performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:NO];
    }

close:
    NSLog( @"renderService exits" );
    fclose( renderStream );
    owner.device = nil;
}

- (NSString *)snapshot:(RemoteCapture *)reference withFormat:(NSString *)format {
    if ( !reference )
        reference = currentBuffer;

    struct _rmframe ftmp = { frame.width, frame.height, .5 };
    RemoteCapture *btmp = [[RemoteCapture alloc] initFrame:&ftmp];
    CGImageRef img = [reference cgImage];

    CGContextScaleCTM(btmp->cg, 1., -1.);
    CGContextDrawImage(btmp->cg, CGRectMake(0., -frame.height, frame.width, frame.height), img);
    CGImageRelease(img);

    img = [btmp cgImage];
    NSMutableData *data = [NSMutableData new];
    CGImageDestinationRef destination = CGImageDestinationCreateWithData((__bridge CFMutableDataRef)data, kUTTypePNG, 1, NULL);
    CGImageDestinationAddImage(destination, img, nil);
    CGImageDestinationFinalize(destination);
    CGImageRelease(img);

    NSString *png64 = [data base64EncodedStringWithOptions:0];
    CFRelease(destination);

    NSData *out = [reference subtractAndEncode:nil];

#ifdef COMPRESS_SNAPSHOT
    struct _rmcompress *buff = malloc( sizeof buff->bytes+[out length]+100 );
    uLongf clen = buff->bytes = [out length];
    if ( compress(buff->data, &clen,
                  (const Bytef *)[out bytes], buff->bytes) != Z_OK )
        NSLog( @"RemoteCapture: Compression problem" );

    data = [NSMutableData dataWithBytesNoCopy:buff length:sizeof buff->bytes+clen freeWhenDone:YES];
#endif

    NSString *enc64 = [data base64EncodedStringWithOptions:0];
    return [NSString stringWithFormat:format, png64, enc64];
}

- (RemoteCapture *)recoverBuffer:(NSString *)enc64 {
    NSData *encData = [[NSData alloc] initWithBase64EncodedString:enc64 options:0];

#ifdef COMPRESS_SNAPSHOT
    struct _rmcompress *buff = (struct _rmcompress *)[encData bytes];
    uLong bytes = buff->bytes;
    void *buff2 = malloc(bytes);

    if ( uncompress(buff2, &bytes, buff->data, [encData length]-sizeof buff->bytes) != Z_OK )
        NSLog( @"RemoteCapture: Uncompress problem" );

    encData = [NSData dataWithBytesNoCopy:buff2 length:bytes freeWhenDone:YES];
#endif

    RemoteCapture *reference = [[RemoteCapture alloc] initFrame:&frame];
    [reference recover:[encData bytes] against:[[RemoteCapture alloc] initFrame:&frame]];
    return reference;
}

- (NSImage *)recoverImage:(NSString *)enc64 {
    CGImageRef img = [[self recoverBuffer:enc64] cgImage];
    NSImage *image = [[NSImage alloc] initWithCGImage:img size:NSMakeSize(frame.width, frame.height)];
    CGImageRelease(img);
    return image;
}

- (unsigned)differenceAgainst:(RemoteCapture *)snapshot {
    NSData *out = [currentBuffer subtractAndEncode:snapshot];
    return (unsigned)[out length]-REMOTE_MINDIFF;
}

- (void)writeEvent:(const struct _rmevent *)event {
    [owner.imageView event:event];
    if ( event && write( clientSocket, event, sizeof *event ) != sizeof *event )
        NSLog( @"Remote: event write error" );
}

- (NSMutableString *)startEvent:(RMTouchPhase)phase {
    NSString *phaseString;
    switch ( phase ) {
        case RMTouchBegan: phaseString = @"Began"; break;
        case RMTouchMoved: phaseString = @"Moved"; break;
        case RMTouchStationary: phaseString = @"Stationary"; break;
        case RMTouchEnded: phaseString = @"Ended"; break;
        case RMTouchCancelled: phaseString = @"Cancelled"; break;
        default: phaseString = @"Unknown";
    }
    return [NSMutableString stringWithFormat:@"%@ %.3f",
            phaseString, [owner timeSinceLastEvent]];
}

- (void)sendEvent:(NSEvent *)theEvent phase:(RMTouchPhase)phase {
    NSPoint loc = theEvent.locationInWindow;
    float locScale = frame.height/owner.imageView.frame.size.height;
    struct _rmevent event = { phase, loc.x*locScale,
        (owner.imageView.frame.size.height-loc.y)*locScale };

    [self writeEvent:&event];

    NSMutableString *arg = [self startEvent:phase];
    [arg appendFormat:@" %.1f %.1f", event.touches[0].x, event.touches[0].y];
    [owner logAdd:arg];
}

@end
