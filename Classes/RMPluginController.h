//
//  XprobePluginMenuController.h
//  XprobePlugin
//
//  Created by John Holdsworth on 01/05/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface RMPluginController : NSObject <NSApplicationDelegate>
@property IBOutlet NSMenuItem *remoteMenu;
- (IBAction)patch:(NSMenuItem *)sender;
- (IBAction)unpatch:(NSMenuItem *)sender;
- (void)runScript:(NSString *)script;
- (NSString *)workspacePath;
@end
