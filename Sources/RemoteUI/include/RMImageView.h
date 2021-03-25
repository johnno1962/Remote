//
//  RMImageView.h
//  Remote
//
//  Created by John Holdsworth on 24/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import "RMDeviceController.h"

@interface RMImageView : NSImageView

- (void)drawTouches:(const struct _rmevent *)newEvent;

@end
