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

#define OTHER_CONTROL_KEY (_flag & kCGEventFlagMaskCommand) || (_flag & kCGEventFlagMaskControl) || \
                          (_flag & kCGEventFlagMaskAlternate) || (_flag & kCGEventFlagMaskSecondaryFn) || \
                          (_flag & kCGEventFlagMaskNumericPad) || (_flag & kCGEventFlagMaskHelp)

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

    void SendKeyCode(Uint32 data) {
        UniChar _newChar = (Uint16)data;
        CGEventRef _newEventDown, _newEventUp;

        if (!(data & CHAR_CODE_MASK)) {
            _newEventDown = CGEventCreateKeyboardEvent(myEventSource, _newChar, true);
            _newEventUp = CGEventCreateKeyboardEvent(myEventSource, _newChar, false);
            CGEventFlags _privateFlag = CGEventGetFlags(_newEventDown);

            if (data & CAPS_MASK) {
                _privateFlag |= kCGEventFlagMaskShift;
            } else {
                _privateFlag &= ~kCGEventFlagMaskShift;
            }
            _privateFlag |= kCGEventFlagMaskNonCoalesced;

            CGEventSetFlags(_newEventDown, _privateFlag);
            CGEventSetFlags(_newEventUp, _privateFlag);
            CGEventTapPostEvent(_proxy, _newEventDown);
            CGEventTapPostEvent(_proxy, _newEventUp);
        } else {
            _newEventDown = CGEventCreateKeyboardEvent(myEventSource, 0, true);
            _newEventUp = CGEventCreateKeyboardEvent(myEventSource, 0, false);
            CGEventKeyboardSetUnicodeString(_newEventDown, 1, &_newChar);
            CGEventKeyboardSetUnicodeString(_newEventUp, 1, &_newChar);
            CGEventTapPostEvent(_proxy, _newEventDown);
            CGEventTapPostEvent(_proxy, _newEventUp);
        }
        CFRelease(_newEventDown);
        CFRelease(_newEventUp);
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
        CGEventTapPostEvent(_proxy, CGEventCreateKeyboardEvent (myEventSource, 51, true));
        CGEventTapPostEvent(_proxy, CGEventCreateKeyboardEvent (myEventSource, 51, false));
    }

    void SendNewCharString(const Uint16& offset=0) {
        int _i = 0;
        Uint16 _newCharSize = pData->newCharCount;
        bool _willContinuteSending = false;
        bool _willSendControlKey = false;
        Uint16 _newCharString[MAX_UNICODE_STRING];

        if (_newCharSize > 0) {
            for (int _j = pData->newCharCount - 1 - offset; _j >= 0; _j--) {
                if (_i >= 16) {
                    _willContinuteSending = true;
                    break;
                }

                Uint32 _tempChar = pData->charData[_j];
                if (_tempChar & PURE_CHARACTER_MASK) {
                    _newCharString[_i++] = _tempChar;
                } else if (!(_tempChar & CHAR_CODE_MASK)) {
                    _newCharString[_i++] = keyCodeToCharacter(_tempChar);
                } else {
                    _newCharString[_i++] = _tempChar;
                }
            }// end for
        }

        // if is restore
        if (!_willContinuteSending && pData->code == vRestore) {
            if (keyCodeToCharacter(_keycode) != 0) {
                _newCharSize++;
                _newCharString[_i++] = keyCodeToCharacter(_keycode | ((_flag & kCGEventFlagMaskAlphaShift) || (_flag & kCGEventFlagMaskShift) ? CAPS_MASK : 0));
            } else {
                _willSendControlKey = true;
            }
        }

        CGEventRef _newEventDown = CGEventCreateKeyboardEvent(myEventSource, 0, true);
        CGEventRef _newEventUp = CGEventCreateKeyboardEvent(myEventSource, 0, false);
        CGEventKeyboardSetUnicodeString(_newEventDown, _willContinuteSending ? 16 : _newCharSize - offset, _newCharString);
        CGEventKeyboardSetUnicodeString(_newEventUp, _willContinuteSending ? 16 : _newCharSize - offset, _newCharString);
        CGEventTapPostEvent(_proxy, _newEventDown);
        CGEventTapPostEvent(_proxy, _newEventUp);
        CFRelease(_newEventDown);
        CFRelease(_newEventUp);

        if (_willContinuteSending) {
            SendNewCharString(16);
        }

        // the case when hCode is vRestore, the word is invalid and last key is
        // control key such as TAB, LEFT ARROW, RIGHT ARROW,...
        if (_willSendControlKey) {
            SendKeyCode(_keycode);
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
            vKeyHandleEvent(vKeyEvent::Keyboard,
                            vKeyEventState::KeyDown,
                            _keycode,
                            _flag & kCGEventFlagMaskShift ? 1 : (_flag & kCGEventFlagMaskAlphaShift ? 2 : 0),
                            OTHER_CONTROL_KEY);
            if (pData->code == vDoNothing) { // do nothing
                return event;
            } else if (pData->code == vWillProcess || pData->code == vRestore) { // handle result signal

                // fix autocomplete
                if (pData->extCode != 4) {
                    SendEmptyCharacter();
                    pData->backspaceCount++;
                }

                // send backspace
                if (pData->backspaceCount > 0 && pData->backspaceCount < MAX_BUFF) {
                    for (int _i = 0; _i < pData->backspaceCount; _i++) {
                        SendBackspace();
                    }
                }

                // send new character
                SendNewCharString();
            }

            return nil;
        }

        return event;
    }
}
