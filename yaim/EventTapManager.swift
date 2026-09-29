// EventTapManager.swift
// yaim
//
//  Created by naa on 2026/09/29.
//  Modified from source code of OpenKey by TuyenVM.
//

import Cocoa

final class EventTapManager {

    private var eventTap: CFMachPort?
    private var runLoopSource: CFRunLoopSource?
    private(set) var isStarted = false

    let processor: EventProcessor

    init(processor: EventProcessor) {
        self.processor = processor
    }

    @discardableResult
    func start() -> Bool {
        guard !isStarted else { return true }

        processor.setup()

        let eventMask: CGEventMask =
            CGEventMaskBit(.keyDown) |
            CGEventMaskBit(.flagsChanged) |
            CGEventMaskBit(.leftMouseDown) |
            CGEventMaskBit(.rightMouseDown) |
            CGEventMaskBit(.leftMouseDragged) |
            CGEventMaskBit(.rightMouseDragged) |
            CGEventMaskBit(.otherMouseDown) |
            CGEventMaskBit(.otherMouseDragged)

        let refcon = Unmanaged.passUnretained(processor).toOpaque()

        guard let tap = CGEventTapCreate(
            kCGSessionEventTap,
            kCGHeadInsertEventTap,
            CGEventTapOptions(rawValue: 0)!,
            eventMask,
            eventTapCallback,
            refcon
        ) else {
            print("yaim: failed to create event tap")
            return false
        }

        eventTap = tap
        isStarted = true

        let source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0)
        runLoopSource = source
        CFRunLoopAddSource(CFRunLoopGetMain(), source, .commonModes)
        CGEventTapEnable(tap, true)

        return true
    }

    func stop() {
        guard isStarted, let tap = eventTap else { return }

        CGEventTapEnable(tap, false)

        if let source = runLoopSource {
            CFRunLoopRemoveSource(CFRunLoopGetMain(), source, CFRunLoopMode.defaultMode)
            runLoopSource = nil
        }

        CFMachPortInvalidate(tap)
        eventTap = nil
        isStarted = false
    }
}
