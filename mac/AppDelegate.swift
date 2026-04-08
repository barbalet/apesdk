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
            .version: "Cycle 6 Interpolated Coast and Cloud Pass",
            .credits: NSAttributedString(string: "A full-screen Metal viewer for ApeSDK, now smoothing terrain, shoreline, beach, water-edge, and cloud transitions by interpolating between terrain samples before drawing the world.")
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

        controlMenuItem.submenu = controlMenu
        mainMenu.addItem(controlMenuItem)

        let viewMenuItem = NSMenuItem(title: "View", action: nil, keyEquivalent: "")
        let viewMenu = NSMenu(title: "View")
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

            switch characters {
            case "[":
                self.windowController?.immersiveViewController.selectPreviousApe()
                return nil
            case "]":
                self.windowController?.immersiveViewController.selectNextApe()
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
    private let presentationMode: ImmersiveApePresentationMode

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

    func windowWillClose(_ notification: Notification) {
        immersiveApeDebugLog("windowWillClose")
        NSApp.terminate(nil)
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

    private func applyPresentation() {
        guard let window else {
            return
        }

        switch presentationMode {
        case .fullScreenPrimary:
            if let screen = window.screen ?? NSScreen.main {
                window.setFrame(screen.frame, display: true)
            }
            window.minSize = NSSize(width: 960, height: 640)
        case .floatingWindow:
            break
        }
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

final class ImmersiveApeViewController: NSViewController {
    private let headlineLabel = NSTextField(labelWithString: "Immersive Ape")
    private let statusLabel = NSTextField(labelWithString: "Booting ApeSDK...")
    private let detailLabel = NSTextField(labelWithString: "Linking ape goals, weather, and surf context...")
    private let storyLabel = NSTextField(labelWithString: "Following the selected ape through the procedural world...")
    private let encounterTitleLabel = NSTextField(labelWithString: "Intent + Encounters")
    private let encounterLabel = NSTextField(labelWithString: "Scanning nearby apes...")
    private let footerLabel = NSTextField(labelWithString: "Esc quit")
    private let errorLabel = NSTextField(labelWithString: "")

    private var metalView: ImmersiveApeMetalView!
    private var renderer: ImmersiveApeRenderer?

    override func loadView() {
        let rootView = NSView()
        rootView.wantsLayer = true
        rootView.layer?.backgroundColor = NSColor.black.cgColor

        let metalView = ImmersiveApeMetalView(frame: .zero, device: MTLCreateSystemDefaultDevice())
        metalView.translatesAutoresizingMaskIntoConstraints = false
        metalView.clearColor = MTLClearColor(red: 0.02, green: 0.03, blue: 0.05, alpha: 1.0)
        rootView.addSubview(metalView)
        self.metalView = metalView

        let hudChrome = NSVisualEffectView()
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

        let encounterChrome = NSVisualEffectView()
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

        let storyChrome = NSVisualEffectView()
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

        headlineLabel.font = .systemFont(ofSize: 24, weight: .bold)
        headlineLabel.textColor = .white
        statusLabel.font = .systemFont(ofSize: 13, weight: .medium)
        statusLabel.textColor = NSColor.white.withAlphaComponent(0.9)
        detailLabel.font = .systemFont(ofSize: 12, weight: .regular)
        detailLabel.textColor = NSColor.white.withAlphaComponent(0.82)
        storyLabel.font = .systemFont(ofSize: 13, weight: .medium)
        storyLabel.textColor = NSColor.white.withAlphaComponent(0.9)
        storyLabel.alignment = .center
        encounterTitleLabel.font = .systemFont(ofSize: 15, weight: .semibold)
        encounterTitleLabel.textColor = .white
        encounterLabel.font = .monospacedSystemFont(ofSize: 12, weight: .medium)
        encounterLabel.textColor = NSColor.white.withAlphaComponent(0.85)
        footerLabel.font = .monospacedSystemFont(ofSize: 11, weight: .medium)
        footerLabel.textColor = NSColor.white.withAlphaComponent(0.75)

        for label in [statusLabel, detailLabel, storyLabel, encounterLabel, footerLabel] {
            label.lineBreakMode = .byWordWrapping
            label.cell?.wraps = true
            label.cell?.usesSingleLineMode = false
        }

        stackView.addArrangedSubview(headlineLabel)
        stackView.addArrangedSubview(statusLabel)
        stackView.addArrangedSubview(detailLabel)
        stackView.addArrangedSubview(footerLabel)

        encounterStack.addArrangedSubview(encounterTitleLabel)
        encounterStack.addArrangedSubview(encounterLabel)

        storyStack.addArrangedSubview(storyLabel)

        errorLabel.translatesAutoresizingMaskIntoConstraints = false
        errorLabel.font = .systemFont(ofSize: 18, weight: .semibold)
        errorLabel.textColor = .white
        errorLabel.alignment = .center
        errorLabel.isHidden = true
        rootView.addSubview(errorLabel)

        NSLayoutConstraint.activate([
            metalView.leadingAnchor.constraint(equalTo: rootView.leadingAnchor),
            metalView.trailingAnchor.constraint(equalTo: rootView.trailingAnchor),
            metalView.topAnchor.constraint(equalTo: rootView.topAnchor),
            metalView.bottomAnchor.constraint(equalTo: rootView.bottomAnchor),

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
            storyChrome.trailingAnchor.constraint(lessThanOrEqualTo: rootView.trailingAnchor, constant: -24),

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

            errorLabel.centerXAnchor.constraint(equalTo: rootView.centerXAnchor),
            errorLabel.centerYAnchor.constraint(equalTo: rootView.centerYAnchor),
            errorLabel.leadingAnchor.constraint(greaterThanOrEqualTo: rootView.leadingAnchor, constant: 32),
            errorLabel.trailingAnchor.constraint(lessThanOrEqualTo: rootView.trailingAnchor, constant: -32)
        ])

        view = rootView
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        immersiveApeDebugLog("viewDidLoad")

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

    func togglePause() {
        renderer?.togglePause()
    }

    func shutdown() {
        renderer?.shutdown()
    }

    private func updateHUD(_ state: ImmersiveApeHUDState) {
        DispatchQueue.main.async { [weak self] in
            self?.headlineLabel.stringValue = state.headline
            self?.statusLabel.stringValue = state.status
            self?.detailLabel.stringValue = state.detail
            self?.storyLabel.stringValue = state.story
            self?.encounterLabel.stringValue = state.encounters
            self?.footerLabel.stringValue = state.footer
        }
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

        switch characters {
        case "[":
            selectPreviousApe()
        case "]":
            selectNextApe()
        case " ":
            togglePause()
        default:
            break
        }
    }
}
