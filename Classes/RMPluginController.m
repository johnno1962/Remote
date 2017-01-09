//
//  XprobePluginMenuController.m
//  XprobePlugin
//
//  Created by John Holdsworth on 01/05/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import "RMPluginController.h"
#import "RMWindowController.h"

RMPluginController *remotePlugin;

@interface INPluginMenuController : NSObject
+ (BOOL)loadRemote:(NSString *)resourcePath;
+ (BOOL)loadBundleForPlugin:(NSString *)resourcePath;
@end

typedef NS_ENUM(int, DBGState) {
    DBGStateIdle,
    DBGStatePaused,
    DBGStateRunning
};

@interface DBGLLDBSession : NSObject
- (DBGState)state;
- (void)requestPause;
- (void)requestContinue;
- (void)evaluateExpression:(id)a0 threadID:(unsigned long)a1 stackFrameID:(unsigned long)a2 queue:(id)a3 completionHandler:(id)a4;
- (void)executeConsoleCommand:(id)a0 threadID:(unsigned long)a1 stackFrameID:(unsigned long)a2 ;
@end

@implementation RMPluginController {
//    IBOutlet RMWindowController *remote;
    IBOutlet NSMenuItem *windowItem;

    NSWindowController *lastWindowController;
    Class IDEWorkspaceWindowController;
    NSMutableData *scriptOutput;
    NSString *macroName;
}

+ (void)pluginDidLoad:(NSBundle *)plugin {
	static dispatch_once_t onceToken;
    NSString *currentApplicationName = [NSBundle mainBundle].infoDictionary[@"CFBundleName"];

    if ([currentApplicationName isEqual:@"Xcode"])
        dispatch_once(&onceToken, ^{
            remotePlugin = [[self alloc] init];
            dispatch_async( dispatch_get_main_queue(), ^{
                [remotePlugin applicationDidFinishLaunching:nil];
            } );
        });
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    if ( ![[NSBundle bundleForClass:[self class]] loadNibNamed:@"RMPluginController" owner:self topLevelObjects:NULL] ) {
        if ( [[NSAlert alertWithMessageText:@"Remote Plugin:"
                              defaultButton:@"OK" alternateButton:@"Goto GitHub" otherButton:nil
                  informativeTextWithFormat:@"Could not load interface nib. This is a problem when using Alcatraz since Xcode6. Please download and build from the sources on GitHub."]
              runModal] == NSAlertAlternateReturn )
            [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/johnno1962/Remote"]];
        return;
    }

    NSMenu *productMenu = [[NSApp mainMenu] itemWithTitle:@"Product"].submenu;
    [productMenu addItem:[NSMenuItem separatorItem]];
    [productMenu addItem:self.remoteMenu];

    NSMenu *windowMenu = [self windowMenu];
    NSInteger where = [windowMenu indexOfItemWithTitle:@"Bring All to Front"];
    if ( where <= 0 )
        NSLog( @"XprobeConsole: Could not locate Window menu item" );
    else
        [windowMenu insertItem:windowItem atIndex:where-1];

    IDEWorkspaceWindowController = NSClassFromString(@"IDEWorkspaceWindowController");
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(workspaceDidChange:)
                                                 name:NSWindowDidBecomeKeyNotification object:nil];

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [[RMWindowController class] startServer];
        remotePlugin = self;
    });
}

- (void)workspaceDidChange:(NSNotification *)notification {
    NSWindow *object = [notification object];
    NSWindowController *currentWindowController = [object windowController];
    if ([currentWindowController isKindOfClass:IDEWorkspaceWindowController])
        lastWindowController = currentWindowController;
}

- (NSString *)workspacePath {
    return [[[lastWindowController document] fileURL] path];
}

- (NSString *)resourcePath {
    return [[NSBundle bundleForClass:[self class]] resourcePath];
}

- (NSMenu *)windowMenu {
    return [[[NSApp mainMenu] itemWithTitle:@"Window"] submenu];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    return YES;
}

