//
//  RPMacroManager.h
//  Remote
//
//  Created by John Holdsworth on 24/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface RMMacroManager : NSObject

- (void)updateLoaderItems;
- (id)callJS:(NSString *)func with:(NSString *)arg;

- (void)updateDivWithID:(NSString *)divID withInnerHTML:(NSString *)snapshotHTML;
- (void)logAnimate:(NSString *)divID;
- (void)replayMacro:(NSString *)name;
- (void)updateImage:(NSImage *)image;

@end
