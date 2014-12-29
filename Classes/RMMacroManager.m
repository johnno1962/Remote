//
//  RPMacroManager.m
//  Remote
//
//  Created by John Holdsworth on 24/12/2014.
//  Copyright (c) 2014 John Holdsworth. All rights reserved.
//

#import "RMMacroManager.h"
#import <WebKit/WebKit.h>

#import "RMWindowController.h"

@implementation RMMacroManager {
    IBOutlet __weak RMWindowController *owner;
    IBOutlet __weak NSTextField *macroName;
    IBOutlet __weak NSPopUpButton *loader;
    IBOutlet __weak WebView *webView;
    IBOutlet __weak NSButton *replayButton;

    IBOutlet NSWindow *snapshotWindow;
    IBOutlet __weak NSImageView *snapshot;
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
    NSLog( @"%@", message );
}

- (void)webView:(WebView *)sender runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
    [[owner class] error:message];
}

- (NSString *)webView:(WebView *)sender runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText initiatedByFrame:(WebFrame *)frame {
    if ( [prompt isEqualToString:@"snapshot"] ) {
        snapshot.image = [owner recoverImage:defaultText];
        NSRect snapshotFrame = owner.window.frame;
        snapshotFrame.origin.x += snapshotFrame.size.width;
        [snapshotWindow setFrame:snapshotFrame display:YES animate:NO];
        [snapshotWindow makeKeyAndOrderFront:self];
    }
    else
        NSLog( @"RPServer: prompt( %@, %@ )", prompt, defaultText );
    return nil;
}

- (NSString *)macroHome {
    NSString *macroHome = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/RemoteMacros"];
    NSFileManager *manager = [NSFileManager defaultManager];
    if ( ![manager fileExistsAtPath:macroHome] )
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

    while ( loader.itemArray.count > 3 )
        [loader removeItemAtIndex:1];
    for ( NSString *name in macros )
        [loader insertItemWithTitle:name atIndex:1];

    id target = [[owner.replayMenu itemArray][0] target];
    while ( owner.replayMenu.itemArray.count > 3 )
        [owner.replayMenu removeItemAtIndex:1];
    for ( NSString *name in macros ) {
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:name action:@selector(replayMacro:) keyEquivalent:@""];
        item.target = target;
        [owner.replayMenu insertItem:item atIndex:1];
    }
}

- (IBAction)loader:(NSPopUpButton *)sender {
    NSString *title = [sender selectedItem].title;
    if ( [title isEqualToString:@"Import Macro..."] ) {
        NSOpenPanel* opener = [NSOpenPanel openPanel];
        [opener setTitle:@"Load Macro Definition..."];
        if ( [opener runModal] != NSOKButton )
            return;
        [owner logSet:[NSString stringWithContentsOfURL:opener.URL encoding:NSUTF8StringEncoding error:NULL]];
        macroName.stringValue = [[opener.URL.path lastPathComponent] stringByDeletingPathExtension];
    }
    else if ( [title isEqualToString:@"Export Macro..."] ) {
        NSSavePanel *saver = [NSSavePanel savePanel];
        [saver setTitle:@"Save Macro Definition"];
        [saver setExtensionHidden:NO];
        [saver setNameFieldStringValue:macroName.stringValue];
        if ( [saver runModal] != NSOKButton )
            return;
        if ( ![[self macroContents] writeToURL:saver.URL atomically:NO encoding:NSUTF8StringEncoding error:NULL] )
            [[owner class] error:@"Unable to export macro"];
        else
            [owner logAdd:@"Macro exported."];
    }
    else {
        [owner logSet:[NSString stringWithContentsOfFile:[self macroPath:title] encoding:NSUTF8StringEncoding error:NULL]];
        macroName.stringValue = title;
    }
}

- (IBAction)save:sender {
    NSString *name = macroName.stringValue;
    if ( !name.length )
        [[owner class] error:@"Enter a name to save the macro under."];
    else {
        if ( ![[self macroContents] writeToFile:[self macroPath:name] atomically:NO encoding:NSUTF8StringEncoding error:NULL] )
            [[owner class] error:@"Error saving macro"];
        else
            [owner logAdd:@"Macro saved."];
        [self updateLoaderItems];
    }
}

- (IBAction)delete:sender {
    NSString *name = macroName.stringValue;
    if ( name.length == 0 )
        [[owner class] error:@"Enter a macro name to delete"];
    else {
        if ( ![[NSFileManager defaultManager] removeItemAtPath:[self macroPath:name] error:NULL] )
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
    if ( !divID.length )
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

@end
