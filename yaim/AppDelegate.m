//
//  AppDelegate.m
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#import "AppDelegate.h"
#import "manager.h"

AppDelegate* appDelegate;
extern void RequestNewSession(void);

bool isVietnamese = true;

@interface AppDelegate ()

@end


@implementation AppDelegate

NSWindowController *_mainWC;
NSStatusItem *statusItem;
NSMenu *theMenu;
NSMenuItem* menuInputMethod;


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    appDelegate = self;

    [self registerSupportedNotification];
    [self createStatusBarMenu];
    dispatch_async(dispatch_get_main_queue(), ^{
        [manager initEventTap];
    });
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    [[NSUserDefaults standardUserDefaults] setBool:isVietnamese forKey:@"vietnamese"];
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


- (void)createStatusBarMenu {
    NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
    statusItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];
    statusItem.button.image = [NSImage imageNamed:@"vi"];

    theMenu = [[NSMenu alloc] initWithTitle:@""];
    [theMenu setAutoenablesItems:NO];

    menuInputMethod = [theMenu addItemWithTitle:@"English"
                                                     action:@selector(onInputMethodChanged)
                                              keyEquivalent:@""];
    [theMenu addItemWithTitle:@"Thoát" action:@selector(terminate:) keyEquivalent:@""];

    //set menu
    [statusItem setMenu:theMenu];

    [self getInitData];
}


- (void) getInitData {
    isVietnamese = [[NSUserDefaults standardUserDefaults] boolForKey:@"vietnamese"];
    [self updateIcon];
}


- (void)onInputMethodChanged {
    isVietnamese = !isVietnamese;
    [self updateIcon];
}


- (void)updateIcon {
    if (isVietnamese) {
        [menuInputMethod setTitle:@"English"];
        statusItem.button.image = [NSImage imageNamed:@"vi"];
    } else {
        [menuInputMethod setTitle:@"Tiếng Việt"];
        statusItem.button.image = [NSImage imageNamed:@"en"];
    }
}


- (void)receiveWakeNote: (NSNotification*)note {
    [manager initEventTap];
}


- (void)receiveSleepNote: (NSNotification*)note {
    [manager stopEventTap];
}


- (void)receiveActiveSpaceChanged: (NSNotification*)note {
    RequestNewSession();
}


- (void)registerSupportedNotification {
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self
                                                           selector: @selector(receiveWakeNote:)
                                                               name: NSWorkspaceDidWakeNotification object: NULL];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self
                                                           selector: @selector(receiveSleepNote:)
                                                               name: NSWorkspaceWillSleepNotification object: NULL];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self
                                                           selector: @selector(receiveActiveSpaceChanged:)
                                                               name: NSWorkspaceActiveSpaceDidChangeNotification object: NULL];
}


@end
