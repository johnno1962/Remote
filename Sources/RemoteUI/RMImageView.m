//
//  RMImageView.m
//  Remote
//
//  Created by John Holdsworth on 24/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//
//  Repo: https://github.com/johnno1962/Remote
//

#import "RMImageView.h"

// send mouse events to client app to be converted into UITouches
@implementation RMImageView {
    IBOutlet __weak id<RMDeviceDelegate> owner;
    struct _rmevent event;
    int touches;
}

- (void)drawTouches:(const struct _rmevent *)newEvent {
    if(newEvent) {
        event = *newEvent;
        switch(event.phase) {
            case RMTouchBeganDouble:
                touches = 2;
                break;

            case RMTouchBegan:
                touches = 1;
                break;

            case RMTouchMoved:
            case RMTouchStationary:
                break;

            case RMTouchEnded:
            case RMTouchCancelled:
                touches = 0;
                break;

            default:
                NSLog(@"RPCapture: Invalid event: %d", event.phase);
        }
    }
    else
        touches = 0;
    
    [self performSelectorOnMainThread:@selector(setNeedsDisplay)
                           withObject:nil waitUntilDone:NO];
}

- (void)drawRect:(NSRect)rect {
    [super drawRect:rect];
    if(!owner.device)
        return;

    [[NSColor colorWithCalibratedWhite:.5 alpha:.6] set];

    CGFloat radius = self.frame.size.height/20.,
        deviceHeight = owner.device->frame.height,
        scale = self.frame.size.height/deviceHeight;

    for(int t=0 ; t<touches ; t++) {
        CGFloat x = event.touches[t].x*scale,
            y = (deviceHeight-event.touches[t].y)*scale;
        [[NSBezierPath bezierPathWithOvalInRect:NSMakeRect(x-radius/2.,
                                                           y-radius/2.,
                                                           radius, radius)] fill];
    }
}

- (void)mouseDown:(NSEvent *)theEvent {
    [owner.device sendEvent:theEvent phase:RMTouchBegan];
}

- (void)mouseDragged:(NSEvent *)theEvent {
    [owner.device sendEvent:theEvent phase:RMTouchMoved];
}

- (void)mouseUp:(NSEvent *)theEvent {
    [owner.device sendEvent:theEvent phase:RMTouchEnded];
}

@end
