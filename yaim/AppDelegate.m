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
bool isABCKeyboard = true;

@interface AppDelegate ()

@end


@implementation AppDelegate

NSStatusItem *statusItem;


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    appDelegate = self;

    [self registerSupportedNotification];
    [self createStatusBarMenu];
    dispatch_async(dispatch_get_main_queue(), ^{
        [manager initEventTap];
    });
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    [[NSUserDefaults standardUserDefaults] setBool:isVietnamese
                                            forKey:@"vietnamese"];
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


- (void)menuDidClose:(NSMenu *)_menu {
    statusItem.menu = nil;
}


- (void)createStatusBarMenu {
    NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
    statusItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];
    statusItem.button.image = [NSImage imageNamed:@"vi"];
    statusItem.button.action = @selector(menuAction:);
    [statusItem.button sendActionOn:NSEventMaskLeftMouseDown | NSEventMaskRightMouseDown];

    [self getInitData];
}


- (void)menuAction:(id)sender {
    NSEvent *event = [[NSApplication sharedApplication] currentEvent];

    if (event.type == NSEventTypeRightMouseDown) {
        NSMenu *theMenu = [[NSMenu alloc] initWithTitle:@""];
        [theMenu setDelegate:self];
        [theMenu setAutoenablesItems:NO];
        [theMenu addItemWithTitle:@"VIE / EN"
                           action:@selector(onInputMethodChanged)
                    keyEquivalent:@"e"];
        [theMenu addItemWithTitle:@"Thoát"
                           action:@selector(terminate:)
                    keyEquivalent:@"q"];
        statusItem.menu = theMenu;
        [statusItem.button performClick:nil];
    } else {
        [self onInputMethodChanged];
    }
}


- (void)getInitData {
    isVietnamese = [[NSUserDefaults standardUserDefaults]
                    boolForKey:@"vietnamese"];
    [self updateIcon];
    [self receiveInputChanged:nil];
}


- (void)onInputMethodChanged {
    isVietnamese = !isVietnamese;
    [self updateIcon];
}


- (void)updateIcon {
    statusItem.button.image = [NSImage imageNamed:(isVietnamese ? @"vi" : @"en")];
}


- (void)receiveWakeNote:(NSNotification*)note {
    [manager initEventTap];
}


- (void)receiveSleepNote:(NSNotification*)note {
    [manager stopEventTap];
}


- (void)receiveActiveSpaceChanged:(NSNotification*)note {
    RequestNewSession();
}


- (void)receiveInputChanged:(NSNotification*)note {
    TISInputSourceRef isource = TISCopyCurrentKeyboardInputSource();
    NSString* layoutID = (__bridge NSString*)TISGetInputSourceProperty(isource, kTISPropertyInputSourceID);
    isABCKeyboard = [layoutID isEqualToString:@"com.apple.keylayout.ABC"];
    CFRelease(isource);
}


- (void)registerSupportedNotification {
    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                                                           selector:@selector(receiveWakeNote:)
                                                               name:NSWorkspaceDidWakeNotification
                                                             object:nil];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                                                           selector:@selector(receiveSleepNote:)
                                                               name:NSWorkspaceWillSleepNotification
                                                             object:nil];

    [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver:self
                                                           selector:@selector(receiveActiveSpaceChanged:)
                                                               name:NSWorkspaceActiveSpaceDidChangeNotification
                                                             object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(receiveInputChanged:)
                                                 name: NSTextInputContextKeyboardSelectionDidChangeNotification
                                               object:nil];
}


@end
