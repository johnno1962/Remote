//
//  RPMacroManager.m
//  Remote
//
//  Created by John Holdsworth on 24/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//
//  Repo: https://github.com/johnno1962/Remote
//  $Id: //depot/Remote/Sources/RemoteUI/RMMacroManager.m#7 $
//

#import "RMMacroManager.h"
#import <WebKit/WebKit.h>

#import "RMWindowController.h"
#import "RMPluginController.h"
#import <objc/runtime.h>

#ifdef INJECTION_III_APP
#import "InjectionIII-Swift.h"
#else
@interface TimeLapseBuilder : NSObject
- (instancetype)initWithImages:(NSArray *)images times:(NSArray *)times;
- (void)build:(NSURL *)url progress:(void (^)(id))progress
                            success:(void (^)(NSURL *))success
                            failure:(void (^)(NSError *))failure;
@end
#endif

@implementation RMMacroManager {
    IBOutlet __weak RMWindowController *owner;
    IBOutlet __weak NSTextField *macroName;
    IBOutlet __weak NSPopUpButton *loader;
    IBOutlet __weak WebView *webView;
    IBOutlet __weak NSButton *replayButton;

    IBOutlet NSWindow *snapshotWindow;
    IBOutlet __weak NSImageView *snapshot;

    IBOutlet __weak NSButton *bRecord, *bPause, *bStop;
    NSMutableArray<NSImage *> *images;
    NSMutableArray<NSNumber *> *times;
    NSTimeInterval lastImageTime, recordedTime;
    NSImage *lastImage;
}

- (void)awakeFromNib {
    NSURL *url = [[NSBundle bundleForClass:[self class]] URLForResource:@"log" withExtension:@"html"];
    [webView.mainFrame loadRequest:[NSURLRequest requestWithURL:url]];
    [webView setEditable:YES];
}

- (id)callJS:(NSString *)func with:(NSString *)arg {
    return [[webView windowScriptObject] callWebScriptMethod:func
                                               withArguments:@[arg?:@"nil"]];
}

- (void)webView:(WebView *)aWebView didFinishLoadForFrame:(WebFrame *)frame {
}

- (void)webView:(WebView *)webView addMessageToConsole:(NSDictionary *)message; {
    NSLog(@"%@", message);
}

- (void)webView:(WebView *)sender runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
    [[owner class] error:message];
}

- (NSString *)webView:(WebView *)sender runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText initiatedByFrame:(WebFrame *)frame {
    if ([prompt isEqualToString:@"snapshot"]) {
        snapshot.image = [owner recoverImage:defaultText];
        NSRect snapshotFrame = owner.window.frame;
        snapshotFrame.origin.x += snapshotFrame.size.width;
        [snapshotWindow setFrame:snapshotFrame display:YES animate:NO];
        [snapshotWindow makeKeyAndOrderFront:self];
    }
    else
        NSLog(@"RPServer: prompt(%@, %@)", prompt, defaultText);
    return nil;
}

- (NSString *)macroHome {
    NSString *macroHome = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/RemoteMacros"];
    NSFileManager *manager = [NSFileManager defaultManager];
    if (![manager fileExistsAtPath:macroHome])
        [manager createDirectoryAtPath:macroHome withIntermediateDirectories:YES attributes:nil error:NULL];
    return macroHome;
}

- (NSString *)macroPath:(NSString *)name {
    return [[self macroHome] stringByAppendingPathComponent:name];
}

- (NSString *)macroContents {
    return [[webView windowScriptObject] evaluateWebScript:@"$().innerHTML;"];
}

- (void)updateLoaderItems {
    NSArray *macros = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[self macroHome] error:NULL];
    macros = [[[macros sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)]
               reverseObjectEnumerator] allObjects];

    while (loader.itemArray.count > 3)
        [loader removeItemAtIndex:1];
    for (NSString *name in macros)
        [loader insertItemWithTitle:name atIndex:1];

    NSMenu *replayMenu = owner.macroMenu.submenu;
    id target = [[replayMenu itemArray][0] target];
    while (replayMenu.itemArray.count > 3)
        [replayMenu removeItemAtIndex:1];
    for (NSString *name in macros) {
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:name action:@selector(replayMacro:) keyEquivalent:@""];
        item.target = target;
        [replayMenu insertItem:item atIndex:1];
    }
}

