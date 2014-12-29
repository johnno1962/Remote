//
//  RPWindowController.h
//  Remote
//
//  Created by John Holdsworth on 20/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RMDeviceController;
@class RMImageView;

@interface RMWindowController : NSWindowController

@property (nonatomic) IBOutlet NSMenu *replayMenu;
@property (nonatomic) RMDeviceController *device;

+ (NSModalResponse)error:(NSString *)msg, ...;
- (NSArray *)serverAddresses;

- (void)replayMacro:(NSString *)name;
- (void)logSet:(NSString *)macroHTML;
- (void)logAdd:(NSString *)entry;

- (NSImage *)recoverImage:(NSString *)enc64;
- (NSString *)takeSnapshot;

- (void)cancelReplay;
- (void)reset;

@end