- (IBAction)load:sender {

    [lastRMWindowController showWindow:self];
    if ( lastRMWindowController.device )
        return;

    Class injectionPlugin = NSClassFromString(@"JuicePluginController");
    if ( [injectionPlugin respondsToSelector:@selector(loadBundleForPlugin:)] &&
        [injectionPlugin loadBundleForPlugin:[self resourcePath]] )
        return;


    injectionPlugin = NSClassFromString(@"INPluginMenuController");
    if ( [injectionPlugin respondsToSelector:@selector(loadRemote:)] &&
        [injectionPlugin loadRemote:[self resourcePath]] )
        return;

    DBGLLDBSession *session = [lastWindowController valueForKeyPath:@"workspace"
                               ".executionEnvironment.selectedLaunchSession.currentDebugSession"];

    if ( !session )
        [[NSAlert alertWithMessageText:@"Remote Plugin:"
                        defaultButton:@"OK" alternateButton:nil otherButton:nil
             informativeTextWithFormat:@"Program is not running."] runModal];
    else {
        if ( session.state != DBGStatePaused )
            [session requestPause];
        [self performSelector:@selector(loadBundle:) withObject:session afterDelay:.1];
    }
}

- (void)loadBundle:(DBGLLDBSession *)session {
    if ( session.state != DBGStatePaused )
        [self performSelector:@selector(loadBundle:) withObject:session afterDelay:.1];
    else
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND,0), ^{
            NSString *loader = [NSString stringWithFormat:@"p (void)[[NSBundle bundleWithPath:"
                                "@\"%@/SimBundle.loader\"] load]\r", [self resourcePath]];
            [session executeConsoleCommand:loader threadID:1 stackFrameID:0];
            dispatch_async(dispatch_get_main_queue(), ^{
                [session requestContinue];
            });
        });
}

- (IBAction)replayMacro:(NSMenuItem *)sender {
    if ( sender ) {
        macroName = [sender title];
        [self load:sender];
    }
    if ( !lastRMWindowController.device )
        [self performSelector:@selector(replayMacro:) withObject:nil afterDelay:.5];
    else
        [lastRMWindowController replayMacro:macroName];
}

- (IBAction)patch:(NSMenuItem *)sender {
    [self runScript:@"patch"];
}

- (IBAction)unpatch:(NSMenuItem *)sender {
    [self runScript:@"unpatch"];
}

- (void)runScript:(NSString *)script {
    NSMutableArray *args = [NSMutableArray arrayWithObjects:[self resourcePath], [self workspacePath], nil];
    [args addObjectsFromArray:[RMWindowController serverAddresses]];
    scriptOutput = [NSMutableData new];

    NSTask *task = [NSTask new];
    task.launchPath = [[NSBundle bundleForClass:[self class]] pathForResource:script ofType:@"py"];
    task.currentDirectoryPath = [self resourcePath];

    task.arguments = args;
    task.standardOutput = [self outputPipe];
    task.standardError = [self outputPipe];

    [task setTerminationHandler:^(NSTask *task) {
        NSString *output = [[NSString alloc] initWithData:scriptOutput encoding:NSUTF8StringEncoding];

        if ( task.terminationStatus )
            dispatch_async(dispatch_get_main_queue(), ^{
                [[NSAlert alertWithMessageText:@"Remote Plugin:"
                                 defaultButton:@"OK" alternateButton:nil otherButton:nil
                     informativeTextWithFormat:@"Script error: %@", output] runModal];
            });

        scriptOutput = nil;
    }];

    [task launch];
}

- (NSPipe *)outputPipe {
    NSPipe *pipe = [NSPipe pipe];
    [[pipe fileHandleForReading] setReadabilityHandler:^(NSFileHandle *pipe) {
        [scriptOutput appendData:[pipe availableData]];
    }];
    return pipe;
}

@end
