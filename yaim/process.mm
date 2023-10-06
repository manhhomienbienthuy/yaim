//
//  process.m
//  yaim
//
//  Created by naa on 2023/04/03.
//  Modified from source code of OpenKey by TuyenVM.
//

#import "engine.hpp"
#import "AppDelegate.h"
#import <IOKit/hidsystem/ev_keymap.h>

extern AppDelegate* appDelegate;

extern "C" {
    #define allowVietnamese (isVietnamese && isABCKeyboard)

    CGEventSourceRef myEventSource = nil;
    vKeyHookState* pData;
    BOOL _isFnPressed = false;
    map<CGKeyCode, CGKeyCode> FuncKeyMap = {
        {kVK_F1, NX_KEYTYPE_BRIGHTNESS_DOWN},
        {kVK_F2, NX_KEYTYPE_BRIGHTNESS_UP},
        {kVK_F5, NX_KEYTYPE_ILLUMINATION_DOWN},
        {kVK_F6, NX_KEYTYPE_ILLUMINATION_UP},
        {kVK_F7, NX_KEYTYPE_PREVIOUS},
        {kVK_F8, NX_KEYTYPE_PLAY},
        {kVK_F9, NX_KEYTYPE_NEXT},
        {kVK_F10, NX_KEYTYPE_MUTE},
        {kVK_F11, NX_KEYTYPE_SOUND_DOWN},
        {kVK_F12, NX_KEYTYPE_SOUND_UP},
    };
    map<CGKeyCode, NSString*> OpenAppMap = {
        {kVK_F3, @"com.apple.exposelauncher"},
        {kVK_F4, @"com.apple.launchpad.launcher"},
    };
    vector<CGKeyCode> ShortcutKeys = {
        kVK_Shift, kVK_RightShift, kVK_Control, kVK_RightControl};

    void init() {
        myEventSource = CGEventSourceCreate(kCGEventSourceStatePrivate);
        pData = (vKeyHookState*)vInit();
    }

    void restartEngine() {
        vInit();
    }

    void sendEmptyCharacter(CGEventTapProxy _proxy) {
        UniChar _newChar = 0x202f; // empty char
        CGEventRef _down = CGEventCreateKeyboardEvent(myEventSource, 0, true);
        CGEventRef _up = CGEventCreateKeyboardEvent(myEventSource, 0, false);
        CGEventKeyboardSetUnicodeString(_down, 1, &_newChar);
        CGEventKeyboardSetUnicodeString(_up, 1, &_newChar);
        CGEventTapPostEvent(_proxy, _down);
        CGEventTapPostEvent(_proxy, _up);
        CFRelease(_down);
        CFRelease(_up);
    }

    void sendBackspaces(CGEventTapProxy _proxy) {
        CGEventRef _down = CGEventCreateKeyboardEvent(myEventSource, kVK_Delete, true);
        CGEventRef _up = CGEventCreateKeyboardEvent(myEventSource, kVK_Delete, false);
        for (char _i = 0; _i < pData->backspaceCount + 1; _i++) {
            CGEventTapPostEvent(_proxy, _down);
            CGEventTapPostEvent(_proxy, _up);
        }
        CFRelease(_down);
        CFRelease(_up);
    }

    void sendCharData(CGEventTapProxy _proxy) {
        if (pData->newCharCount > 0) {
            CGEventRef _down = CGEventCreateKeyboardEvent(myEventSource, 0, true);
            CGEventRef _up = CGEventCreateKeyboardEvent(myEventSource, 0, false);
            CGEventKeyboardSetUnicodeString(_down, pData->newCharCount, pData->charData);
            CGEventKeyboardSetUnicodeString(_up, pData->newCharCount, pData->charData);
            CGEventTapPostEvent(_proxy, _down);
            CGEventTapPostEvent(_proxy, _up);
            CFRelease(_down);
            CFRelease(_up);
        }
    }

    bool checkHotKey(CGEventFlags _flag) {
        return (_flag & kCGEventFlagMaskControl) &&
            (_flag & kCGEventFlagMaskShift) &&
            !(_flag & kCGEventFlagMaskAlternate) &&
            !(_flag & kCGEventFlagMaskCommand);
    }

    /**
     * MAIN HOOK entry, very important function.
     * MAIN Callback.
     */
    CGEventRef callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
        // don't handle my event
        if (CGEventGetIntegerValueField(event, kCGEventSourceStateID) == CGEventSourceGetSourceStateID(myEventSource)) {
            return event;
        }

        if (type != kCGEventFlagsChanged && type != kCGEventKeyDown) {
            if (allowVietnamese) {
                restartEngine();
            }
            return event;
        }

        CGEventFlags _flag = CGEventGetFlags(event);
        CGKeyCode _keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // switch language shortcut
        if (type == kCGEventFlagsChanged) {
            if ((std::find(ShortcutKeys.begin(), ShortcutKeys.end(), _keycode) != ShortcutKeys.end()) &&
                checkHotKey(_flag)) {
                [appDelegate onInputMethodChanged];
                NSBeep();
                return nil;
            }
            if (_keycode == kVK_Function) {
                _isFnPressed = (_flag & kCGEventFlagMaskSecondaryFn) != 0;
            }
            return event;
        }

        if (_isFnPressed && _flag & kCGEventFlagMaskSecondaryFn) {
            if (FuncKeyMap.find(_keycode) != FuncKeyMap.end()) {
                CGKeyCode _code = FuncKeyMap[_keycode];
                CGEventRef _down = [[NSEvent otherEventWithType:NSEventTypeSystemDefined
                                                       location:NSZeroPoint
                                                  modifierFlags:0xa00
                                                      timestamp:0
                                                   windowNumber:0
                                                        context:nil
                                                        subtype:8
                                                          data1:((_code << 16) | (0xa << 8))
                                                          data2:-1]
                                    CGEvent];
                CGEventRef _up = [[NSEvent otherEventWithType:NSEventTypeSystemDefined
                                                     location:NSZeroPoint
                                                modifierFlags:0xb00
                                                    timestamp:0
                                                 windowNumber:0
                                                      context:nil
                                                      subtype:8
                                                        data1:((_code << 16) | (0xb << 8))
                                                        data2:-1]
                                  CGEvent];
                CGEventPost(kCGHIDEventTap, _down);
                CGEventPost(kCGHIDEventTap, _up);
                return nil;
            }

            if (OpenAppMap.find(_keycode) != OpenAppMap.end()) {
                NSURL* url = [[NSWorkspace sharedWorkspace]
                              URLForApplicationWithBundleIdentifier:OpenAppMap[_keycode]];
                [[NSWorkspace sharedWorkspace] openApplicationAtURL:url
                                                      configuration:[NSWorkspaceOpenConfiguration configuration]
                                                  completionHandler:nil];
                return nil;
            }
        }

        if (!allowVietnamese) {
            return event;
        }

        if ((_flag & kCGEventFlagMaskCommand) ||
            (_flag & kCGEventFlagMaskControl) ||
            (_flag & kCGEventFlagMaskNumericPad) ||
            (_flag & kCGEventFlagMaskHelp) ||
            (_flag & kCGEventFlagMaskSecondaryFn)) {
            restartEngine();
            return event;
        }

        UInt16 _charCode;
        UniCharCount _tmp = 0;
        CGEventKeyboardGetUnicodeString(event, 1, &_tmp, &_charCode);
        if (!_tmp) {
            restartEngine();
            return event;
        }

        if (_charCode == 0x08 && _flag & kCGEventFlagMaskAlternate) {
            _charCode = 0x7f;
        } else if (_charCode == 0x1b && _flag & kCGEventFlagMaskAlternate) {
            _charCode = 0x00;
        }

        vHandleKey(_charCode);
        if (pData->code == vDoNothing) {
            return event;
        } else {
            sendEmptyCharacter(proxy);
            sendBackspaces(proxy);
            sendCharData(proxy);
            return pData->code == vRestore ? event : nil;
        }
    }
}
