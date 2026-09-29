// AppDelegate.swift
// yaim
//
//  Created by naa on 2026/09/29.
//  Modified from source code of OpenKey by TuyenVM.
//

import Cocoa
import Carbon

@objc(AppDelegate)
class AppDelegate: NSObject, NSApplicationDelegate {

    private var statusItem: NSStatusItem?
    private var tapManager: EventTapManager!

    // Shared input state (read by EventProcessor)
    var isVietnamese = true {
        didSet { processor.engine.isVietnamese = isVietnamese }
    }
    var isABCKeyboard = true {
        didSet { processor.engine.isABCKeyboard = isABCKeyboard }
    }

    private let processor = EventProcessor()

    func applicationDidFinishLaunching(_ notification: Notification) {
        processor.appDelegate = self

        tapManager = EventTapManager(processor: processor)

        registerNotifications()
        createStatusBarMenu()

        loadPreferences()
        detectKeyboard()

        DispatchQueue.main.async { [weak self] in
            self?.tapManager.start()
        }
    }

    func applicationWillTerminate(_ notification: Notification) {
        UserDefaults.standard.set(isVietnamese, forKey: "vietnamese")
    }

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool { true }

    // MARK: - Status bar

    private func createStatusBarMenu() {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        guard let button = statusItem?.button else { return }
        button.action = #selector(menuAction(_:))
        button.sendAction(on: [.leftMouseDown, .rightMouseDown])
        updateIcon()
    }

    @objc private func menuAction(_ sender: Any?) {
        let event = NSApp.currentEvent
        if event?.type == .rightMouseDown && event?.modifierFlags.contains(.command) == true {
            NSApp.terminate(nil)
        } else {
            onInputMethodChanged()
        }
    }

    @objc func onInputMethodChanged() {
        isVietnamese = !isVietnamese
        updateIcon()
        processor.restartEngine()
    }

    private func updateIcon() {
        statusItem?.button?.image = NSImage(named: isVietnamese ? "vi" : "en")
        statusItem?.button?.image?.isTemplate = true
    }

    // MARK: - Preferences

    private func loadPreferences() {
        isVietnamese = UserDefaults.standard.bool(forKey: "vietnamese")
        updateIcon()
    }

    // MARK: - Keyboard layout detection

    @objc private func detectKeyboard() {
        guard let src = TISCopyCurrentKeyboardInputSource() else { return }
        defer { CFRelease(src) }
        if let ptr = TISGetInputSourceProperty(src, kTISPropertyInputSourceID) {
            let id = Unmanaged<CFString>.fromOpaque(ptr).takeUnretainedValue() as String
            isABCKeyboard = (id == "com.apple.keylayout.ABC")
        }
    }

    // MARK: - System notifications

    private func registerNotifications() {
        let wsnc = NSWorkspace.shared.notificationCenter

        wsnc.addObserver(self, selector: #selector(handleWake(_:)),
                         name: NSWorkspace.didWakeNotification, object: nil)
        wsnc.addObserver(self, selector: #selector(handleSleep(_:)),
                         name: NSWorkspace.willSleepNotification, object: nil)
        wsnc.addObserver(self, selector: #selector(handleSpaceChange(_:)),
                         name: NSWorkspace.activeSpaceDidChangeNotification, object: nil)
        wsnc.addObserver(self, selector: #selector(handleSpaceChange(_:)),
                         name: NSWorkspace.didActivateApplicationNotification, object: nil)

        NotificationCenter.default.addObserver(
            self, selector: #selector(detectKeyboard),
            name: NSTextInputContext.keyboardSelectionDidChangeNotification, object: nil)
    }

    @objc private func handleWake(_ note: Notification) {
        tapManager.start()
    }

    @objc private func handleSleep(_ note: Notification) {
        tapManager.stop()
    }

    @objc private func handleSpaceChange(_ note: Notification) {
        processor.restartEngine()
    }
}