- (IBAction)loader:(NSPopUpButton *)sender {
    NSString *name = [sender selectedItem].title;
    if ([name isEqualToString:@"Import Macro..."]) {
        NSOpenPanel* opener = [NSOpenPanel openPanel];
        [opener setTitle:@"Load Macro Definition..."];
        if ([opener runModal] != NSModalResponseOK)
            return;
        [owner logSet:[NSString stringWithContentsOfURL:opener.URL encoding:NSUTF8StringEncoding error:NULL]];
        macroName.stringValue = [[opener.URL.path lastPathComponent] stringByDeletingPathExtension];
    }
    else if ([name isEqualToString:@"Export Macro..."]) {
        NSSavePanel *saver = [NSSavePanel savePanel];
        [saver setTitle:@"Save Macro Definition"];
        [saver setExtensionHidden:NO];
        [saver setNameFieldStringValue:macroName.stringValue];
        if ([saver runModal] != NSModalResponseOK)
            return;
        if (![[self macroContents] writeToURL:saver.URL atomically:NO encoding:NSUTF8StringEncoding error:NULL])
            [[owner class] error:@"Unable to export macro"];
        else
            [owner logAdd:@"Macro exported."];
    }
    else {
        [owner logSet:[NSString stringWithContentsOfFile:[self macroPath:name] encoding:NSUTF8StringEncoding error:NULL]];
        macroName.stringValue = name;
    }
}

- (IBAction)save:sender {
    NSString *name = macroName.stringValue;
    if (!name.length)
        [[owner class] error:@"Enter a name to save the macro under."];
    else {
        if (![[self macroContents] writeToFile:[self macroPath:name] atomically:NO encoding:NSUTF8StringEncoding error:NULL])
            [[owner class] error:@"Error saving macro"];
        else
            [owner logAdd:[NSString stringWithFormat:@"Macro saved under name %@.", name]];
        [self updateLoaderItems];
    }
}

- (IBAction)delete:sender {
    NSString *name = macroName.stringValue;
    if (name.length == 0)
        [[owner class] error:@"Enter a macro name to delete"];
    else {
        if (![[NSFileManager defaultManager] removeItemAtPath:[self macroPath:name] error:NULL])
            [[owner class] error:@"Error deleting macro"];
        else
            [owner logAdd:@"Macro deleted."];
        macroName.stringValue = @"";
        [self updateLoaderItems];
    }
}

- (void)updateDivWithID:(NSString *)divID withInnerHTML:(NSString *)snapshotHTML {
    [[webView windowScriptObject] callWebScriptMethod:@"logUpdate"
                                        withArguments:@[divID, snapshotHTML]];
}

- (void)logAnimate:(NSString *)divID {
    [self callJS:@"logAnimate" with:divID];
    if (!divID.length)
        replayButton.enabled = YES;
}

- (void)replayMacro:(NSString *)name {
    [owner logSet:[NSString stringWithContentsOfFile:[self macroPath:name] encoding:NSUTF8StringEncoding error:NULL]];
    macroName.stringValue = name;
    [self replay:self];
}

- (IBAction)replay:sender {
    replayButton.enabled = NO;
    [owner performSelectorInBackground:@selector(replay:) withObject:[self macroContents]];
}

- (IBAction)cancel:sender {
    [owner cancelReplay];
}

- (IBAction)snapshot:sender {
    [owner logAdd:[owner takeSnapshot]];
}

- (IBAction)clear:sender {
    [owner logSet:@""];
    [owner reset];
}

- (IBAction)record:sender {
    lastImageTime = [NSDate timeIntervalSinceReferenceDate];
    images = [NSMutableArray new];
    times = [NSMutableArray new];
    recordedTime = 0.0;
    [self recordImage:lastImage];
    bRecord.enabled = FALSE;
    bStop.enabled = TRUE;
}

- (IBAction)pause:sender {
    if (bPause.state)
        [self recordImage:nil];
    else
        lastImageTime = [NSDate timeIntervalSinceReferenceDate];
}

- (IBAction)stop:sender {
    bRecord.enabled = TRUE;
    bStop.enabled = FALSE;
    [self recordImage:lastImage];

    NSString *movieTmp = [NSTemporaryDirectory()
        stringByAppendingPathComponent:@"remote.mov"];

    [[[TimeLapseBuilder alloc]
         initWithImages:images times:times]
        build:[NSURL fileURLWithPath:movieTmp]
     progress:^(NSProgress *progress){
            NSLog(@"Progress: %@", progress);
        }
      success:^(NSURL * _Nonnull url){
            NSLog(@"Success: %@", url);
            [[NSWorkspace sharedWorkspace]
             openURL:[[NSURL alloc]
                      initFileURLWithPath:url.path]];
            images = nil;
            times = nil;
        }
      failure:^(NSError * _Nonnull err){
        dispatch_async(dispatch_get_main_queue(), ^{
            [[owner class] error:@"Error: %@", err];
        });
        }];
}

- (void)recordImage:(NSImage *)image {
    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    if (lastImage && (!bPause.state || !image)) {
        recordedTime += now - lastImageTime;
        [images addObject:lastImage];
        [times addObject:[NSNumber numberWithDouble:recordedTime]];
    }

    lastImageTime = now;
    if (image)
        lastImage = image;
}

@end
