// EventProcessor.swift
// yaim
//
//  Created by naa on 2026/09/29.
//  Modified from source code of OpenKey by TuyenVM.
//

import Cocoa
import Carbon
import IOKit.hidsystem

final class EventProcessor {

    private var eventSource: CGEventSource?
    private var isFnPressed = false
    let engine = Engine()

    // Weak back-reference to app delegate for mode-toggle callback
    weak var appDelegate: AppDelegate?

    // F1-F12 → media key codes
    private static let funcKeyMap: [CGKeyCode: Int32] = [
        CGKeyCode(kVK_F1):  Int32(NX_KEYTYPE_BRIGHTNESS_DOWN),
        CGKeyCode(kVK_F2):  Int32(NX_KEYTYPE_BRIGHTNESS_UP),
        CGKeyCode(kVK_F5):  Int32(NX_KEYTYPE_ILLUMINATION_DOWN),
        CGKeyCode(kVK_F6):  Int32(NX_KEYTYPE_ILLUMINATION_UP),
        CGKeyCode(kVK_F7):  Int32(NX_KEYTYPE_PREVIOUS),
        CGKeyCode(kVK_F8):  Int32(NX_KEYTYPE_PLAY),
        CGKeyCode(kVK_F9):  Int32(NX_KEYTYPE_NEXT),
        CGKeyCode(kVK_F10): Int32(NX_KEYTYPE_MUTE),
        CGKeyCode(kVK_F11): Int32(NX_KEYTYPE_SOUND_DOWN),
        CGKeyCode(kVK_F12): Int32(NX_KEYTYPE_SOUND_UP),
    ]

    // F3/F4 → launch app by bundle ID
    private static let openAppMap: [CGKeyCode: String] = [
        CGKeyCode(kVK_F3): "com.apple.exposelauncher",
        CGKeyCode(kVK_F4): "com.apple.launchpad.launcher",
    ]

    // Keys that, combined with Ctrl+Shift, trigger the language toggle
    private static let shortcutKeys: Set<CGKeyCode> = [
        CGKeyCode(kVK_Shift), CGKeyCode(kVK_RightShift),
        CGKeyCode(kVK_Control), CGKeyCode(kVK_RightControl),
    ]

    func setup() {
        eventSource = CGEventSource(stateID: .privateState)
        engine.reset()
    }

    func restartEngine() {
        engine.reset()
    }

    // MARK: - Event dispatch (called from C callback via refcon)

    func handleEvent(proxy: CGEventTapProxy, type: CGEventType, event: CGEvent) -> Unmanaged<CGEvent>? {
        // Don't process events we synthesised ourselves
        if let src = eventSource,
           Int64(CGEventSourceGetSourceStateID(src).rawValue) == CGEventGetIntegerValueField(event, .eventSourceStateID) {
            return Unmanaged.passRetained(event)
        }

        let allowVN = engine.isVietnamese && engine.isABCKeyboard

        if type != .flagsChanged && type != .keyDown {
            if allowVN { restartEngine() }
            return Unmanaged.passRetained(event)
        }

        let flags = event.flags
        let keycode = CGKeyCode(UInt32(CGEventGetIntegerValueField(event, .keyboardEventKeycode)))

        // ── Flags-changed: handle language toggle and Fn tracking ──
        if type == .flagsChanged {
            if Self.shortcutKeys.contains(keycode) && checkHotKey(flags) {
                DispatchQueue.main.async { [weak self] in
                    self?.appDelegate?.onInputMethodChanged()
                    NSSound.beep()
                }
                return nil
            }
            if keycode == CGKeyCode(kVK_Function) {
                isFnPressed = flags.contains(.secondaryFn)
            }
            return Unmanaged.passRetained(event)
        }

        // ── Fn + function key → media key or app launcher ──
        if isFnPressed && flags.contains(.secondaryFn) {
            if let mediaCode = Self.funcKeyMap[keycode] {
                postMediaKey(mediaCode)
                return nil
            }
            if let bundleID = Self.openAppMap[keycode] {
                openApp(bundleID: bundleID)
                return nil
            }
        }

        // ── Skip if Vietnamese input is off ──
        if !allowVN { return Unmanaged.passRetained(event) }

        // ── Skip modifier-combo keystrokes ──
        if flags.contains(.maskCommand) || flags.contains(.maskControl) ||
           flags.contains(.maskNumericPad) || flags.contains(.maskHelp) ||
           flags.contains(.secondaryFn) {
            restartEngine()
            return Unmanaged.passRetained(event)
        }

        // ── Extract Unicode character ──
        var charCode: UInt16 = 0
        var actualCount: Int = 0
        CGEventKeyboardGetUnicodeString(event, 1, &actualCount, &charCode)
        guard actualCount > 0 else {
            restartEngine()
            return Unmanaged.passRetained(event)
        }

        // Option+Backspace / Option+ESC
        if charCode == 0x08 && flags.contains(.maskAlternate) { charCode = 0x7f }
        else if charCode == 0x1b && flags.contains(.maskAlternate) { charCode = 0x00 }

        engine.handleKey(charCode)

        if engine.hookState.code == .doNothing {
            return Unmanaged.passRetained(event)
        }

        sendEmptyCharacter(proxy: proxy)
        sendBackspaces(proxy: proxy)
        sendCharData(proxy: proxy)

        return engine.hookState.code == .restore ? Unmanaged.passRetained(event) : nil
    }

