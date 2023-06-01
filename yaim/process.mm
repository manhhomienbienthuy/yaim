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

#define MAX_UNICODE_STRING 20

extern AppDelegate* appDelegate;

extern "C" {
    CGEventSourceRef myEventSource = nil;
    vKeyHookState* pData;
    CGKeyCode _keycode;
    CGEventFlags _flag;
    CGEventTapProxy _proxy;
    BOOL _isFnPressed = false;
    map<int, int> FuncKeyMap = {
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
    vector<int> ShortcutKeys = {kVK_Shift, kVK_RightShift, kVK_Control, kVK_RightControl};

    void init() {
        myEventSource = CGEventSourceCreate(kCGEventSourceStatePrivate);
        pData = (vKeyHookState*)vKeyInit();
    }

    void RequestNewSession() {
        // send event signal to Engine
        vKeyHandleEvent(vKeyEvent::Mouse, vKeyEventState::MouseDown, 0);
    }

    void SendEmptyCharacter() {
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

    void SendBackspace() {
        CGEventRef _newEventDown = CGEventCreateKeyboardEvent (myEventSource, kVK_Delete, true);
        CGEventRef _newEventUp = CGEventCreateKeyboardEvent (myEventSource, kVK_Delete, false);
        for (int _i = 0; _i < pData->backspaceCount; _i++) {
            CGEventTapPostEvent(_proxy, _newEventDown);
            CGEventTapPostEvent(_proxy, _newEventUp);
        }
        CFRelease(_newEventDown);
        CFRelease(_newEventUp);
    }

    void SendNewCharString() {
        if (pData->newCharCount > 0) {
            Uint16 _newCharString[MAX_UNICODE_STRING];
            for (int _i = pData->newCharCount - 1; _i >= 0; _i--) {
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

    bool checkHotKey() {
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

        _flag = CGEventGetFlags(event);
        _keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // switch language shortcut; convert hotkey
        if (type == kCGEventFlagsChanged) {
            if ((std::find(ShortcutKeys.begin(), ShortcutKeys.end(), _keycode) != ShortcutKeys.end()) &&
                checkHotKey()) {
                [appDelegate onInputMethodChanged];
                startNewSession();
                return nil;
            }
            if (_keycode == kVK_Function) {
                _isFnPressed = (_flag & kCGEventFlagMaskSecondaryFn) != 0;
            }
        }

        // Also check correct event hooked
        if ((type != kCGEventKeyDown) && (type != kCGEventKeyUp) &&
            (type != kCGEventLeftMouseDown) && (type != kCGEventRightMouseDown) &&
            (type != kCGEventLeftMouseDragged) && (type != kCGEventRightMouseDragged))
            return event;


        if (_isFnPressed && type == kCGEventKeyDown && _flag & kCGEventFlagMaskSecondaryFn) {
            if (FuncKeyMap.find(_keycode) != FuncKeyMap.end()) {
                int code = FuncKeyMap[_keycode];
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

        // if is in english mode
        if (!isVietnamese || !isABCKeyboard) {
            return event;
        }

        // handle mouse
        if (type == kCGEventLeftMouseDown || type == kCGEventRightMouseDown || type == kCGEventLeftMouseDragged || type == kCGEventRightMouseDragged) {
            RequestNewSession();
            return event;
        }

        _proxy = proxy;

        // handle keyboard
        if (type == kCGEventKeyDown) {
            // send event signal to Engine
            bool controlKeys = (_flag & kCGEventFlagMaskCommand) ||
                (_flag & kCGEventFlagMaskControl) ||
                (_flag & kCGEventFlagMaskAlternate) ||
                (_flag & kCGEventFlagMaskSecondaryFn) ||
                (_flag & kCGEventFlagMaskNumericPad) ||
                (_flag & kCGEventFlagMaskHelp);
            vKeyHandleEvent(vKeyEvent::Keyboard,
                            vKeyEventState::KeyDown,
                            _keycode,
                            _flag & kCGEventFlagMaskShift ? 1 : (_flag & kCGEventFlagMaskAlphaShift ? 2 : 0),
                            controlKeys);
            if (pData->code == vDoNothing) {
                return event;
            } else {
                SendEmptyCharacter();
                pData->backspaceCount++;
                SendBackspace();
                SendNewCharString();
                return pData->code == vRestore ? event : nil;
            }
        }

        return event;
    }
}
