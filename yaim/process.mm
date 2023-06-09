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
    vector<CGKeyCode> AlphaKeys = {kVK_ANSI_A, kVK_ANSI_B, kVK_ANSI_C, kVK_ANSI_D, kVK_ANSI_E,
        kVK_ANSI_F, kVK_ANSI_G, kVK_ANSI_H, kVK_ANSI_I, kVK_ANSI_J, kVK_ANSI_K, kVK_ANSI_L,
        kVK_ANSI_M, kVK_ANSI_N, kVK_ANSI_O, kVK_ANSI_P, kVK_ANSI_Q, kVK_ANSI_R, kVK_ANSI_S,
        kVK_ANSI_T, kVK_ANSI_U, kVK_ANSI_V, kVK_ANSI_W, kVK_ANSI_X, kVK_ANSI_Y, kVK_ANSI_Z};

    void init() {
        myEventSource = CGEventSourceCreate(kCGEventSourceStatePrivate);
        pData = (vKeyHookState*)vKeyInit();
    }

    void restartEngine() {
        // send event signal to Engine
        vKeyHandleEvent(0, false, true);
    }

    void sendEmptyCharacter(CGEventTapProxy _proxy) {
        UniChar _newChar = 0x202F; // empty char
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
            Uint16 _newCharString[MAX_BUFF];
            for (char _i = pData->newCharCount - 1; _i >= 0; _i--) {
                Uint32 _tempChar = pData->charData[_i];
                if (_tempChar & PURE_CHARACTER_MASK) {
                    _newCharString[pData->newCharCount - 1 - _i] = _tempChar;
                } else if (!(_tempChar & CHAR_CODE_MASK)) {
                    _newCharString[pData->newCharCount - 1 - _i] = keyCodeToCharacter(_tempChar);
                } else {
                    _newCharString[pData->newCharCount - 1 - _i] = _tempChar;
                }
            }

            CGEventRef _newEventDown = CGEventCreateKeyboardEvent(myEventSource, 0, true);
            CGEventRef _newEventUp = CGEventCreateKeyboardEvent(myEventSource, 0, false);
            CGEventKeyboardSetUnicodeString(_newEventDown, pData->newCharCount, _newCharString);
            CGEventKeyboardSetUnicodeString(_newEventUp, pData->newCharCount, _newCharString);
            CGEventTapPostEvent(_proxy, _newEventDown);
            CGEventTapPostEvent(_proxy, _newEventUp);
            CFRelease(_newEventDown);
            CFRelease(_newEventUp);
        }
    }

    bool checkHotKey(CGEventFlags _flag) {
        return (_flag & kCGEventFlagMaskControl) != 0 &&
            (_flag & kCGEventFlagMaskShift) != 0 &&
            (_flag & kCGEventFlagMaskAlternate) == 0 &&
            (_flag & kCGEventFlagMaskCommand) == 0;
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
                return nil;
            }
            if (_keycode == kVK_Function) {
                _isFnPressed = (_flag & kCGEventFlagMaskSecondaryFn) != 0;
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

        // send event signal to Engine
        bool _isModifier = (_flag & kCGEventFlagMaskCommand) ||
            (_flag & kCGEventFlagMaskControl) ||
            (_flag & kCGEventFlagMaskAlternate) ||
            (_flag & kCGEventFlagMaskSecondaryFn) ||
            (_flag & kCGEventFlagMaskNumericPad) ||
            (_flag & kCGEventFlagMaskHelp);
        bool _isCaps = (_flag & kCGEventFlagMaskShift) ||
            (_flag & kCGEventFlagMaskAlphaShift &&
             std::find(AlphaKeys.begin(), AlphaKeys.end(), _keycode) != AlphaKeys.end());
        vKeyHandleEvent(_keycode, _isCaps, _isModifier);
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
