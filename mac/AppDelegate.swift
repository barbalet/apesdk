/****************************************************************

 AppDelegate.swift

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 ****************************************************************/

import AVFoundation
import Cocoa
import MetalKit

#if DEBUG
private func immersiveApeDebugLog(_ message: String) {
    let line = "[ImmersiveApe] \(message)\n"
    let url = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent("immersiveape-launch.log")

    if let data = line.data(using: .utf8) {
        if FileManager.default.fileExists(atPath: url.path) == false {
            try? data.write(to: url)
            return
        }

        if let handle = try? FileHandle(forWritingTo: url) {
            defer { try? handle.close() }
            do {
                try handle.seekToEnd()
                try handle.write(contentsOf: data)
            } catch {
            }
        }
    }
}
#else
private func immersiveApeDebugLog(_ message: String) {
}
#endif

@MainActor
final class AppDelegate: NSObject, NSApplicationDelegate {
    private(set) var windowController: ImmersiveApeWindowController?
    private var legacyShared: SimulationShared?
    private var eventMonitor: Any?

    private var appName: String {
        let bundleName = Bundle.main.object(forInfoDictionaryKey: kCFBundleNameKey as String) as? String
        return bundleName?.isEmpty == false ? bundleName! : "Immersive Ape"
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        immersiveApeDebugLog("applicationDidFinishLaunching")
        NSApp.setActivationPolicy(.regular)
        NSApp.mainMenu = makeMainMenu()
        installEventMonitor()

        let controller = ImmersiveApeWindowController(presentationMode: .fullScreenPrimary)
        windowController = controller
        controller.showWindow(self)

        NSApp.activate(ignoringOtherApps: true)
    }

    func applicationDidBecomeActive(_ notification: Notification) {
        immersiveApeDebugLog("applicationDidBecomeActive")
        ensurePrimaryWindowVisible()
    }

    func applicationShouldHandleReopen(_ sender: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        if !flag {
            ensurePrimaryWindowVisible()
        }
        return true
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        false
    }

    func applicationWillTerminate(_ notification: Notification) {
        immersiveApeDebugLog("applicationWillTerminate")
        if let eventMonitor {
            NSEvent.removeMonitor(eventMonitor)
            self.eventMonitor = nil
        }
        windowController?.shutdown()
        legacyShared?.close()
    }

    func add(shared: SimulationShared) {
        legacyShared = shared
    }

    @objc private func showAboutPanel(_ sender: Any?) {
        NSApp.orderFrontStandardAboutPanel(options: [
            .applicationName: appName,
            .version: "Cycle \(immersiveApeCurrentDevelopmentCycle) \(immersiveApeCurrentDevelopmentCycleTitle)",
            .credits: NSAttributedString(string: immersiveApeCurrentDevelopmentCycleSummary)
        ])
    }

    @objc private func selectPreviousApe(_ sender: Any?) {
        windowController?.immersiveViewController.selectPreviousApe()
    }

    @objc private func selectNextApe(_ sender: Any?) {
        windowController?.immersiveViewController.selectNextApe()
    }

    @objc private func togglePause(_ sender: Any?) {
        windowController?.immersiveViewController.togglePause()
    }

    @objc private func selectPreviousFocusHistory(_ sender: Any?) {
        windowController?.immersiveViewController.selectPreviousFocusHistory()
    }

    @objc private func selectNextFocusHistory(_ sender: Any?) {
        windowController?.immersiveViewController.selectNextFocusHistory()
    }

    @objc private func recallFocusAnchor(_ sender: Any?) {
        let slot = (sender as? NSMenuItem)?.tag ?? 0
        windowController?.immersiveViewController.recallFocusAnchor(slot: slot)
    }

    @objc private func saveFocusAnchor(_ sender: Any?) {
        let slot = (sender as? NSMenuItem)?.tag ?? 0
        windowController?.immersiveViewController.saveFocusAnchor(slot: slot)
    }

    @objc private func selectPreviousCameraMode(_ sender: Any?) {
        windowController?.immersiveViewController.selectPreviousCameraMode()
    }

    @objc private func selectNextCameraMode(_ sender: Any?) {
        windowController?.immersiveViewController.selectNextCameraMode()
    }

    @objc private func selectPreviousPerformanceBudget(_ sender: Any?) {
        windowController?.immersiveViewController.selectPreviousPerformanceBudget()
    }

    @objc private func selectNextPerformanceBudget(_ sender: Any?) {
        windowController?.immersiveViewController.selectNextPerformanceBudget()
    }

    @objc private func announceAccessibilitySummary(_ sender: Any?) {
        windowController?.immersiveViewController.announceAccessibilitySummary()
    }

    @objc private func toggleInteractionGuide(_ sender: Any?) {
        windowController?.immersiveViewController.toggleInteractionGuide()
    }

    @objc private func toggleFloatingWindowMode(_ sender: Any?) {
        windowController?.togglePresentationMode()
    }

    @objc private func toggleAudioAmbience(_ sender: Any?) {
        windowController?.immersiveViewController.toggleAudioAmbience()
    }

