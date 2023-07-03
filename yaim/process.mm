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
    CGEventSourceRef myEventSource = nil;
    vKeyHookState* pData;
    BOOL _isFnPressed = false;
    map<CGKeyCode, CGKeyCode> FuncKeyMap = {
        {kVK_F1, NX_KEYTYPE_BRIGHTNESS_DOWN},
        {kVK_F2, NX_KEYTYPE_BRIGHTNESS_UP},
//        {kVK_F3, NX_KEYTYPE_CONTRAST_DOWN},
//        {kVK_F4, NX_KEYTYPE_CONTRAST_UP},
        {kVK_F5, NX_KEYTYPE_ILLUMINATION_DOWN},
        {kVK_F6, NX_KEYTYPE_ILLUMINATION_UP},
        {kVK_F7, NX_KEYTYPE_PREVIOUS},
        {kVK_F8, NX_KEYTYPE_PLAY},
        {kVK_F9, NX_KEYTYPE_NEXT},
        {kVK_F10, NX_KEYTYPE_MUTE},
        {kVK_F11, NX_KEYTYPE_SOUND_DOWN},
        {kVK_F12, NX_KEYTYPE_SOUND_UP},
    };
    vector<CGKeyCode> ShortcutKeys = {kVK_Shift, kVK_RightShift, kVK_Control, kVK_RightControl};

    void init() {
        myEventSource = CGEventSourceCreate(kCGEventSourceStatePrivate);
        pData = (vKeyHookState*)vInit();
    }

    void restartEngine() {
        vInit();
    }

    void sendEmptyCharacter(CGEventTapProxy _proxy) {
        UniChar _newChar = 0x202f; // empty char
        CGEventRef _newEventDown = CGEventCreateKeyboardEvent(myEventSource, 0, true);
        CGEventRef _newEventUp = CGEventCreateKeyboardEvent(myEventSource, 0, false);
        CGEventKeyboardSetUnicodeString(_newEventDown, 1, &_newChar);
        CGEventKeyboardSetUnicodeString(_newEventUp, 1, &_newChar);
        CGEventTapPostEvent(_proxy, _newEventDown);
        CGEventTapPostEvent(_proxy, _newEventUp);
        CFRelease(_newEventDown);
        CFRelease(_newEventUp);
    }

    void sendBackspaces(CGEventTapProxy _proxy) {
        CGEventRef _newEventDown = CGEventCreateKeyboardEvent (myEventSource, kVK_Delete, true);
        CGEventRef _newEventUp = CGEventCreateKeyboardEvent (myEventSource, kVK_Delete, false);
        for (char _i = 0; _i < pData->backspaceCount + 1; _i++) {
            CGEventTapPostEvent(_proxy, _newEventDown);
            CGEventTapPostEvent(_proxy, _newEventUp);
        }
        CFRelease(_newEventDown);
        CFRelease(_newEventUp);
    }

    void sendCharData(CGEventTapProxy _proxy) {
        if (pData->newCharCount > 0) {
            CGEventRef _newEventDown = CGEventCreateKeyboardEvent(myEventSource, 0, true);
            CGEventRef _newEventUp = CGEventCreateKeyboardEvent(myEventSource, 0, false);
            CGEventKeyboardSetUnicodeString(_newEventDown, pData->newCharCount, pData->charData);
            CGEventKeyboardSetUnicodeString(_newEventUp, pData->newCharCount, pData->charData);
            CGEventTapPostEvent(_proxy, _newEventDown);
            CGEventTapPostEvent(_proxy, _newEventUp);
            CFRelease(_newEventDown);
            CFRelease(_newEventUp);
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
        // dont handle my event
        if (CGEventGetIntegerValueField(event, kCGEventSourceStateID) == CGEventSourceGetSourceStateID(myEventSource)) {
            return event;
        }

        // handle mouse
        if (type == kCGEventLeftMouseDown || type == kCGEventRightMouseDown || type == kCGEventLeftMouseDragged || type == kCGEventRightMouseDragged) {
            restartEngine();
            return event;
        }

        CGEventFlags _flag = CGEventGetFlags(event);
        CGKeyCode _keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // switch language shortcut
        if (type == kCGEventFlagsChanged) {
            if ((std::find(ShortcutKeys.begin(), ShortcutKeys.end(), _keycode) != ShortcutKeys.end()) &&
                checkHotKey(_flag)) {
                [appDelegate onInputMethodChanged];
                NSBeep();
                return nil;
            }
            if (_keycode == kVK_Function) {
                _isFnPressed = (_flag & kCGEventFlagMaskSecondaryFn);
            }
        }

        if (_isFnPressed && type == kCGEventKeyDown && _flag & kCGEventFlagMaskSecondaryFn) {
            if (FuncKeyMap.find(_keycode) != FuncKeyMap.end()) {
                CGKeyCode code = FuncKeyMap[_keycode];
                CGEventRef _newEventDown = [[NSEvent otherEventWithType:NSEventTypeSystemDefined
                                                               location:NSZeroPoint
                                                          modifierFlags:0xa00
                                                              timestamp:0
                                                           windowNumber:0
                                                                context:nil
                                                                subtype:8
                                                                  data1:((code << 16) | (0xa << 8))
                                                                  data2:-1]
                                            CGEvent];
                CGEventRef _newEventUp = [[NSEvent otherEventWithType:NSEventTypeSystemDefined
                                                             location:NSZeroPoint
                                                        modifierFlags:0xb00
                                                            timestamp:0
                                                         windowNumber:0
                                                              context:nil
                                                              subtype:8
                                                                data1:((code << 16) | (0xb << 8))
                                                                data2:-1]
                                          CGEvent];
                CGEventPost(kCGHIDEventTap, _newEventDown);
                CGEventPost(kCGHIDEventTap, _newEventUp);
                return nil;
            }
        }

        if (type != kCGEventKeyDown || !isVietnamese || !isABCKeyboard) {
            return event;
        }

        if ((_flag & kCGEventFlagMaskCommand) ||
            (_flag & kCGEventFlagMaskControl) ||
            (_flag & kCGEventFlagMaskNumericPad) ||
            (_flag & kCGEventFlagMaskHelp)) {
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
