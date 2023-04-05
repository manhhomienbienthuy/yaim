//
//  manager.m
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#import "manager.h"

extern void init(void);

extern CGEventRef callback(CGEventTapProxy proxy,
                                  CGEventType type,
                                  CGEventRef event,
                                  void *refcon);

@interface manager ()

@end

@implementation manager {

}
static BOOL _isInited = NO;

static CFMachPortRef      eventTap;
static CGEventMask        eventMask;
static CFRunLoopSourceRef runLoopSource;

+ (BOOL)isInited {
    return _isInited;
}

+ (BOOL)initEventTap {
    if (_isInited)
        return true;

    init();

    // Create an event tap. We are interested in key presses.
    eventMask = ((1 << kCGEventKeyDown) |
                 (1 << kCGEventKeyUp) |
                 (1 << kCGEventFlagsChanged) |
                 (1 << kCGEventLeftMouseDown) |
                 (1 << kCGEventRightMouseDown) |
                 (1 << kCGEventLeftMouseDragged) |
                 (1 << kCGEventRightMouseDragged));

    eventTap = CGEventTapCreate(kCGSessionEventTap,
                                kCGHeadInsertEventTap,
                                0,
                                eventMask,
                                callback,
                                nil);

    if (!eventTap) {
        NSLog(@"failed to create event tap");
        return NO;
    }

    _isInited = YES;

    // Create a run loop source.
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);

    // Add to the current run loop.
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);

    // Enable the event tap.
    CGEventTapEnable(eventTap, true);

    // Set it all running.
    CFRunLoopRun();

    return YES;
}

+ (BOOL)stopEventTap {
    if (_isInited) { //release all object
        CFRunLoopStop(CFRunLoopGetCurrent());

        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
        CFRelease(runLoopSource);
        runLoopSource = nil;

        CFMachPortInvalidate(eventTap);
        CFRelease(eventTap);
        eventTap = nil;

        _isInited = false;
    }
    return YES;
}


@end