    private func makeMainMenu() -> NSMenu {
        let mainMenu = NSMenu(title: "MainMenu")

        let appMenuItem = NSMenuItem(title: appName, action: nil, keyEquivalent: "")
        let appMenu = NSMenu(title: appName)

        let aboutItem = NSMenuItem(title: "About \(appName)", action: #selector(showAboutPanel(_:)), keyEquivalent: "")
        aboutItem.target = self
        appMenu.addItem(aboutItem)
        appMenu.addItem(.separator())

        let servicesItem = NSMenuItem(title: "Services", action: nil, keyEquivalent: "")
        let servicesMenu = NSMenu(title: "Services")
        servicesItem.submenu = servicesMenu
        appMenu.addItem(servicesItem)
        NSApp.servicesMenu = servicesMenu

        appMenu.addItem(.separator())

        let hideItem = NSMenuItem(title: "Hide \(appName)", action: #selector(NSApplication.hide(_:)), keyEquivalent: "h")
        hideItem.target = NSApp
        appMenu.addItem(hideItem)

        let hideOthersItem = NSMenuItem(title: "Hide Others", action: #selector(NSApplication.hideOtherApplications(_:)), keyEquivalent: "h")
        hideOthersItem.keyEquivalentModifierMask = [.command, .option]
        hideOthersItem.target = NSApp
        appMenu.addItem(hideOthersItem)

        let showAllItem = NSMenuItem(title: "Show All", action: #selector(NSApplication.unhideAllApplications(_:)), keyEquivalent: "")
        showAllItem.target = NSApp
        appMenu.addItem(showAllItem)

        appMenu.addItem(.separator())

        let quitItem = NSMenuItem(title: "Quit \(appName)", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q")
        quitItem.target = NSApp
        appMenu.addItem(quitItem)

        appMenuItem.submenu = appMenu
        mainMenu.addItem(appMenuItem)

        let controlMenuItem = NSMenuItem(title: "Control", action: nil, keyEquivalent: "")
        let controlMenu = NSMenu(title: "Control")

        let pauseItem = NSMenuItem(title: "Pause Simulation", action: #selector(togglePause(_:)), keyEquivalent: " ")
        pauseItem.target = self
        controlMenu.addItem(pauseItem)

        let previousItem = NSMenuItem(title: "Previous Ape", action: #selector(selectPreviousApe(_:)), keyEquivalent: "[")
        previousItem.target = self
        controlMenu.addItem(previousItem)

        let nextItem = NSMenuItem(title: "Next Ape", action: #selector(selectNextApe(_:)), keyEquivalent: "]")
        nextItem.target = self
        controlMenu.addItem(nextItem)

        let previousFocusHistoryItem = NSMenuItem(title: "History Back", action: #selector(selectPreviousFocusHistory(_:)), keyEquivalent: "-")
        previousFocusHistoryItem.target = self
        previousFocusHistoryItem.keyEquivalentModifierMask = []
        controlMenu.addItem(previousFocusHistoryItem)

        let nextFocusHistoryItem = NSMenuItem(title: "History Forward", action: #selector(selectNextFocusHistory(_:)), keyEquivalent: "=")
        nextFocusHistoryItem.target = self
        nextFocusHistoryItem.keyEquivalentModifierMask = []
        controlMenu.addItem(nextFocusHistoryItem)

        controlMenu.addItem(.separator())

        for slot in 0..<4 {
            let recallItem = NSMenuItem(
                title: "Recall Anchor \(slot + 1)",
                action: #selector(recallFocusAnchor(_:)),
                keyEquivalent: "\(slot + 5)"
            )
            recallItem.target = self
            recallItem.keyEquivalentModifierMask = []
            recallItem.tag = slot
            controlMenu.addItem(recallItem)
        }

        for slot in 0..<4 {
            let saveItem = NSMenuItem(
                title: "Save Anchor \(slot + 1)",
                action: #selector(saveFocusAnchor(_:)),
                keyEquivalent: "\(slot + 5)"
            )
            saveItem.target = self
            saveItem.keyEquivalentModifierMask = [.shift]
            saveItem.tag = slot
            controlMenu.addItem(saveItem)
        }

        controlMenu.addItem(.separator())

        let previousCameraItem = NSMenuItem(title: "Previous Camera Mode", action: #selector(selectPreviousCameraMode(_:)), keyEquivalent: ",")
        previousCameraItem.target = self
        controlMenu.addItem(previousCameraItem)

        let nextCameraItem = NSMenuItem(title: "Next Camera Mode", action: #selector(selectNextCameraMode(_:)), keyEquivalent: ".")
        nextCameraItem.target = self
        controlMenu.addItem(nextCameraItem)

        let previousPerformanceBudgetItem = NSMenuItem(title: "Previous Performance Budget", action: #selector(selectPreviousPerformanceBudget(_:)), keyEquivalent: ";")
        previousPerformanceBudgetItem.keyEquivalentModifierMask = []
        previousPerformanceBudgetItem.target = self
        controlMenu.addItem(previousPerformanceBudgetItem)

        let nextPerformanceBudgetItem = NSMenuItem(title: "Next Performance Budget", action: #selector(selectNextPerformanceBudget(_:)), keyEquivalent: "'")
        nextPerformanceBudgetItem.keyEquivalentModifierMask = []
        nextPerformanceBudgetItem.target = self
        controlMenu.addItem(nextPerformanceBudgetItem)

        controlMenu.addItem(.separator())

        let accessibilityItem = NSMenuItem(title: "Announce Scene Summary", action: #selector(announceAccessibilitySummary(_:)), keyEquivalent: "a")
        accessibilityItem.target = self
        controlMenu.addItem(accessibilityItem)

        let guideItem = NSMenuItem(title: "Toggle Interaction Guide", action: #selector(toggleInteractionGuide(_:)), keyEquivalent: "/")
        guideItem.keyEquivalentModifierMask = []
        guideItem.target = self
        controlMenu.addItem(guideItem)

        let audioItem = NSMenuItem(title: "Toggle Ambience Audio", action: #selector(toggleAudioAmbience(_:)), keyEquivalent: "m")
        audioItem.keyEquivalentModifierMask = []
        audioItem.target = self
        controlMenu.addItem(audioItem)

        controlMenuItem.submenu = controlMenu
        mainMenu.addItem(controlMenuItem)

        let viewMenuItem = NSMenuItem(title: "View", action: nil, keyEquivalent: "")
        let viewMenu = NSMenu(title: "View")
        let floatingModeItem = NSMenuItem(title: "Toggle Floating Window Mode", action: #selector(toggleFloatingWindowMode(_:)), keyEquivalent: "w")
        floatingModeItem.keyEquivalentModifierMask = []
        floatingModeItem.target = self
        viewMenu.addItem(floatingModeItem)

        let fullScreenItem = NSMenuItem(title: "Toggle Full Screen", action: #selector(NSWindow.toggleFullScreen(_:)), keyEquivalent: "f")
        fullScreenItem.keyEquivalentModifierMask = [.control, .command]
        viewMenu.addItem(fullScreenItem)
        viewMenuItem.submenu = viewMenu
        mainMenu.addItem(viewMenuItem)

        return mainMenu
    }

    private func ensurePrimaryWindowVisible() {
        guard let windowController else {
            return
        }

        if windowController.window?.isVisible != true {
            windowController.showWindow(self)
        } else {
            windowController.bringToFront()
        }
    }

    private func installEventMonitor() {
        eventMonitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            guard let self else {
                return event
            }

            if event.keyCode == 53 {
                NSApp.terminate(nil)
                return nil
            }

            guard let characters = event.charactersIgnoringModifiers else {
                return event
            }

            let usesShift = event.modifierFlags.intersection(.deviceIndependentFlagsMask).contains(.shift)
            if let slot = ["5": 0, "6": 1, "7": 2, "8": 3][characters] {
                if usesShift {
                    self.windowController?.immersiveViewController.saveFocusAnchor(slot: slot)
                } else {
                    self.windowController?.immersiveViewController.recallFocusAnchor(slot: slot)
                }
                return nil
            }

            switch characters {
            case "[":
                self.windowController?.immersiveViewController.selectPreviousApe()
                return nil
            case "]":
                self.windowController?.immersiveViewController.selectNextApe()
                return nil
            case "1":
                self.windowController?.immersiveViewController.selectSelectionCandidate(slot: 0)
                return nil
            case "2":
                self.windowController?.immersiveViewController.selectSelectionCandidate(slot: 1)
                return nil
            case "3":
                self.windowController?.immersiveViewController.selectSelectionCandidate(slot: 2)
                return nil
            case "4":
                self.windowController?.immersiveViewController.selectSelectionCandidate(slot: 3)
                return nil
            case "-":
                self.windowController?.immersiveViewController.selectPreviousFocusHistory()
                return nil
            case "=", "+":
                self.windowController?.immersiveViewController.selectNextFocusHistory()
                return nil
            case ",":
                self.windowController?.immersiveViewController.selectPreviousCameraMode()
                return nil
            case ".":
                self.windowController?.immersiveViewController.selectNextCameraMode()
                return nil
            case ";":
                self.windowController?.immersiveViewController.selectPreviousPerformanceBudget()
                return nil
            case "'":
                self.windowController?.immersiveViewController.selectNextPerformanceBudget()
                return nil
            case "a", "A":
                self.windowController?.immersiveViewController.announceAccessibilitySummary()
                return nil
            case "/", "?":
                self.windowController?.immersiveViewController.toggleInteractionGuide()
                return nil
            case "m", "M":
                self.windowController?.immersiveViewController.toggleAudioAmbience()
                return nil
            case "w", "W":
                self.windowController?.togglePresentationMode()
                return nil
            case " ":
                self.windowController?.immersiveViewController.togglePause()
                return nil
            default:
                return event
            }
        }
    }
}

enum ImmersiveApePresentationMode {
    case fullScreenPrimary
    case floatingWindow

    var label: String {
        switch self {
        case .fullScreenPrimary:
            return "Full Screen Primary"
        case .floatingWindow:
            return "Floating Window"
        }
    }

    var hudText: String {
        switch self {
        case .fullScreenPrimary:
            return """
                Mode 1 / 2  •  Full Screen Primary
                Fills the main display and keeps the immersive HUD spread wide across the screen.
                Key: W toggle floating window
                """
        case .floatingWindow:
            return """
                Mode 2 / 2  •  Floating Window
                Uses a centered resizable window so the simulation can share space with other Mac apps.
                Key: W toggle full-screen primary
                """
        }
    }

    var accessibilitySentence: String {
        switch self {
        case .fullScreenPrimary:
            return "Window mode full screen primary."
        case .floatingWindow:
            return "Window mode floating window."
        }
    }

    var windowTitle: String {
        switch self {
        case .fullScreenPrimary:
            return "Immersive Ape"
        case .floatingWindow:
            return "Immersive Ape Floating Window"
        }
    }

    func toggled() -> ImmersiveApePresentationMode {
        switch self {
        case .fullScreenPrimary:
            return .floatingWindow
        case .floatingWindow:
            return .fullScreenPrimary
        }
    }
}

enum ImmersiveApePanelLayoutMode {
    case wideImmersive
    case sharedWindow

    var hudText: String {
        switch self {
        case .wideImmersive:
            return """
                Layout 1 / 2  •  Wide Immersive
                Keeps the HUD left, encounters right, and the story low and centered for the broad full-screen field.
                Auto: full-screen primary
                """
        case .sharedWindow:
            return """
                Layout 2 / 2  •  Shared Window
                Keeps the HUD left while encounter and story panels dock into a right-side column for desktop sharing.
                Auto: floating window
                """
        }
    }

    var accessibilitySentence: String {
        switch self {
        case .wideImmersive:
            return "Panel layout wide immersive field."
        case .sharedWindow:
            return "Panel layout shared window column."
        }
    }
}

final class ImmersiveApeWindow: NSWindow {
    override var canBecomeKey: Bool {
        true
    }

    override var canBecomeMain: Bool {
        true
    }

    override func cancelOperation(_ sender: Any?) {
        NSApp.terminate(sender)
    }
}

final class ImmersiveApeWindowController: NSWindowController, NSWindowDelegate {
    let immersiveViewController = ImmersiveApeViewController()
    private var presentationMode: ImmersiveApePresentationMode
    private var lastFloatingFrame: NSRect?

    init(presentationMode: ImmersiveApePresentationMode) {
        self.presentationMode = presentationMode

        let screenFrame = NSScreen.main?.frame ?? NSRect(x: 0, y: 0, width: 1600, height: 1000)
        let styleMask: NSWindow.StyleMask = [.titled, .closable, .miniaturizable, .resizable, .fullSizeContentView]

        let window = ImmersiveApeWindow(
            contentRect: screenFrame,
            styleMask: styleMask,
            backing: .buffered,
            defer: false
        )

        window.title = "Immersive Ape"
        window.isReleasedWhenClosed = false
        window.titleVisibility = .hidden
        window.titlebarAppearsTransparent = true
        window.backgroundColor = .black
        window.isOpaque = true
        window.hasShadow = true
        window.level = .normal
        window.collectionBehavior = [.managed, .fullScreenPrimary]
        window.contentViewController = immersiveViewController

        super.init(window: window)
        window.delegate = self
        applyPresentation()
    }

    @available(*, unavailable)
    required init?(coder: NSCoder) {
        nil
    }

    override func showWindow(_ sender: Any?) {
        super.showWindow(sender)

        guard let window else {
            return
        }

        immersiveApeDebugLog("showWindow visible=\(window.isVisible) frame=\(NSStringFromRect(window.frame))")
        applyPresentation()
        NSApp.activate(ignoringOtherApps: true)
        window.orderFrontRegardless()
        window.makeKeyAndOrderFront(sender)
        window.makeMain()

        DispatchQueue.main.async { [weak self, weak window] in
            guard let self, let window else {
                return
            }

            self.immersiveViewController.focusRenderer()
            window.makeKeyAndOrderFront(sender)
        }
    }

    func shutdown() {
        immersiveApeDebugLog("shutdown")
        immersiveViewController.shutdown()
    }

    func togglePresentationMode() {
        setPresentationMode(presentationMode.toggled())
    }

    func windowWillClose(_ notification: Notification) {
        immersiveApeDebugLog("windowWillClose")
        NSApp.terminate(nil)
    }

    func windowDidMove(_ notification: Notification) {
        cacheFloatingFrameIfNeeded()
    }

    func windowDidResize(_ notification: Notification) {
        cacheFloatingFrameIfNeeded()
    }

    func bringToFront() {
        guard let window else {
            return
        }
        applyPresentation()
        NSApp.activate(ignoringOtherApps: true)
        window.orderFrontRegardless()
        window.makeKeyAndOrderFront(nil)
        window.makeMain()
        immersiveViewController.focusRenderer()
    }

    private func setPresentationMode(_ mode: ImmersiveApePresentationMode) {
        guard presentationMode != mode else {
            return
        }

        if presentationMode == .floatingWindow, let window {
            lastFloatingFrame = window.frame
        }

        presentationMode = mode
        applyPresentation()
        NSApp.activate(ignoringOtherApps: true)
        window?.orderFrontRegardless()
        window?.makeKeyAndOrderFront(nil)
        immersiveViewController.focusRenderer()
    }

    private func cacheFloatingFrameIfNeeded() {
        guard presentationMode == .floatingWindow, let window else {
            return
        }

        lastFloatingFrame = window.frame
    }

    private func defaultFloatingFrame(for screen: NSScreen?) -> NSRect {
        let visibleFrame = (screen ?? NSScreen.main)?.visibleFrame ?? NSRect(x: 0, y: 0, width: 1600, height: 1000)
        let width = min(max(visibleFrame.width - 120, 1100), 1440)
        let height = min(max(visibleFrame.height - 140, 720), 920)
        let origin = NSPoint(
            x: visibleFrame.midX - (width * 0.5),
            y: visibleFrame.midY - (height * 0.5)
        )
        return NSRect(origin: origin, size: NSSize(width: width, height: height)).integral
    }

    private func applyPresentation() {
        guard let window else {
            return
        }

        switch presentationMode {
        case .fullScreenPrimary:
            window.title = presentationMode.windowTitle
            window.collectionBehavior = [.managed, .fullScreenPrimary]
            window.titleVisibility = .hidden
            window.titlebarAppearsTransparent = true
            window.isMovableByWindowBackground = false
            if let screen = window.screen ?? NSScreen.main {
                window.setFrame(screen.frame, display: true, animate: true)
            }
            window.minSize = NSSize(width: 960, height: 640)
        case .floatingWindow:
            window.title = presentationMode.windowTitle
            window.collectionBehavior = [.managed]
            window.titleVisibility = .visible
            window.titlebarAppearsTransparent = false
            window.isMovableByWindowBackground = false
            window.minSize = NSSize(width: 960, height: 640)

            let frame = lastFloatingFrame ?? defaultFloatingFrame(for: window.screen ?? NSScreen.main)
            window.setFrame(frame, display: true, animate: true)
        }

        immersiveViewController.setPresentationMode(presentationMode)
    }
}

final class ImmersiveApeMetalView: MTKView {
    var keyDownHandler: ((NSEvent) -> Void)?

    override var acceptsFirstResponder: Bool {
        true
    }

    override func keyDown(with event: NSEvent) {
        if let keyDownHandler {
            keyDownHandler(event)
        } else {
            super.keyDown(with: event)
        }
    }
}

private final class ImmersiveApeAudioController: @unchecked Sendable {
    private let engine = AVAudioEngine()
    private let playerNode = AVAudioPlayerNode()
    private let renderQueue = DispatchQueue(label: "com.apesdk.immersiveape.audio")
    private let format = AVAudioFormat(standardFormatWithSampleRate: 44_100, channels: 2)!
    private let bufferFrameCount: AVAudioFrameCount = 4096
    private let queuedBufferTarget = 3
    private var queuedBuffers = 0
    private var hasAttachedGraph = false
    private var shuttingDown = false
    private var muted = false
    private var latestState = ImmersiveApeAudioState.silent

    private var randomState: UInt64 = 0x5deece66d
    private var surfNoise: Float = 0
    private var windNoise: Float = 0
    private var rainNoise: Float = 0
    private var surfMotionPhase: Float = 0
    private var surfTonePhase: Float = 0
    private var surfPanPhase: Float = 0
    private var windMotionPhase: Float = 0
    private var windPanPhase: Float = 0
    private var duskPhaseA: Float = 0
    private var duskPhaseB: Float = 0
    private var nightPhaseA: Float = 0
    private var nightPhaseB: Float = 0
    private var socialGatePhase: Float = 0
    private var socialVoicePhase: Float = 0
    private var socialPanPhase: Float = 0

    private var sampleRate: Float {
        Float(format.sampleRate)
    }

    func update(_ state: ImmersiveApeAudioState, muted: Bool) {
        renderQueue.async { [weak self] in
            guard let self else {
                return
            }

            self.latestState = state
            self.muted = muted
            self.ensureRunning()
            self.fillBufferQueueIfNeeded()
        }
    }

    func shutdown() {
        renderQueue.async { [weak self] in
            guard let self else {
                return
            }

            self.shuttingDown = true
            self.queuedBuffers = 0
            self.playerNode.stop()
            self.engine.stop()
        }
    }

    private func ensureRunning() {
        guard shuttingDown == false else {
            return
        }

        if hasAttachedGraph == false {
            engine.attach(playerNode)
            engine.connect(playerNode, to: engine.mainMixerNode, format: format)
            engine.mainMixerNode.outputVolume = 0.3
            engine.prepare()
            hasAttachedGraph = true
        }

        if engine.isRunning == false {
            do {
                try engine.start()
            } catch {
                immersiveApeDebugLog("audio engine failed: \(error.localizedDescription)")
                return
            }
        }

        if playerNode.isPlaying == false {
            playerNode.play()
        }
    }

    private func fillBufferQueueIfNeeded() {
        guard shuttingDown == false, engine.isRunning else {
            return
        }

        while queuedBuffers < queuedBufferTarget {
            guard let buffer = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: bufferFrameCount) else {
                return
            }

            buffer.frameLength = bufferFrameCount
            render(buffer: buffer, state: muted ? .silent : latestState)
            queuedBuffers += 1

            playerNode.scheduleBuffer(buffer) { [weak self] in
                guard let self else {
                    return
                }

                self.renderQueue.async { [weak self] in
                    guard let self else {
                        return
                    }

                    guard self.shuttingDown == false else {
                        return
                    }

                    self.queuedBuffers = max(0, self.queuedBuffers - 1)
                    self.fillBufferQueueIfNeeded()
                }
            }
        }

        if playerNode.isPlaying == false {
            playerNode.play()
        }
    }

    private func render(buffer: AVAudioPCMBuffer, state: ImmersiveApeAudioState) {
        guard let channels = buffer.floatChannelData else {
            return
        }

        let channelCount = Int(format.channelCount)
        let leftChannel = channels[0]
        let rightChannel = channels[channelCount > 1 ? 1 : 0]
        let frameCount = Int(buffer.frameLength)
        let combinedLevel = state.surfLevel + state.windLevel + state.rainLevel + state.duskLevel + state.nightLevel + state.socialLevel + state.vocalLevel

        guard combinedLevel > 0.001 else {
            for frame in 0..<frameCount {
                leftChannel[frame] = 0
                rightChannel[frame] = 0
            }
            return
        }

        for frame in 0..<frameCount {
            let surfMotion = 0.5 + (0.5 * advancePhase(&surfMotionPhase, frequency: 0.07 + (state.surfLevel * 0.06)))
            let surfPan = advancePhase(&surfPanPhase, frequency: 0.03) * 0.12
            surfNoise = (surfNoise * 0.992) + (nextNoise() * 0.008)
            let surfTone = advancePhase(&surfTonePhase, frequency: 42 + (state.surfLevel * 18))
            let surfSample = ((surfNoise * (0.26 + (surfMotion * 0.16))) + (surfTone * 0.05)) * (0.03 + (state.surfLevel * 0.16))

            let windMotion = 0.5 + (0.5 * advancePhase(&windMotionPhase, frequency: 0.05 + (state.windLevel * 0.03)))
            let windPan = advancePhase(&windPanPhase, frequency: 0.025) * 0.28
            windNoise = (windNoise * 0.97) + (nextNoise() * 0.03)
            let windSample = windNoise * (0.015 + (state.windLevel * (0.08 + (windMotion * 0.06))))

            rainNoise = (rainNoise * 0.12) + (((nextNoise() * 0.7) + (nextNoise() * 0.3)) * 0.88)
            let rainSample = rainNoise * (0.01 + (state.rainLevel * 0.09))

            let duskSample = (
                advancePhase(&duskPhaseA, frequency: 196)
                    + (0.45 * advancePhase(&duskPhaseB, frequency: 294))
            ) * (0.008 + (state.duskLevel * 0.014))

            let nightSample = (
                advancePhase(&nightPhaseA, frequency: 110)
                    + (0.5 * advancePhase(&nightPhaseB, frequency: 164))
            ) * (0.008 + (state.nightLevel * 0.014))

            let socialGate = powf(max(0, advancePhase(&socialGatePhase, frequency: 0.34 + (state.socialLevel * 0.22))), 2.6)
            let socialPan = advancePhase(&socialPanPhase, frequency: 0.18) * 0.24
            let socialVoice = advancePhase(&socialVoicePhase, frequency: 260 + (state.vocalLevel * 180))
            let vocalSample = socialVoice * socialGate * (0.002 + (state.socialLevel * 0.02) + (state.vocalLevel * 0.028))

            var leftSample: Float = 0
            var rightSample: Float = 0

            mix(sample: surfSample, pan: surfPan, left: &leftSample, right: &rightSample)
            mix(sample: windSample, pan: windPan, left: &leftSample, right: &rightSample)
            mix(sample: rainSample, pan: -windPan * 0.45, left: &leftSample, right: &rightSample)
            mix(sample: duskSample + nightSample, pan: 0, left: &leftSample, right: &rightSample)
            mix(sample: vocalSample, pan: socialPan, left: &leftSample, right: &rightSample)

            leftChannel[frame] = clamp(leftSample * 0.52)
            rightChannel[frame] = clamp(rightSample * 0.52)
        }
    }

    private func mix(sample: Float, pan: Float, left: inout Float, right: inout Float) {
        let clampedPan = max(-1, min(1, pan))
        let leftGain = 1 - max(0, clampedPan)
        let rightGain = 1 + min(0, clampedPan)
        left += sample * leftGain
        right += sample * rightGain
    }

    private func advancePhase(_ phase: inout Float, frequency: Float) -> Float {
        let fullTurn = Float.pi * 2
        phase += (fullTurn * frequency) / sampleRate
        if phase >= fullTurn {
            phase -= fullTurn
        }
        return sin(phase)
    }

    private func nextNoise() -> Float {
        randomState = randomState &* 2862933555777941757 &+ 3037000493
        let unit = Float(UInt32(truncatingIfNeeded: randomState >> 33)) / Float(UInt32.max)
        return (unit * 2) - 1
    }

    private func clamp(_ value: Float) -> Float {
        min(0.95, max(-0.95, value))
    }
}

final class ImmersiveApeViewController: NSViewController {
    private struct ImmersiveApeFocusHistoryEntry: Equatable {
        let apeIndex: Int
        let totalCount: Int
        let name: String
    }

    private let headlineLabel = NSTextField(labelWithString: "Immersive Ape")
    private let statusLabel = NSTextField(labelWithString: "Booting ApeSDK...")
    private let detailLabel = NSTextField(labelWithString: "Linking ape goals, weather, and surf context...")
    private let navigatorTitleLabel = NSTextField(labelWithString: "Focus Neighborhood")
    private let navigatorLabel = NSTextField(labelWithString: "Gathering nearby handoffs, remembered trail steps, and saved anchors...")
    private let selectionTitleLabel = NSTextField(labelWithString: "Selection Layer")
    private let selectionLabel = NSTextField(labelWithString: "Tracking previous, next, and nearby ape handoffs...")
    private let historyTitleLabel = NSTextField(labelWithString: "Focus History")
    private let historyLabel = NSTextField(labelWithString: "No tracked focus history yet. Use [ ], 1-4, then - or = to revisit recent selected apes.")
    private let anchorTitleLabel = NSTextField(labelWithString: "Focus Anchors")
    private let anchorLabel = NSTextField(labelWithString: "Saved 0 / 4  •  Recall 5-8  •  Save Shift+5-8\nNo saved focus anchors yet. Save the current ape into a direct slot.")
    private let cameraTitleLabel = NSTextField(labelWithString: "Camera")
    private let cameraLabel = NSTextField(labelWithString: "Embodied camera is active. Use , and . to cycle wider follow presets.")
    private let audioTitleLabel = NSTextField(labelWithString: "Ambience")
    private let audioLabel = NSTextField(labelWithString: "Procedural ambience is standing by for surf, wind, rain, and nearby ape voices...")
    private let presentationTitleLabel = NSTextField(labelWithString: "Window")
    private let presentationLabel = NSTextField(labelWithString: "Full-screen primary mode is active. Press W to toggle a floating window.")
    private let layoutTitleLabel = NSTextField(labelWithString: "Layout")
    private let layoutLabel = NSTextField(labelWithString: "Wide immersive layout is active. Switch to a floating window to dock encounter and story panels into a shared desktop column.")
    private let guideTitleLabel = NSTextField(labelWithString: "Guide")
    private let guideLabel = NSTextField(labelWithString: "Guide hidden. Press / to open the interaction guide for controls, budgets, ambience, and current shell state.")
    private let feedbackTitleLabel = NSTextField(labelWithString: "Feedback")
    private let feedbackLabel = NSTextField(labelWithString: "No recent control feedback. Use the keyboard or menu to drive the immersive shell.")
    private let performanceLabel = NSTextField(labelWithString: """
        Budget 2 / 3  •  Balanced
        FPS --  •  Target 60  •  Floor 30  •  Quality Balanced
        Adaptive range Balanced-Emergency  •  Mesh x1
        LOD Near 16m  •  Mid 40m  •  Horizon 72m
        Trade-offs Terrain steady  •  Surface selective  •  Sky only  •  Guide only
        Stability Cadence live  •  Cache not needed  •  Recovery settled
        Headroom Floor roomy  •  Recovery ready  •  Target on pace
        Sample Reading seeded  •  Window priming  •  Watch starting
        Gate Guard calm  •  Recover peaked  •  Cooldown clear
        Outlook Hold current quality  •  No shift queued  •  Current quality favored
        Confidence Locked  •  Rich bound already reached  •  No richer step left
        Threshold Leaner 30.0fps away  •  Richer bound reached  •  First sample pending
        Band Rich bound lane  •  Hold span 8.0fps  •  Best detail already active
        Window Rebuild each draw  •  Detail already peaked
        Recent No adaptive shift yet  •  Session Down 0  •  Up 0
        Margin Richer 0  •  Leaner 2  •  At rich bound
        """)
    private let storyLabel = NSTextField(labelWithString: "Following the selected ape through the procedural world...")
    private let encounterTitleLabel = NSTextField(labelWithString: "Intent + Encounters")
    private let encounterLabel = NSTextField(labelWithString: "Scanning nearby apes...")
    private let footerLabel = NSTextField(labelWithString: "Esc quit  [ ] cycle apes  1-4 pick nearby  - = history  5-8 anchors  Shift+5-8 save  , . camera  ; ' budget  M audio  / help  W window  A announce  Space pause")
    private let errorLabel = NSTextField(labelWithString: "")
    private let hudChrome = NSVisualEffectView()
    private let encounterChrome = NSVisualEffectView()
    private let storyChrome = NSVisualEffectView()
    private let helpChrome = NSVisualEffectView()
    private let feedbackChrome = NSVisualEffectView()
    private let helpTitleLabel = NSTextField(labelWithString: "Interaction Guide")
    private let helpBodyLabel = NSTextField(labelWithString: "")
    private let feedbackOverlayTitleLabel = NSTextField(labelWithString: "")
    private let feedbackOverlayDetailLabel = NSTextField(labelWithString: "")

    private var metalView: ImmersiveApeMetalView!
    private var renderer: ImmersiveApeRenderer?
    private var currentPresentationMode: ImmersiveApePresentationMode = .fullScreenPrimary
    private var currentPanelLayoutMode: ImmersiveApePanelLayoutMode = .wideImmersive
    private var helpOverlayVisible = false
    private let audioController = ImmersiveApeAudioController()
    private var latestSceneAccessibilitySummary = "Immersive Ape is starting up. Wait for a selected ape, then press A to hear the current scene summary."
    private var latestAudioState = ImmersiveApeAudioState.silent
    private var latestControlFeedbackAccessibility = "No recent control feedback."
    private var latestAccessibilitySummary = "Immersive Ape is starting up. Wait for a selected ape, then press A to hear the current scene summary. Audio standing by for the next selected ape. Performance budget balanced with a 60 frames per second target, spatial LOD rings focused near the selected ape, and adaptive trade-offs, frame headroom, sample window state, adaptive outlook, adaptive confidence, adaptive thresholds, adaptive band position, shift readiness, plus budget margins standing by for the next scene. Focus neighborhood waiting for a selected ape. Focus history unavailable. Focus anchors empty. Window mode full screen primary. Panel layout wide immersive field. Interaction guide closed. No recent control feedback."
    private var widePanelConstraints: [NSLayoutConstraint] = []
    private var sharedWindowConstraints: [NSLayoutConstraint] = []
    private var feedbackHideTimer: Timer?
    private var feedbackDisplayToken = 0
    private var audioMuted = false
    private var focusHistory: [ImmersiveApeFocusHistoryEntry] = []
    private var focusHistoryPosition: Int?
    private var focusAnchors: [ImmersiveApeFocusHistoryEntry?] = Array(repeating: nil, count: 4)
    private var currentSelectionSnapshot: ImmersiveApeFocusHistoryEntry?
    private var lastSelectionFeedbackLine: String?
    private var lastCameraFeedbackLine: String?
    private var lastPerformanceFeedbackLine: String?
    private var lastPerformanceStabilityFeedbackLine: String?
    private var lastPerformanceHeadroomFeedbackLine: String?
    private var lastPerformanceGateFeedbackLine: String?
    private var lastPerformanceOutlookFeedbackLine: String?
    private var lastPerformanceConfidenceFeedbackLine: String?
    private var lastPerformanceRecentFeedbackLine: String?
    private var lastPerformanceMarginFeedbackLine: String?
    private var lastPausedState: Bool?

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    override func loadView() {
        let rootView = NSView()
        rootView.wantsLayer = true
        rootView.layer?.backgroundColor = NSColor.black.cgColor

        let metalView = ImmersiveApeMetalView(frame: .zero, device: MTLCreateSystemDefaultDevice())
        metalView.translatesAutoresizingMaskIntoConstraints = false
        metalView.clearColor = MTLClearColor(red: 0.02, green: 0.03, blue: 0.05, alpha: 1.0)
        rootView.addSubview(metalView)
        self.metalView = metalView

        hudChrome.translatesAutoresizingMaskIntoConstraints = false
        hudChrome.material = .hudWindow
        hudChrome.blendingMode = .withinWindow
        hudChrome.state = .active
        hudChrome.wantsLayer = true
        hudChrome.layer?.cornerRadius = 18
        rootView.addSubview(hudChrome)

        let stackView = NSStackView()
        stackView.translatesAutoresizingMaskIntoConstraints = false
        stackView.orientation = .vertical
        stackView.spacing = 6
        hudChrome.addSubview(stackView)

        encounterChrome.translatesAutoresizingMaskIntoConstraints = false
        encounterChrome.material = .hudWindow
        encounterChrome.blendingMode = .withinWindow
        encounterChrome.state = .active
        encounterChrome.wantsLayer = true
        encounterChrome.layer?.cornerRadius = 18
        rootView.addSubview(encounterChrome)

        let encounterStack = NSStackView()
        encounterStack.translatesAutoresizingMaskIntoConstraints = false
        encounterStack.orientation = .vertical
        encounterStack.spacing = 6
        encounterChrome.addSubview(encounterStack)

        storyChrome.translatesAutoresizingMaskIntoConstraints = false
        storyChrome.material = .hudWindow
        storyChrome.blendingMode = .withinWindow
        storyChrome.state = .active
        storyChrome.wantsLayer = true
        storyChrome.layer?.cornerRadius = 18
        rootView.addSubview(storyChrome)

        let storyStack = NSStackView()
        storyStack.translatesAutoresizingMaskIntoConstraints = false
        storyStack.orientation = .vertical
        storyStack.spacing = 4
        storyChrome.addSubview(storyStack)

        feedbackChrome.translatesAutoresizingMaskIntoConstraints = false
        feedbackChrome.material = .hudWindow
        feedbackChrome.blendingMode = .withinWindow
        feedbackChrome.state = .active
        feedbackChrome.wantsLayer = true
        feedbackChrome.layer?.cornerRadius = 18
        feedbackChrome.alphaValue = 0
        feedbackChrome.isHidden = true
        rootView.addSubview(feedbackChrome)

        let feedbackStack = NSStackView()
        feedbackStack.translatesAutoresizingMaskIntoConstraints = false
        feedbackStack.orientation = .vertical
        feedbackStack.spacing = 4
        feedbackChrome.addSubview(feedbackStack)

        helpChrome.translatesAutoresizingMaskIntoConstraints = false
        helpChrome.material = .hudWindow
        helpChrome.blendingMode = .withinWindow
        helpChrome.state = .active
        helpChrome.wantsLayer = true
        helpChrome.layer?.cornerRadius = 20
        helpChrome.isHidden = true
        rootView.addSubview(helpChrome)

        let helpStack = NSStackView()
        helpStack.translatesAutoresizingMaskIntoConstraints = false
        helpStack.orientation = .vertical
        helpStack.spacing = 8
        helpChrome.addSubview(helpStack)

        helpTitleLabel.font = .systemFont(ofSize: 21, weight: .bold)
        helpTitleLabel.textColor = .white

        helpBodyLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        helpBodyLabel.textColor = NSColor.white.withAlphaComponent(0.9)
        helpBodyLabel.lineBreakMode = .byWordWrapping
        helpBodyLabel.cell?.wraps = true
        helpBodyLabel.cell?.usesSingleLineMode = false

        helpStack.addArrangedSubview(helpTitleLabel)
        helpStack.addArrangedSubview(helpBodyLabel)

        headlineLabel.font = .systemFont(ofSize: 24, weight: .bold)
        headlineLabel.textColor = .white
        statusLabel.font = .systemFont(ofSize: 13, weight: .medium)
        statusLabel.textColor = NSColor.white.withAlphaComponent(0.9)
        detailLabel.font = .systemFont(ofSize: 12, weight: .regular)
        detailLabel.textColor = NSColor.white.withAlphaComponent(0.82)
        navigatorTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        navigatorTitleLabel.textColor = .white
        navigatorLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        navigatorLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        selectionTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        selectionTitleLabel.textColor = .white
        selectionLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        selectionLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        historyTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        historyTitleLabel.textColor = .white
        historyLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        historyLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        anchorTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        anchorTitleLabel.textColor = .white
        anchorLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        anchorLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        cameraTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        cameraTitleLabel.textColor = .white
        cameraLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        cameraLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        audioTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        audioTitleLabel.textColor = .white
        audioLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        audioLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        presentationTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        presentationTitleLabel.textColor = .white
        presentationLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        presentationLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        layoutTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        layoutTitleLabel.textColor = .white
        layoutLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        layoutLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        guideTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        guideTitleLabel.textColor = .white
        guideLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        guideLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        feedbackTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        feedbackTitleLabel.textColor = .white
        feedbackLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        feedbackLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        performanceLabel.font = .monospacedSystemFont(ofSize: 12, weight: .semibold)
        performanceLabel.textColor = NSColor.white.withAlphaComponent(0.88)
        storyLabel.font = .systemFont(ofSize: 13, weight: .medium)
        storyLabel.textColor = NSColor.white.withAlphaComponent(0.9)
        storyLabel.alignment = .center
        encounterTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        encounterTitleLabel.textColor = .white
        encounterLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        encounterLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        footerLabel.font = .monospacedSystemFont(ofSize: 11, weight: .medium)
        footerLabel.textColor = NSColor.white.withAlphaComponent(0.75)
        feedbackOverlayTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        feedbackOverlayTitleLabel.textColor = .white
        feedbackOverlayDetailLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        feedbackOverlayDetailLabel.textColor = NSColor.white.withAlphaComponent(0.9)

        for label in [statusLabel, detailLabel, navigatorLabel, selectionLabel, historyLabel, anchorLabel, cameraLabel, audioLabel, presentationLabel, layoutLabel, guideLabel, feedbackLabel, performanceLabel, storyLabel, encounterLabel, footerLabel, feedbackOverlayDetailLabel] {
            label.lineBreakMode = .byWordWrapping
            label.cell?.wraps = true
            label.cell?.usesSingleLineMode = false
        }

        stackView.addArrangedSubview(headlineLabel)
        stackView.addArrangedSubview(statusLabel)
        stackView.addArrangedSubview(detailLabel)
        stackView.addArrangedSubview(navigatorTitleLabel)
        stackView.addArrangedSubview(navigatorLabel)
        stackView.addArrangedSubview(selectionTitleLabel)
        stackView.addArrangedSubview(selectionLabel)
        stackView.addArrangedSubview(historyTitleLabel)
        stackView.addArrangedSubview(historyLabel)
        stackView.addArrangedSubview(anchorTitleLabel)
        stackView.addArrangedSubview(anchorLabel)
        stackView.addArrangedSubview(cameraTitleLabel)
        stackView.addArrangedSubview(cameraLabel)
        stackView.addArrangedSubview(audioTitleLabel)
        stackView.addArrangedSubview(audioLabel)
        stackView.addArrangedSubview(presentationTitleLabel)
        stackView.addArrangedSubview(presentationLabel)
        stackView.addArrangedSubview(layoutTitleLabel)
        stackView.addArrangedSubview(layoutLabel)
        stackView.addArrangedSubview(guideTitleLabel)
        stackView.addArrangedSubview(guideLabel)
        stackView.addArrangedSubview(feedbackTitleLabel)
        stackView.addArrangedSubview(feedbackLabel)
        stackView.addArrangedSubview(performanceLabel)
        stackView.addArrangedSubview(footerLabel)

        encounterStack.addArrangedSubview(encounterTitleLabel)
        encounterStack.addArrangedSubview(encounterLabel)

        storyStack.addArrangedSubview(storyLabel)
        feedbackStack.addArrangedSubview(feedbackOverlayTitleLabel)
        feedbackStack.addArrangedSubview(feedbackOverlayDetailLabel)

        errorLabel.translatesAutoresizingMaskIntoConstraints = false
        errorLabel.font = .systemFont(ofSize: 18, weight: .semibold)
        errorLabel.textColor = .white
        errorLabel.alignment = .center
        errorLabel.isHidden = true
        rootView.addSubview(errorLabel)

        refreshPresentationHUD()
        refreshLayoutHUD()
        refreshGuideHUD()
        refreshAudioHUD()
        setPerformanceHUD(performanceLabel.stringValue)
        updateHistoryHUD()
        updateAnchorHUD()
        updateNavigatorHUD()
        helpBodyLabel.stringValue = interactionGuideText()
        configureAccessibilityElements()
        applyAccessibilityDisplayPreferences()

        widePanelConstraints = [
            hudChrome.leadingAnchor.constraint(equalTo: rootView.leadingAnchor, constant: 24),
            hudChrome.topAnchor.constraint(equalTo: rootView.safeAreaLayoutGuide.topAnchor, constant: 24),
            hudChrome.widthAnchor.constraint(lessThanOrEqualToConstant: 520),

            encounterChrome.topAnchor.constraint(equalTo: rootView.safeAreaLayoutGuide.topAnchor, constant: 24),
            encounterChrome.trailingAnchor.constraint(equalTo: rootView.trailingAnchor, constant: -24),
            encounterChrome.widthAnchor.constraint(lessThanOrEqualToConstant: 360),
            encounterChrome.leadingAnchor.constraint(greaterThanOrEqualTo: hudChrome.trailingAnchor, constant: 24),

            storyChrome.centerXAnchor.constraint(equalTo: rootView.centerXAnchor),
            storyChrome.bottomAnchor.constraint(equalTo: rootView.safeAreaLayoutGuide.bottomAnchor, constant: -24),
            storyChrome.widthAnchor.constraint(lessThanOrEqualToConstant: 760),
            storyChrome.leadingAnchor.constraint(greaterThanOrEqualTo: rootView.leadingAnchor, constant: 24),
            storyChrome.trailingAnchor.constraint(lessThanOrEqualTo: rootView.trailingAnchor, constant: -24)
        ]

        sharedWindowConstraints = [
            hudChrome.leadingAnchor.constraint(equalTo: rootView.leadingAnchor, constant: 24),
            hudChrome.topAnchor.constraint(equalTo: rootView.safeAreaLayoutGuide.topAnchor, constant: 24),
            hudChrome.widthAnchor.constraint(lessThanOrEqualToConstant: 440),

            encounterChrome.topAnchor.constraint(equalTo: rootView.safeAreaLayoutGuide.topAnchor, constant: 24),
            encounterChrome.trailingAnchor.constraint(equalTo: rootView.trailingAnchor, constant: -24),
            encounterChrome.widthAnchor.constraint(lessThanOrEqualToConstant: 320),
            encounterChrome.leadingAnchor.constraint(greaterThanOrEqualTo: hudChrome.trailingAnchor, constant: 16),

            storyChrome.topAnchor.constraint(equalTo: encounterChrome.bottomAnchor, constant: 20),
            storyChrome.leadingAnchor.constraint(equalTo: encounterChrome.leadingAnchor),
            storyChrome.trailingAnchor.constraint(equalTo: encounterChrome.trailingAnchor),
            storyChrome.bottomAnchor.constraint(lessThanOrEqualTo: rootView.safeAreaLayoutGuide.bottomAnchor, constant: -24)
        ]

        NSLayoutConstraint.activate([
            metalView.leadingAnchor.constraint(equalTo: rootView.leadingAnchor),
            metalView.trailingAnchor.constraint(equalTo: rootView.trailingAnchor),
            metalView.topAnchor.constraint(equalTo: rootView.topAnchor),
            metalView.bottomAnchor.constraint(equalTo: rootView.bottomAnchor),

            stackView.leadingAnchor.constraint(equalTo: hudChrome.leadingAnchor, constant: 18),
            stackView.trailingAnchor.constraint(equalTo: hudChrome.trailingAnchor, constant: -18),
            stackView.topAnchor.constraint(equalTo: hudChrome.topAnchor, constant: 16),
            stackView.bottomAnchor.constraint(equalTo: hudChrome.bottomAnchor, constant: -16),

            encounterStack.leadingAnchor.constraint(equalTo: encounterChrome.leadingAnchor, constant: 18),
            encounterStack.trailingAnchor.constraint(equalTo: encounterChrome.trailingAnchor, constant: -18),
            encounterStack.topAnchor.constraint(equalTo: encounterChrome.topAnchor, constant: 16),
            encounterStack.bottomAnchor.constraint(equalTo: encounterChrome.bottomAnchor, constant: -16),

            storyStack.leadingAnchor.constraint(equalTo: storyChrome.leadingAnchor, constant: 20),
            storyStack.trailingAnchor.constraint(equalTo: storyChrome.trailingAnchor, constant: -20),
            storyStack.topAnchor.constraint(equalTo: storyChrome.topAnchor, constant: 14),
            storyStack.bottomAnchor.constraint(equalTo: storyChrome.bottomAnchor, constant: -14),

            feedbackStack.leadingAnchor.constraint(equalTo: feedbackChrome.leadingAnchor, constant: 18),
            feedbackStack.trailingAnchor.constraint(equalTo: feedbackChrome.trailingAnchor, constant: -18),
            feedbackStack.topAnchor.constraint(equalTo: feedbackChrome.topAnchor, constant: 14),
            feedbackStack.bottomAnchor.constraint(equalTo: feedbackChrome.bottomAnchor, constant: -14),

            feedbackChrome.topAnchor.constraint(equalTo: rootView.safeAreaLayoutGuide.topAnchor, constant: 24),
            feedbackChrome.centerXAnchor.constraint(equalTo: rootView.centerXAnchor),
            feedbackChrome.widthAnchor.constraint(lessThanOrEqualToConstant: 460),
            feedbackChrome.leadingAnchor.constraint(greaterThanOrEqualTo: rootView.leadingAnchor, constant: 40),
            feedbackChrome.trailingAnchor.constraint(lessThanOrEqualTo: rootView.trailingAnchor, constant: -40),

            helpStack.leadingAnchor.constraint(equalTo: helpChrome.leadingAnchor, constant: 20),
            helpStack.trailingAnchor.constraint(equalTo: helpChrome.trailingAnchor, constant: -20),
            helpStack.topAnchor.constraint(equalTo: helpChrome.topAnchor, constant: 18),
            helpStack.bottomAnchor.constraint(equalTo: helpChrome.bottomAnchor, constant: -18),

            helpChrome.centerXAnchor.constraint(equalTo: rootView.centerXAnchor),
            helpChrome.centerYAnchor.constraint(equalTo: rootView.centerYAnchor),
            helpChrome.widthAnchor.constraint(lessThanOrEqualToConstant: 640),
            helpChrome.leadingAnchor.constraint(greaterThanOrEqualTo: rootView.leadingAnchor, constant: 40),
            helpChrome.trailingAnchor.constraint(lessThanOrEqualTo: rootView.trailingAnchor, constant: -40),

            errorLabel.centerXAnchor.constraint(equalTo: rootView.centerXAnchor),
            errorLabel.centerYAnchor.constraint(equalTo: rootView.centerYAnchor),
            errorLabel.leadingAnchor.constraint(greaterThanOrEqualTo: rootView.leadingAnchor, constant: 32),
            errorLabel.trailingAnchor.constraint(lessThanOrEqualTo: rootView.trailingAnchor, constant: -32)
        ] + widePanelConstraints)

        view = rootView
        updatePanelLayout()
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        immersiveApeDebugLog("viewDidLoad")

        NotificationCenter.default.addObserver(
            self,
            selector: #selector(accessibilityDisplayOptionsDidChange(_:)),
            name: NSWorkspace.accessibilityDisplayOptionsDidChangeNotification,
            object: nil
        )

        metalView.keyDownHandler = { [weak self] event in
            self?.handleKeyDown(event)
        }

        guard metalView.device != nil else {
            errorLabel.stringValue = "Metal is required to run Immersive Ape on this Mac."
            errorLabel.isHidden = false
            statusLabel.stringValue = "Metal device unavailable."
            return
        }

        do {
            renderer = try ImmersiveApeRenderer(
                view: metalView,
                hudUpdater: { [weak self] state in
                    self?.updateHUD(state)
                },
                quitHandler: {
                    NSApp.terminate(nil)
                }
            )
            immersiveApeDebugLog("renderer initialized")
        } catch {
            immersiveApeDebugLog("renderer failed: \(error.localizedDescription)")
            errorLabel.stringValue = "Immersive Ape failed to initialize: \(error.localizedDescription)"
            errorLabel.isHidden = false
            statusLabel.stringValue = "Renderer initialization failed."
        }
    }

    override func viewDidAppear() {
        super.viewDidAppear()
        focusRenderer()
    }

    func focusRenderer() {
        DispatchQueue.main.async { [weak self] in
            guard let self else {
                return
            }
            self.view.window?.makeFirstResponder(self.metalView)
        }
    }

    func selectPreviousApe() {
        renderer?.selectPreviousApe()
    }

    func selectNextApe() {
        renderer?.selectNextApe()
    }

    func selectSelectionCandidate(slot: Int) {
        renderer?.selectSelectionCandidate(slot: slot)
    }

    func selectPreviousFocusHistory() {
        navigateFocusHistory(step: -1)
    }

    func selectNextFocusHistory() {
        navigateFocusHistory(step: 1)
    }

    func recallFocusAnchor(slot: Int) {
        guard focusAnchors.indices.contains(slot) else {
            return
        }

        guard let anchor = focusAnchors[slot] else {
            showControlFeedback(
                title: "Anchor",
                detail: "Anchor \(slot + 1) is empty. Save the current ape with Shift+\(slot + 5) first.",
                accessibility: "Focus anchor \(slot + 1) is empty."
            )
            return
        }

        guard let currentSelectionSnapshot else {
            showControlFeedback(
                title: "Anchor",
                detail: "Focus anchors are stored, but no ape is currently selected. Wait for a live focus, then try \(slot + 5) again.",
                accessibility: "No current focus is available for focus anchor recall."
            )
            return
        }

        guard anchor.apeIndex != currentSelectionSnapshot.apeIndex else {
            showControlFeedback(
                title: "Anchor",
                detail: "Anchor \(slot + 1) already matches the current focus: \(anchor.name).",
                accessibility: "Focus anchor \(slot + 1) already matches the current focus."
            )
            return
        }

        renderer?.selectApe(
            targetIndex: anchor.apeIndex,
            currentIndex: currentSelectionSnapshot.apeIndex,
            totalCount: currentSelectionSnapshot.totalCount
        )
        focusRenderer()
        showControlFeedback(
            title: "Anchor",
            detail: appendingNavigatorContext(to: "Recalled \(anchor.name) from anchor \(slot + 1).\n\(firstMeaningfulLine(anchorLabel.stringValue) ?? "")"),
            accessibility: "Recalled focus anchor \(slot + 1): \(anchor.name)."
        )
    }

    func saveFocusAnchor(slot: Int) {
        guard focusAnchors.indices.contains(slot) else {
            return
        }

        guard let currentSelectionSnapshot else {
            showControlFeedback(
                title: "Anchor",
                detail: "No selected ape is available to save right now. Wait for a live focus, then press Shift+\(slot + 5).",
                accessibility: "No current focus is available to save as an anchor."
            )
            return
        }

        focusAnchors[slot] = currentSelectionSnapshot
        updateAnchorHUD()
        updateNavigatorHUD()
        showControlFeedback(
            title: "Anchor",
            detail: appendingNavigatorContext(to: "Saved \(currentSelectionSnapshot.name) to anchor \(slot + 1).\n\(firstMeaningfulLine(anchorLabel.stringValue) ?? "")"),
            accessibility: "Saved \(currentSelectionSnapshot.name) to focus anchor \(slot + 1)."
        )
    }

    func selectPreviousCameraMode() {
        renderer?.selectPreviousCameraMode()
    }

    func selectNextCameraMode() {
        renderer?.selectNextCameraMode()
    }

    func selectPreviousPerformanceBudget() {
        guard let performanceReadout = renderer?.selectPreviousPerformanceBudget() else {
            return
        }

        setPerformanceHUD(performanceReadout)
        showControlFeedback(
            title: "Performance",
            detail: performanceReadout,
            accessibility: "Performance budget updated. \(normalizedFeedbackSentence(performanceReadout))"
        )
        lastPerformanceFeedbackLine = performanceTradeoffLine(from: performanceReadout)
        lastPerformanceStabilityFeedbackLine = performanceStabilityLine(from: performanceReadout)
        lastPerformanceHeadroomFeedbackLine = performanceHeadroomLine(from: performanceReadout)
        lastPerformanceGateFeedbackLine = performanceGateLine(from: performanceReadout)
        lastPerformanceOutlookFeedbackLine = performanceOutlookLine(from: performanceReadout)
        lastPerformanceConfidenceFeedbackLine = performanceConfidenceLine(from: performanceReadout)
        lastPerformanceRecentFeedbackLine = performanceRecentLine(from: performanceReadout)
        lastPerformanceMarginFeedbackLine = performanceMarginLine(from: performanceReadout)
    }

    func selectNextPerformanceBudget() {
        guard let performanceReadout = renderer?.selectNextPerformanceBudget() else {
            return
        }

        setPerformanceHUD(performanceReadout)
        showControlFeedback(
            title: "Performance",
            detail: performanceReadout,
            accessibility: "Performance budget updated. \(normalizedFeedbackSentence(performanceReadout))"
        )
        lastPerformanceFeedbackLine = performanceTradeoffLine(from: performanceReadout)
        lastPerformanceStabilityFeedbackLine = performanceStabilityLine(from: performanceReadout)
        lastPerformanceHeadroomFeedbackLine = performanceHeadroomLine(from: performanceReadout)
        lastPerformanceGateFeedbackLine = performanceGateLine(from: performanceReadout)
        lastPerformanceOutlookFeedbackLine = performanceOutlookLine(from: performanceReadout)
        lastPerformanceConfidenceFeedbackLine = performanceConfidenceLine(from: performanceReadout)
        lastPerformanceRecentFeedbackLine = performanceRecentLine(from: performanceReadout)
        lastPerformanceMarginFeedbackLine = performanceMarginLine(from: performanceReadout)
    }

    func setPresentationMode(_ mode: ImmersiveApePresentationMode) {
        let previousMode = currentPresentationMode
        currentPresentationMode = mode
        refreshPresentationHUD()
        updatePanelLayout()
        refreshAccessibilitySummary()
        if previousMode != mode {
            showControlFeedback(
                title: "Window",
                detail: [firstMeaningfulLine(presentationLabel.stringValue), firstMeaningfulLine(layoutLabel.stringValue)]
                    .compactMap { $0 }
                    .joined(separator: "\n"),
                accessibility: "\(mode.accessibilitySentence) \(currentPanelLayoutMode.accessibilitySentence)"
            )
        }
    }

    func announceAccessibilitySummary() {
        guard latestAccessibilitySummary.isEmpty == false else {
            return
        }

        postAccessibilityAnnouncement(latestAccessibilitySummary)
        showControlFeedback(
            title: "Accessibility",
            detail: "Scene summary announced for the current selection.",
            accessibility: "Scene summary announced."
        )
    }

    func toggleAudioAmbience() {
        audioMuted.toggle()
        audioController.update(latestAudioState, muted: audioMuted)
        refreshAudioHUD()
        showControlFeedback(
            title: "Audio",
            detail: audioMuted
                ? "Procedural ambience muted.\nLast mix: \(latestAudioState.mixSummary)"
                : "Procedural ambience live again.\n\(latestAudioState.mixSummary)",
            accessibility: audioMuted ? "Procedural ambience muted." : "Procedural ambience resumed."
        )
    }

    func toggleInteractionGuide() {
        helpOverlayVisible.toggle()
        refreshGuideHUD()
        focusRenderer()

        if helpOverlayVisible {
            postAccessibilityAnnouncement(interactionGuideAnnouncement())
            showControlFeedback(
                title: "Guide",
                detail: firstMeaningfulLine(guideLabel.stringValue) ?? "Interaction guide opened.",
                accessibility: "Interaction guide opened."
            )
        } else {
            postAccessibilityAnnouncement("Interaction guide closed.")
            showControlFeedback(
                title: "Guide",
                detail: firstMeaningfulLine(guideLabel.stringValue) ?? "Interaction guide closed.",
                accessibility: "Interaction guide closed."
            )
        }
    }

    private func postAccessibilityAnnouncement(_ message: String) {
        guard message.isEmpty == false else {
            return
        }

        let announcementUserInfo: [NSAccessibility.NotificationUserInfoKey: Any] = [
            .announcement: message,
            .priority: NSAccessibilityPriorityLevel.high.rawValue
        ]
        NSAccessibility.post(
            element: metalView as Any,
            notification: .announcementRequested,
            userInfo: announcementUserInfo
        )
    }

    func togglePause() {
        renderer?.togglePause()
    }

    func shutdown() {
        audioController.shutdown()
        renderer?.shutdown()
    }

    private func updateHUD(_ state: ImmersiveApeHUDState) {
        DispatchQueue.main.async { [weak self] in
            self?.headlineLabel.stringValue = state.headline
            self?.statusLabel.stringValue = state.status
            self?.detailLabel.stringValue = state.detail
            self?.latestAudioState = state.audioState
            self?.refreshAudioHUD()
            if let self {
                self.audioController.update(state.audioState, muted: self.audioMuted)
            }
            self?.selectionLabel.stringValue = state.selection
            self?.updateFocusHistory(state)
            self?.cameraLabel.stringValue = state.camera
            self?.setPerformanceHUD(state.performance)
            self?.storyLabel.stringValue = state.story
            self?.encounterLabel.stringValue = state.encounters
            self?.footerLabel.stringValue = state.footer
            self?.updateControlFeedback(state)
            self?.refreshHelpOverlay()
            self?.updateAccessibilityState(state)
        }
    }

    @objc private func accessibilityDisplayOptionsDidChange(_ notification: Notification) {
        applyAccessibilityDisplayPreferences()
    }

    private func configureAccessibilityElements() {
        metalView.setAccessibilityElement(true)
        metalView.setAccessibilityLabel("Immersive ape scene")
        metalView.setAccessibilityHelp("Use left and right brackets to cycle apes, number keys one through four for nearby apes, minus and equals to move through recent focus history, number keys five through eight to recall focus anchors, shift plus five through eight to save focus anchors, comma and period for camera modes, semicolon and apostrophe for performance budgets, M to mute or restore procedural ambience, slash to toggle the interaction guide, W to toggle floating window mode, A to announce the current scene summary, space to pause, and escape to quit.")
        metalView.setAccessibilityValue(latestAccessibilitySummary)

        hudChrome.setAccessibilityElement(true)
        hudChrome.setAccessibilityLabel("Simulation heads up display")
        hudChrome.setAccessibilityHelp("Contains the selected ape headline, status, focus detail, focus neighborhood, selection state, focus history, focus anchors, camera state, audio ambience state, performance budget, adaptive trade-off state, render cadence state, frame-headroom state, sample-window state, shift-gate state, adaptive-outlook state, adaptive-confidence state, adaptive-threshold state, adaptive-band state, recovery-window state, shift-history state, budget-margin state, window mode, panel layout, guide state, latest control feedback, and keyboard summary.")

        encounterChrome.setAccessibilityElement(true)
        encounterChrome.setAccessibilityLabel("Encounter panel")
        encounterChrome.setAccessibilityHelp("Contains nearby ape encounter and environment summaries.")

        storyChrome.setAccessibilityElement(true)
        storyChrome.setAccessibilityLabel("Story panel")
        storyChrome.setAccessibilityHelp("Contains the current narrative scene summary.")

        helpChrome.setAccessibilityElement(true)
        helpChrome.setAccessibilityLabel("Interaction guide")
        helpChrome.setAccessibilityHelp("Contains the current keyboard and shell reference for selection, history, focus anchors, camera, performance, audio, window, layout, accessibility, pause, and quit controls.")

        feedbackChrome.setAccessibilityElement(true)
        feedbackChrome.setAccessibilityLabel("Control feedback overlay")
        feedbackChrome.setAccessibilityHelp("Shows the most recent interaction confirmation for selection, history, focus anchors, camera, performance, audio, guide, window, accessibility, and pause controls.")

        headlineLabel.setAccessibilityLabel("Selected ape headline")
        statusLabel.setAccessibilityLabel("Simulation status")
        detailLabel.setAccessibilityLabel("Focus detail")
        navigatorLabel.setAccessibilityLabel("Focus neighborhood detail")
        selectionLabel.setAccessibilityLabel("Selection detail")
        historyLabel.setAccessibilityLabel("Focus history detail")
        anchorLabel.setAccessibilityLabel("Focus anchor detail")
        cameraLabel.setAccessibilityLabel("Camera detail")
        audioLabel.setAccessibilityLabel("Audio ambience detail")
        presentationLabel.setAccessibilityLabel("Window mode detail")
        layoutLabel.setAccessibilityLabel("Panel layout detail")
        guideLabel.setAccessibilityLabel("Guide detail")
        feedbackLabel.setAccessibilityLabel("Latest control feedback")
        performanceLabel.setAccessibilityLabel("Performance detail")
        storyLabel.setAccessibilityLabel("Story detail")
        encounterLabel.setAccessibilityLabel("Encounter detail")
        footerLabel.setAccessibilityLabel("Keyboard shortcuts")
        errorLabel.setAccessibilityLabel("Error")

        hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        encounterChrome.setAccessibilityValue(encounterLabel.stringValue)
        storyChrome.setAccessibilityValue(storyLabel.stringValue)
        helpChrome.setAccessibilityValue("Interaction guide hidden.")
        feedbackChrome.setAccessibilityValue(latestControlFeedbackAccessibility)
    }

    private func updateAccessibilityState(_ state: ImmersiveApeHUDState) {
        latestSceneAccessibilitySummary = state.accessibility
        refreshAccessibilitySummary()
        hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        encounterChrome.setAccessibilityValue(state.encounters)
        storyChrome.setAccessibilityValue(state.story)
        errorLabel.setAccessibilityValue(errorLabel.stringValue)
    }

    private func refreshPresentationHUD() {
        presentationLabel.stringValue = currentPresentationMode.hudText
        presentationLabel.setAccessibilityValue(currentPresentationMode.hudText)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
    }

    private func refreshLayoutHUD() {
        storyLabel.alignment = currentPanelLayoutMode == .wideImmersive ? .center : .left
        layoutLabel.stringValue = currentPanelLayoutMode.hudText
        layoutLabel.setAccessibilityValue(currentPanelLayoutMode.hudText)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
    }

    private func refreshGuideHUD() {
        guideLabel.stringValue = helpOverlayVisible
            ? """
                Guide 2 / 2  •  Open
                The interaction guide overlay is visible. Press / again to return to the live scene.
                """
            : """
                Guide 1 / 2  •  Hidden
                Press / to open the interaction guide for selection, anchors, camera, performance, audio, window, and pause controls.
                """
        guideLabel.setAccessibilityValue(guideLabel.stringValue)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
        refreshAccessibilitySummary()
    }

    private func refreshAudioHUD() {
        audioLabel.stringValue = audioMuted
            ? """
                Mode 2 / 2  •  Muted
                Press M to restore procedural ambience.
                Last mix: \(latestAudioState.mixSummary)
                """
            : latestAudioState.panelText

        audioLabel.setAccessibilityValue(audioLabel.stringValue)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
        refreshAccessibilitySummary()
    }

    private func setPerformanceHUD(_ text: String) {
        performanceLabel.stringValue = text
        performanceLabel.setAccessibilityValue(text)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
        refreshAccessibilitySummary()
    }

    private func audioAccessibilitySentence() -> String {
        if audioMuted {
            return "Procedural ambience muted. Last mix \(normalizedFeedbackSentence(latestAudioState.mixSummary))"
        }
        return latestAudioState.accessibility
    }

    private func performanceAccessibilitySentence() -> String {
        "Performance \(normalizedFeedbackSentence(performanceLabel.stringValue))"
    }

    private func performanceTradeoffLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Trade-offs ") })
    }

    private func performanceStabilityLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Stability ") })
    }

    private func performanceHeadroomLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Headroom ") })
    }

    private func performanceGateLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Gate ") })
    }

    private func performanceOutlookLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Outlook ") })
    }

    private func performanceConfidenceLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Confidence ") })
    }

    private func performanceRecentLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Recent ") })
    }

    private func performanceMarginLine(from text: String) -> String? {
        meaningfulLines(text).first(where: { $0.hasPrefix("Margin ") })
    }

    private func performanceFeedbackDetail(from text: String) -> String {
        let focusedLines = meaningfulLines(text).filter {
            $0.hasPrefix("LOD ") || $0.hasPrefix("Trade-offs ") || $0.hasPrefix("Stability ") || $0.hasPrefix("Headroom ") || $0.hasPrefix("Sample ") || $0.hasPrefix("Gate ") || $0.hasPrefix("Outlook ") || $0.hasPrefix("Confidence ") || $0.hasPrefix("Threshold ") || $0.hasPrefix("Band ") || $0.hasPrefix("Window ") || $0.hasPrefix("Recent ") || $0.hasPrefix("Margin ")
        }

        if focusedLines.isEmpty {
            return text
        }

        return focusedLines.joined(separator: "\n")
    }

    private func performanceFeedbackAccessibility(
        tradeoffChanged: Bool,
        tradeoffLine: String?,
        stabilityChanged: Bool,
        stabilityLine: String?,
        headroomChanged: Bool,
        headroomLine: String?,
        gateChanged: Bool,
        gateLine: String?,
        outlookChanged: Bool,
        outlookLine: String?,
        confidenceChanged: Bool,
        confidenceLine: String?,
        recentChanged: Bool,
        recentLine: String?,
        marginChanged: Bool,
        marginLine: String?
    ) -> String {
        let changeSummary = [
            tradeoffChanged ? tradeoffLine : nil,
            stabilityChanged ? stabilityLine : nil,
            headroomChanged ? headroomLine : nil,
            gateChanged ? gateLine : nil,
            outlookChanged ? outlookLine : nil,
            confidenceChanged ? confidenceLine : nil,
            recentChanged ? recentLine : nil,
            marginChanged ? marginLine : nil
        ]
        .compactMap { $0 }
        .map { normalizedFeedbackSentence($0) }
        .joined(separator: " ")

        if changeSummary.isEmpty {
            return "Performance updated."
        }

        return "Performance updated. \(changeSummary)"
    }

    private func refreshAccessibilitySummary() {
        latestAccessibilitySummary = "\(latestSceneAccessibilitySummary) \(audioAccessibilitySentence()) \(performanceAccessibilitySentence()) \(focusNavigatorAccessibilitySentence()) \(focusHistoryAccessibilitySentence()) \(focusAnchorAccessibilitySentence()) \(currentPresentationMode.accessibilitySentence) \(currentPanelLayoutMode.accessibilitySentence) \(helpOverlayVisible ? "Interaction guide open." : "Interaction guide closed.") \(latestControlFeedbackAccessibility)"
        if isViewLoaded {
            metalView.setAccessibilityValue(latestAccessibilitySummary)
        }
    }

    private func currentHUDAccessibilityValue() -> String {
        "\(headlineLabel.stringValue). \(statusLabel.stringValue). \(detailLabel.stringValue). \(navigatorLabel.stringValue). \(selectionLabel.stringValue). \(historyLabel.stringValue). \(anchorLabel.stringValue). \(cameraLabel.stringValue). \(audioLabel.stringValue). \(presentationLabel.stringValue). \(layoutLabel.stringValue). \(guideLabel.stringValue). \(feedbackLabel.stringValue). \(performanceLabel.stringValue). \(footerLabel.stringValue)"
    }

    private func selectionSnapshot(headline: String, selection: String) -> ImmersiveApeFocusHistoryEntry? {
        guard let selectionLine = firstMeaningfulLine(selection) else {
            return nil
        }

        let leadingSegment = selectionLine
            .components(separatedBy: "•")
            .first?
            .trimmingCharacters(in: .whitespacesAndNewlines) ?? ""

        guard leadingSegment.hasPrefix("Ape ") else {
            return nil
        }

        let counts = leadingSegment
            .replacingOccurrences(of: "Ape ", with: "")
            .split(separator: "/")
            .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }

        guard counts.count == 2,
              let humanIndex = Int(counts[0]),
              let totalCount = Int(counts[1]),
              humanIndex > 0,
              totalCount > 0 else {
            return nil
        }

        let name = headline
            .components(separatedBy: "•")
            .first?
            .trimmingCharacters(in: .whitespacesAndNewlines) ?? "Selected ape"

        return ImmersiveApeFocusHistoryEntry(
            apeIndex: humanIndex - 1,
            totalCount: totalCount,
            name: name
        )
    }

    private func updateFocusHistory(_ state: ImmersiveApeHUDState) {
        guard let snapshot = selectionSnapshot(headline: state.headline, selection: state.selection) else {
            currentSelectionSnapshot = nil
            updateHistoryHUD()
            updateAnchorHUD()
            updateNavigatorHUD()
            return
        }

        currentSelectionSnapshot = snapshot

        for slot in focusAnchors.indices {
            if let anchor = focusAnchors[slot], anchor.apeIndex == snapshot.apeIndex {
                focusAnchors[slot] = snapshot
            }
        }

        if let focusHistoryPosition,
           focusHistory.indices.contains(focusHistoryPosition),
           focusHistory[focusHistoryPosition].apeIndex == snapshot.apeIndex {
            focusHistory[focusHistoryPosition] = snapshot
            updateHistoryHUD()
            updateAnchorHUD()
            updateNavigatorHUD()
            return
        }

        if focusHistory.isEmpty == false,
           let focusHistoryPosition,
           focusHistoryPosition < (focusHistory.count - 1) {
            focusHistory = Array(focusHistory.prefix(focusHistoryPosition + 1))
        }

        focusHistory.append(snapshot)

        if focusHistory.count > 6 {
            focusHistory.removeFirst(focusHistory.count - 6)
        }

        self.focusHistoryPosition = focusHistory.count - 1
        updateHistoryHUD()
        updateAnchorHUD()
        updateNavigatorHUD()
    }

    private func updateHistoryHUD() {
        guard let focusHistoryPosition,
              focusHistory.indices.contains(focusHistoryPosition) else {
            historyLabel.stringValue = """
                Trail 0 / 0  •  No tracked focus yet
                Use [ ], 1-4, then - or = to revisit recent selected apes.
                """
            historyLabel.setAccessibilityValue(historyLabel.stringValue)
            refreshHelpOverlay()
            if isViewLoaded {
                hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
            }
            refreshAccessibilitySummary()
            return
        }

        let current = focusHistory[focusHistoryPosition]
        let backName = focusHistoryPosition > 0 ? focusHistory[focusHistoryPosition - 1].name : "Oldest"
        let forwardName = focusHistoryPosition < (focusHistory.count - 1) ? focusHistory[focusHistoryPosition + 1].name : "Newest"
        let backLine = focusHistoryPosition > 0
            ? "Back: \(focusHistory[focusHistoryPosition - 1].name)  •  Key: -"
            : "Back: oldest remembered focus reached."
        let forwardLine = focusHistoryPosition < (focusHistory.count - 1)
            ? "Forward: \(focusHistory[focusHistoryPosition + 1].name)  •  Key: ="
            : "Forward: newest remembered focus reached."

        historyLabel.stringValue = [
            "Trail \(focusHistoryPosition + 1) / \(focusHistory.count)  •  Back \(backName)  •  Forward \(forwardName)",
            "Now: \(current.name)",
            backLine,
            forwardLine
        ].joined(separator: "\n")
        historyLabel.setAccessibilityValue(historyLabel.stringValue)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
        refreshAccessibilitySummary()
    }

    private func focusHistoryAccessibilitySentence() -> String {
        guard let focusHistoryPosition,
              focusHistory.indices.contains(focusHistoryPosition) else {
            return "Focus history unavailable."
        }

        let current = focusHistory[focusHistoryPosition]
        let backSentence = focusHistoryPosition > 0
            ? "Older focus \(focusHistory[focusHistoryPosition - 1].name)."
            : "At the oldest remembered focus."
        let forwardSentence = focusHistoryPosition < (focusHistory.count - 1)
            ? "Newer focus \(focusHistory[focusHistoryPosition + 1].name)."
            : "At the newest remembered focus."

        return "Focus history \(focusHistoryPosition + 1) of \(focusHistory.count). Current \(current.name). \(backSentence) \(forwardSentence)"
    }

    private func updateAnchorHUD() {
        let filledCount = focusAnchors.compactMap { $0 }.count
        let slotLine = focusAnchors.enumerated().map { index, anchor in
            "\(index + 5) \(anchor?.name ?? "Empty")"
        }.joined(separator: "  •  ")

        anchorLabel.stringValue = [
            "Saved \(filledCount) / \(focusAnchors.count)  •  Recall 5-8  •  Save Shift+5-8",
            filledCount == 0 ? "No saved focus anchors yet. Save the current ape into a direct slot." : slotLine
        ].joined(separator: "\n")
        anchorLabel.setAccessibilityValue(anchorLabel.stringValue)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
        refreshAccessibilitySummary()
    }

    private func focusAnchorAccessibilitySentence() -> String {
        let filledSlots = focusAnchors.enumerated().compactMap { index, anchor -> String? in
            guard let anchor else {
                return nil
            }

            return "Slot \(index + 1) \(anchor.name)."
        }

        guard filledSlots.isEmpty == false else {
            return "Focus anchors empty."
        }

        return "Focus anchors \(filledSlots.count) of \(focusAnchors.count) filled. \(filledSlots.joined(separator: " "))"
    }

    private func selectionNeighborhoodOptions() -> [String] {
        meaningfulLines(selectionLabel.stringValue)
            .dropFirst()
            .prefix(2)
            .compactMap { line in
                let segments = line
                    .components(separatedBy: "•")
                    .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
                    .filter { $0.isEmpty == false }

                guard let lead = segments.first else {
                    return nil
                }

                let compactLead = lead.replacingOccurrences(of: ". ", with: " ")
                let distance = segments.count > 2 ? segments[2] : nil
                let direction = segments.count > 3 ? segments[3] : nil

                return [compactLead, distance, direction]
                    .compactMap { $0 }
                    .filter { $0.isEmpty == false }
                    .joined(separator: " ")
            }
    }

    private func focusNavigatorHistoryLine() -> String {
        guard let currentSelectionSnapshot else {
            return "Waiting for a live selected ape before building the focus neighborhood."
        }

        guard let focusHistoryPosition,
              focusHistory.indices.contains(focusHistoryPosition) else {
            return "Now \(currentSelectionSnapshot.name)  •  Trail 1 / 1  •  Change focus to grow reversible history."
        }

        let current = focusHistory[focusHistoryPosition]
        let backLabel = focusHistoryPosition > 0 ? focusHistory[focusHistoryPosition - 1].name : "Oldest"
        let forwardLabel = focusHistoryPosition < (focusHistory.count - 1) ? focusHistory[focusHistoryPosition + 1].name : "Newest"

        return "Now \(current.name)  •  Trail \(focusHistoryPosition + 1) / \(focusHistory.count)  •  - \(backLabel)  •  = \(forwardLabel)"
    }

    private func focusNavigatorQuickLine() -> String {
        let quickOptions = selectionNeighborhoodOptions()

        guard quickOptions.isEmpty == false else {
            return "Quick 1-4  •  No nearby handoffs inside the current selection radius."
        }

        return "Quick 1-4  •  " + quickOptions.joined(separator: "  •  ")
    }

    private func focusNavigatorAnchorLine() -> String {
        let slotLine = focusAnchors.enumerated().map { index, anchor in
            guard let anchor else {
                return "\(index + 5) Empty"
            }

            let currentSuffix = currentSelectionSnapshot?.apeIndex == anchor.apeIndex ? " (now)" : ""
            return "\(index + 5) \(anchor.name)\(currentSuffix)"
        }
        .joined(separator: "  •  ")

        return "Anchors 5-8  •  \(slotLine)"
    }

    private func updateNavigatorHUD() {
        if currentSelectionSnapshot == nil {
            navigatorLabel.stringValue = """
                Waiting for a live selected ape.
                Quick picks, remembered trail steps, and saved anchors will gather here once focus locks.
                """
        } else {
            navigatorLabel.stringValue = [
                focusNavigatorHistoryLine(),
                focusNavigatorQuickLine(),
                focusNavigatorAnchorLine()
            ].joined(separator: "\n")
        }

        navigatorLabel.setAccessibilityValue(navigatorLabel.stringValue)
        refreshHelpOverlay()
        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }
        refreshAccessibilitySummary()
    }

    private func focusNavigatorAccessibilitySentence() -> String {
        guard let currentSelectionSnapshot else {
            return "Focus neighborhood waiting for a selected ape."
        }

        let historySentence: String
        if let focusHistoryPosition,
           focusHistory.indices.contains(focusHistoryPosition) {
            let backSentence = focusHistoryPosition > 0
                ? "Back leads to \(focusHistory[focusHistoryPosition - 1].name)."
                : "Back is at the oldest remembered focus."
            let forwardSentence = focusHistoryPosition < (focusHistory.count - 1)
                ? "Forward leads to \(focusHistory[focusHistoryPosition + 1].name)."
                : "Forward is at the newest remembered focus."
            historySentence = "Focus neighborhood centered on \(currentSelectionSnapshot.name). Trail \(focusHistoryPosition + 1) of \(focusHistory.count). \(backSentence) \(forwardSentence)"
        } else {
            historySentence = "Focus neighborhood centered on \(currentSelectionSnapshot.name). Trail will grow after the next focus change."
        }

        let quickOptions = selectionNeighborhoodOptions()
        let quickSentence = quickOptions.isEmpty
            ? "No nearby quick handoffs are available."
            : "Nearby quick handoffs \(quickOptions.joined(separator: ". "))."

        let filledAnchors = focusAnchors.enumerated().compactMap { index, anchor -> String? in
            guard let anchor else {
                return nil
            }

            let currentSuffix = currentSelectionSnapshot.apeIndex == anchor.apeIndex ? ", current focus" : ""
            return "slot \(index + 1) \(anchor.name)\(currentSuffix)"
        }
        let anchorSentence = filledAnchors.isEmpty
            ? "No focus anchors are saved."
            : "Saved anchors \(filledAnchors.joined(separator: ". "))."

        return "\(historySentence) \(quickSentence) \(anchorSentence)"
    }

    private func meaningfulLines(_ text: String) -> [String] {
        text
            .split(separator: "\n")
            .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
            .filter { $0.isEmpty == false }
    }

    private func firstMeaningfulLine(_ text: String) -> String? {
        meaningfulLines(text).first
    }

    private func leadingMeaningfulLines(_ text: String, count: Int) -> String {
        meaningfulLines(text)
            .prefix(count)
            .joined(separator: "\n")
    }

    private func normalizedFeedbackSentence(_ text: String) -> String {
        let normalized = text
            .replacingOccurrences(of: "•", with: ",")
            .replacingOccurrences(of: "/", with: " of ")
            .replacingOccurrences(of: "\n", with: ". ")
        let collapsed = normalized
            .components(separatedBy: .whitespacesAndNewlines)
            .filter { $0.isEmpty == false }
            .joined(separator: " ")

        guard collapsed.isEmpty == false else {
            return "No recent control feedback."
        }

        return collapsed.hasSuffix(".") ? collapsed : "\(collapsed)."
    }

    private func appendingNavigatorContext(to detail: String) -> String {
        guard let navigatorLine = firstMeaningfulLine(navigatorLabel.stringValue),
              detail.contains(navigatorLine) == false else {
            return detail
        }

        return detail.isEmpty ? navigatorLine : "\(detail)\n\(navigatorLine)"
    }

    private func showControlFeedback(title: String, detail: String, accessibility: String) {
        guard detail.isEmpty == false else {
            return
        }

        feedbackHideTimer?.invalidate()
        feedbackHideTimer = nil
        feedbackDisplayToken += 1
        let displayToken = feedbackDisplayToken

        feedbackLabel.stringValue = "\(title)  •  \(detail)"
        feedbackLabel.setAccessibilityValue(feedbackLabel.stringValue)
        latestControlFeedbackAccessibility = accessibility
        feedbackOverlayTitleLabel.stringValue = title
        feedbackOverlayDetailLabel.stringValue = detail
        feedbackChrome.setAccessibilityValue("\(title). \(normalizedFeedbackSentence(detail))")
        feedbackChrome.alphaValue = 1
        feedbackChrome.isHidden = false
        refreshAccessibilitySummary()

        if isViewLoaded {
            hudChrome.setAccessibilityValue(currentHUDAccessibilityValue())
        }

        feedbackHideTimer = Timer.scheduledTimer(
            timeInterval: 2.6,
            target: self,
            selector: #selector(hideControlFeedbackTimerFired(_:)),
            userInfo: displayToken,
            repeats: false
        )
    }

    @objc private func hideControlFeedbackTimerFired(_ timer: Timer) {
        feedbackHideTimer = nil

        guard let displayToken = timer.userInfo as? Int, feedbackDisplayToken == displayToken else {
            return
        }

        feedbackChrome.alphaValue = 0
        feedbackChrome.isHidden = true
    }

    private func updateControlFeedback(_ state: ImmersiveApeHUDState) {
        if let selectionLine = firstMeaningfulLine(state.selection) {
            if let lastSelectionFeedbackLine {
                if lastSelectionFeedbackLine != selectionLine {
                    showControlFeedback(
                        title: "Selection",
                        detail: appendingNavigatorContext(to: leadingMeaningfulLines(state.selection, count: 2)),
                        accessibility: "Selection updated. \(normalizedFeedbackSentence(selectionLine))"
                    )
                }
            }
            lastSelectionFeedbackLine = selectionLine
        }

        if let cameraLine = firstMeaningfulLine(state.camera) {
            if let lastCameraFeedbackLine {
                if lastCameraFeedbackLine != cameraLine {
                    showControlFeedback(
                        title: "Camera",
                        detail: leadingMeaningfulLines(state.camera, count: 2),
                        accessibility: "Camera updated. \(normalizedFeedbackSentence(cameraLine))"
                    )
                }
            }
            lastCameraFeedbackLine = cameraLine
        }

        let performanceLine = performanceTradeoffLine(from: state.performance)
        let stabilityLine = performanceStabilityLine(from: state.performance)
        let headroomLine = performanceHeadroomLine(from: state.performance)
        let gateLine = performanceGateLine(from: state.performance)
        let outlookLine = performanceOutlookLine(from: state.performance)
        let confidenceLine = performanceConfidenceLine(from: state.performance)
        let recentLine = performanceRecentLine(from: state.performance)
        let marginLine = performanceMarginLine(from: state.performance)
        let tradeoffChanged = lastPerformanceFeedbackLine != nil && lastPerformanceFeedbackLine != performanceLine
        let stabilityChanged = lastPerformanceStabilityFeedbackLine != nil && lastPerformanceStabilityFeedbackLine != stabilityLine
        let headroomChanged = lastPerformanceHeadroomFeedbackLine != nil && lastPerformanceHeadroomFeedbackLine != headroomLine
        let gateChanged = lastPerformanceGateFeedbackLine != nil && lastPerformanceGateFeedbackLine != gateLine
        let outlookChanged = lastPerformanceOutlookFeedbackLine != nil && lastPerformanceOutlookFeedbackLine != outlookLine
        let confidenceChanged = lastPerformanceConfidenceFeedbackLine != nil && lastPerformanceConfidenceFeedbackLine != confidenceLine
        let recentChanged = lastPerformanceRecentFeedbackLine != nil && lastPerformanceRecentFeedbackLine != recentLine
        let marginChanged = lastPerformanceMarginFeedbackLine != nil && lastPerformanceMarginFeedbackLine != marginLine

        if tradeoffChanged || stabilityChanged || headroomChanged || gateChanged || outlookChanged || confidenceChanged || recentChanged || marginChanged {
            showControlFeedback(
                title: "Performance",
                detail: performanceFeedbackDetail(from: state.performance),
                accessibility: performanceFeedbackAccessibility(
                    tradeoffChanged: tradeoffChanged,
                    tradeoffLine: performanceLine,
                    stabilityChanged: stabilityChanged,
                    stabilityLine: stabilityLine,
                    headroomChanged: headroomChanged,
                    headroomLine: headroomLine,
                    gateChanged: gateChanged,
                    gateLine: gateLine,
                    outlookChanged: outlookChanged,
                    outlookLine: outlookLine,
                    confidenceChanged: confidenceChanged,
                    confidenceLine: confidenceLine,
                    recentChanged: recentChanged,
                    recentLine: recentLine,
                    marginChanged: marginChanged,
                    marginLine: marginLine
                )
            )
        }
        lastPerformanceFeedbackLine = performanceLine
        lastPerformanceStabilityFeedbackLine = stabilityLine
        lastPerformanceHeadroomFeedbackLine = headroomLine
        lastPerformanceGateFeedbackLine = gateLine
        lastPerformanceOutlookFeedbackLine = outlookLine
        lastPerformanceConfidenceFeedbackLine = confidenceLine
        lastPerformanceRecentFeedbackLine = recentLine
        lastPerformanceMarginFeedbackLine = marginLine

        let paused = state.status.contains("Paused")
        if let lastPausedState {
            if lastPausedState != paused {
                showControlFeedback(
                    title: "Session",
                    detail: paused ? "Simulation paused. Press Space to resume the current scene." : "Simulation resumed. The selected ape is moving live again.",
                    accessibility: paused ? "Simulation paused." : "Simulation resumed."
                )
            }
        }
        lastPausedState = paused
    }

    private func navigateFocusHistory(step: Int) {
        guard let currentSelectionSnapshot else {
            showControlFeedback(
                title: "History",
                detail: "Focus history is stored, but no ape is currently selected. Wait for a live focus, then try - or = again.",
                accessibility: "No current focus is available for history navigation."
            )
            return
        }

        guard let focusHistoryPosition,
              focusHistory.indices.contains(focusHistoryPosition),
              focusHistory.count > 1 else {
            showControlFeedback(
                title: "History",
                detail: "No focus history is available yet. Change ape selection to start a reversible trail.",
                accessibility: "No focus history available."
            )
            return
        }

        let targetPosition = focusHistoryPosition + step

        guard focusHistory.indices.contains(targetPosition) else {
            showControlFeedback(
                title: "History",
                detail: step < 0
                    ? "Already at the oldest remembered focus. Switch apes to extend the trail."
                    : "Already at the newest remembered focus. Change focus again to grow the trail.",
                accessibility: step < 0
                    ? "Already at the oldest remembered focus."
                    : "Already at the newest remembered focus."
            )
            return
        }

        let target = focusHistory[targetPosition]
        self.focusHistoryPosition = targetPosition
        updateHistoryHUD()
        updateNavigatorHUD()
        renderer?.selectApe(
            targetIndex: target.apeIndex,
            currentIndex: currentSelectionSnapshot.apeIndex,
            totalCount: currentSelectionSnapshot.totalCount
        )
        focusRenderer()
        showControlFeedback(
            title: "History",
            detail: appendingNavigatorContext(to: "\(step < 0 ? "Back" : "Forward") to \(target.name).\n\(firstMeaningfulLine(historyLabel.stringValue) ?? "")"),
            accessibility: "\(step < 0 ? "Moved back" : "Moved forward") to \(target.name) in focus history."
        )
    }

    private func compactGuideText(_ text: String) -> String {
        text
            .split(separator: "\n")
            .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
            .filter { $0.isEmpty == false }
            .joined(separator: " / ")
    }

    private func interactionGuideText() -> String {
        [
            "Press / again to close this guide.",
            "",
            "Current focus",
            headlineLabel.stringValue,
            statusLabel.stringValue,
            "",
            "Focus neighborhood",
            compactGuideText(navigatorLabel.stringValue),
            "",
            "Selection",
            "[ ] previous and next ape",
            "1-4 nearby quick pick handoffs",
            compactGuideText(selectionLabel.stringValue),
            "",
            "History",
            "- older remembered focus",
            "= newer remembered focus",
            compactGuideText(historyLabel.stringValue),
            "",
            "Focus anchors",
            "5-8 recall saved anchor slots",
            "Shift+5-8 save the current selection",
            compactGuideText(anchorLabel.stringValue),
            "",
            "Camera",
            ", . cycle camera modes",
            compactGuideText(cameraLabel.stringValue),
            "",
            "Performance",
            "; previous session budget",
            "' next session budget",
            compactGuideText(performanceLabel.stringValue),
            "",
            "Ambience",
            "M mute or restore procedural ambience",
            compactGuideText(audioLabel.stringValue),
            "",
            "Window and layout",
            "W toggle full-screen primary and floating window",
            compactGuideText(presentationLabel.stringValue),
            compactGuideText(layoutLabel.stringValue),
            "",
            "Accessibility and session",
            "A announce the current scene summary",
            "Space pause simulation",
            "Esc quit"
        ].joined(separator: "\n")
    }

    private func interactionGuideAnnouncement() -> String {
        "Interaction guide open. Use left and right brackets to cycle apes, number keys one through four for nearby apes, minus and equals for recent focus history, number keys five through eight to recall focus anchors, shift plus five through eight to save focus anchors, comma and period for camera modes, semicolon and apostrophe for performance budgets, M to mute or restore procedural ambience, W to change window mode, A to announce the current scene summary, slash to close the guide, space to pause, and escape to quit. \(audioAccessibilitySentence()) \(performanceAccessibilitySentence()) \(focusNavigatorAccessibilitySentence()) \(currentPresentationMode.accessibilitySentence) \(currentPanelLayoutMode.accessibilitySentence)"
    }

    private func refreshHelpOverlay() {
        helpChrome.isHidden = !helpOverlayVisible
        helpBodyLabel.stringValue = interactionGuideText()
        helpChrome.setAccessibilityValue(helpOverlayVisible ? interactionGuideText() : "Interaction guide hidden.")
    }

    private func preferredPanelLayoutMode() -> ImmersiveApePanelLayoutMode {
        switch currentPresentationMode {
        case .fullScreenPrimary:
            return .wideImmersive
        case .floatingWindow:
            return .sharedWindow
        }
    }

    private func updatePanelLayout() {
        let mode = preferredPanelLayoutMode()

        guard widePanelConstraints.isEmpty == false, sharedWindowConstraints.isEmpty == false else {
            currentPanelLayoutMode = mode
            return
        }

        if currentPanelLayoutMode != mode {
            switch mode {
            case .wideImmersive:
                NSLayoutConstraint.deactivate(sharedWindowConstraints)
                NSLayoutConstraint.activate(widePanelConstraints)
            case .sharedWindow:
                NSLayoutConstraint.deactivate(widePanelConstraints)
                NSLayoutConstraint.activate(sharedWindowConstraints)
            }

            currentPanelLayoutMode = mode

            if isViewLoaded {
                view.layoutSubtreeIfNeeded()
            }
        }

        refreshLayoutHUD()
        refreshAccessibilitySummary()
    }

    private func applyAccessibilityDisplayPreferences() {
        let workspace = NSWorkspace.shared
        let emphasizeChrome = workspace.accessibilityDisplayShouldIncreaseContrast
            || workspace.accessibilityDisplayShouldDifferentiateWithoutColor
        let reduceTransparency = workspace.accessibilityDisplayShouldReduceTransparency
        let chromeViews = [hudChrome, encounterChrome, storyChrome, helpChrome, feedbackChrome]
        let chromeBackgroundAlpha: CGFloat = reduceTransparency
            ? (emphasizeChrome ? 0.96 : 0.88)
            : (emphasizeChrome ? 0.26 : 0.08)
        let chromeBorderWidth: CGFloat = emphasizeChrome ? 1.8 : 0.7
        let chromeBorderAlpha: CGFloat = emphasizeChrome ? 0.46 : 0.14

        for chrome in chromeViews {
            chrome.material = reduceTransparency ? .windowBackground : .hudWindow
            chrome.blendingMode = .withinWindow
            chrome.state = .active
            chrome.wantsLayer = true
            chrome.layer?.cornerRadius = 18
            chrome.layer?.borderWidth = chromeBorderWidth
            chrome.layer?.borderColor = NSColor.white.withAlphaComponent(chromeBorderAlpha).cgColor
            chrome.layer?.backgroundColor = NSColor(calibratedWhite: 0.05, alpha: chromeBackgroundAlpha).cgColor
        }

        statusLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.9)
        detailLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.96 : 0.82)
        navigatorLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        selectionLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        historyLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        anchorLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        cameraLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        audioLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        presentationLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        layoutLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        guideLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        feedbackLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        performanceLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.88)
        storyLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.9)
        encounterLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.85)
        footerLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.94 : 0.75)
        feedbackOverlayDetailLabel.textColor = NSColor.white.withAlphaComponent(emphasizeChrome ? 0.98 : 0.9)
        errorLabel.textColor = emphasizeChrome ? NSColor.systemRed.withAlphaComponent(0.96) : .white
    }

    private func handleKeyDown(_ event: NSEvent) {
        switch event.keyCode {
        case 53:
            NSApp.terminate(nil)
            return
        case 49:
            togglePause()
            return
        default:
            break
        }

        guard let characters = event.charactersIgnoringModifiers else {
            return
        }

        let usesShift = event.modifierFlags.intersection(.deviceIndependentFlagsMask).contains(.shift)
        if let slot = ["5": 0, "6": 1, "7": 2, "8": 3][characters] {
            if usesShift {
                saveFocusAnchor(slot: slot)
            } else {
                recallFocusAnchor(slot: slot)
            }
            return
        }

        switch characters {
        case "[":
            selectPreviousApe()
        case "]":
            selectNextApe()
        case "1":
            selectSelectionCandidate(slot: 0)
        case "2":
            selectSelectionCandidate(slot: 1)
        case "3":
            selectSelectionCandidate(slot: 2)
        case "4":
            selectSelectionCandidate(slot: 3)
        case "-":
            selectPreviousFocusHistory()
        case "=", "+":
            selectNextFocusHistory()
        case ",":
            selectPreviousCameraMode()
        case ".":
            selectNextCameraMode()
        case ";":
            selectPreviousPerformanceBudget()
        case "'":
            selectNextPerformanceBudget()
        case "m", "M":
            toggleAudioAmbience()
        case "/", "?":
            toggleInteractionGuide()
        case "w", "W":
            (view.window?.windowController as? ImmersiveApeWindowController)?.togglePresentationMode()
        case "a", "A":
            announceAccessibilitySummary()
        case " ":
            togglePause()
        default:
            break
        }
    }
}