    // MARK: - Hotkey check

    private func checkHotKey(_ flags: CGEventFlags) -> Bool {
        flags.contains(.maskControl) &&
        flags.contains(.maskShift) &&
        !flags.contains(.maskAlternate) &&
        !flags.contains(.maskCommand)
    }

    // MARK: - Synthetic event senders

    private func sendEmptyCharacter(proxy: CGEventTapProxy) {
        var ch: UniChar = 0x202f  // narrow no-break space
        guard let down = CGEvent(keyboardEventSource: eventSource, virtualKey: 0, keyDown: true),
              let up   = CGEvent(keyboardEventSource: eventSource, virtualKey: 0, keyDown: false)
        else { return }
        CGEventKeyboardSetUnicodeString(down, 1, &ch)
        CGEventKeyboardSetUnicodeString(up,   1, &ch)
        CGEventTapPostEvent(proxy, down)
        CGEventTapPostEvent(proxy, up)
    }

    private func sendBackspaces(proxy: CGEventTapProxy) {
        guard let down = CGEvent(keyboardEventSource: eventSource, virtualKey: CGKeyCode(kVK_Delete), keyDown: true),
              let up   = CGEvent(keyboardEventSource: eventSource, virtualKey: CGKeyCode(kVK_Delete), keyDown: false)
        else { return }
        let count = engine.hookState.backspaceCount + 1
        for _ in 0..<count {
            CGEventTapPostEvent(proxy, down)
            CGEventTapPostEvent(proxy, up)
        }
    }

    private func sendCharData(proxy: CGEventTapProxy) {
        let n = engine.hookState.newCharCount
        guard n > 0 else { return }
        guard let down = CGEvent(keyboardEventSource: eventSource, virtualKey: 0, keyDown: true),
              let up   = CGEvent(keyboardEventSource: eventSource, virtualKey: 0, keyDown: false)
        else { return }
        engine.hookState.charData.withUnsafeBufferPointer { ptr in
            CGEventKeyboardSetUnicodeString(down, n, ptr.baseAddress)
            CGEventKeyboardSetUnicodeString(up,   n, ptr.baseAddress)
        }
        CGEventTapPostEvent(proxy, down)
        CGEventTapPostEvent(proxy, up)
    }

    // MARK: - Media key

    private func postMediaKey(_ keyType: Int32) {
        let down = NSEvent.otherEvent(
            with: .systemDefined, location: .zero, modifierFlags: NSEvent.ModifierFlags(rawValue: 0xa00),
            timestamp: 0, windowNumber: 0, context: nil, subtype: 8,
            data1: Int((keyType << 16) | (0xa << 8)), data2: -1)
        let up = NSEvent.otherEvent(
            with: .systemDefined, location: .zero, modifierFlags: NSEvent.ModifierFlags(rawValue: 0xb00),
            timestamp: 0, windowNumber: 0, context: nil, subtype: 8,
            data1: Int((keyType << 16) | (0xb << 8)), data2: -1)
        if let d = down?.cgEvent { CGEventPost(.hidEventTap, d) }
        if let u = up?.cgEvent   { CGEventPost(.hidEventTap, u) }
    }

    // MARK: - Open app

    private func openApp(bundleID: String) {
        if let url = NSWorkspace.shared.urlForApplication(withBundleIdentifier: bundleID) {
            NSWorkspace.shared.openApplication(at: url,
                                               configuration: NSWorkspaceOpenConfiguration(),
                                               completionHandler: nil)
        }
    }
}

// The C-compatible tap callback (no captured context — state flows through refcon)
let eventTapCallback: CGEventTapCallBack = { proxy, type, event, refcon in
    guard let refcon else { return Unmanaged.passRetained(event) }
    let processor = Unmanaged<EventProcessor>.fromOpaque(refcon).takeUnretainedValue()
    return processor.handleEvent(proxy: proxy, type: type, event: event)
}
