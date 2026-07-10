/****************************************************************

 ApeSimApp.swift

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

import SwiftUI
import UIKit

// MARK: - SwiftUI App Entry Point
private enum ApeSimulationWindow {
    static let id = "simulation-window"

    static var supportsAdditionalWindows: Bool {
        UIApplication.shared.supportsMultipleScenes && UIDevice.current.userInterfaceIdiom == .pad
    }
}

enum ApeSimulationSurface {
    case view
    case terrain
    case control

    var identifier: n_int {
        switch self {
        case .view:
            return n_int(NUM_VIEW)
        case .terrain:
            return n_int(NUM_TERRAIN)
        case .control:
            return n_int(NUM_CONTROL)
        }
    }

#if targetEnvironment(macCatalyst)
    var title: String {
        switch self {
        case .view:
            return "View"
        case .terrain:
            return "Terrain"
        case .control:
            return "Control"
        }
    }

    var minimumSize: CGSize {
        switch self {
        case .view:
            return CGSize(width: 512, height: 512)
        case .terrain:
            return CGSize(width: 512, height: 400)
        case .control:
            return CGSize(width: 342, height: 512)
        }
    }

    var defaultSize: CGSize {
        switch self {
        case .view:
            return CGSize(width: 512, height: 512)
        case .terrain:
            return CGSize(width: 600, height: 500)
        case .control:
            return CGSize(width: 400, height: 600)
        }
    }
#endif
}

#if targetEnvironment(macCatalyst)
private enum ApeCatalystWindow {
    static let viewID = "catalyst-view-window"
    static let terrainID = "catalyst-terrain-window"
    static let controlID = "catalyst-control-window"
}
#endif

@main
struct ApeSimApp: App {
    @UIApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
#if targetEnvironment(macCatalyst)
    @StateObject private var catalystSceneModel = ApeSimulationSceneModel()
    @State private var isPaused = false
    @State private var isTerritory = false
    @State private var isWeather = true
    @State private var isBrain = true
    @State private var isBraincode = false
#endif

    var body: some Scene {
#if targetEnvironment(macCatalyst)
        WindowGroup("View", id: ApeCatalystWindow.viewID) {
            CatalystSimulationWindow(sceneModel: catalystSceneModel,
                                     surface: .view,
                                     opensAuxiliaryWindows: true)
        }
        .defaultSize(width: ApeSimulationSurface.view.defaultSize.width,
                     height: ApeSimulationSurface.view.defaultSize.height)
        .commands {
            CommandGroup(replacing: CommandGroupPlacement.newItem) {
                Button("New Simulation") {
                    catalystSceneModel.newSimulation()
                }
                .keyboardShortcut("n", modifiers: [.command])
            }

            CommandMenu("Control") {
                Button(isPaused ? "Resume" : "Pause") {
                    isPaused = catalystSceneModel.menu(n_int(NA_MENU_PAUSE)) == 1
                }
                .keyboardShortcut("p", modifiers: [.command])
                Divider()
                Button("Previous Ape") {
                    _ = catalystSceneModel.menu(n_int(NA_MENU_PREVIOUS_APE))
                }
                .keyboardShortcut("[", modifiers: [.command])
                Button("Next Ape") {
                    _ = catalystSceneModel.menu(n_int(NA_MENU_NEXT_APE))
                }
                .keyboardShortcut("]", modifiers: [.command])
                Divider()
                Button("Clear Errors") {
                    _ = catalystSceneModel.menu(n_int(NA_MENU_CLEAR_ERRORS))
                }
                .keyboardShortcut("e", modifiers: [.command])
                Divider()
                Button(isTerritory ? "No Territory" : "Territory") {
                    isTerritory = catalystSceneModel.menu(n_int(NA_MENU_TERRITORY)) == 1
                }
                Button(isWeather ? "No Weather" : "Weather") {
                    isWeather = catalystSceneModel.menu(n_int(NA_MENU_WEATHER)) == 1
                }
                Button(isBrain ? "No Brain" : "Brain") {
                    isBrain = catalystSceneModel.menu(n_int(NA_MENU_BRAIN)) == 1
                }
                Button(isBraincode ? "No Braincode" : "Braincode") {
                    isBraincode = catalystSceneModel.menu(n_int(NA_MENU_BRAINCODE)) == 1
                }
            }
        }

        WindowGroup("Terrain", id: ApeCatalystWindow.terrainID) {
            CatalystSimulationWindow(sceneModel: catalystSceneModel,
                                     surface: .terrain,
                                     opensAuxiliaryWindows: false)
        }
        .defaultSize(width: ApeSimulationSurface.terrain.defaultSize.width,
                     height: ApeSimulationSurface.terrain.defaultSize.height)

        WindowGroup("Control", id: ApeCatalystWindow.controlID) {
            CatalystSimulationWindow(sceneModel: catalystSceneModel,
                                     surface: .control,
                                     opensAuxiliaryWindows: false)
        }
        .defaultSize(width: ApeSimulationSurface.control.defaultSize.width,
                     height: ApeSimulationSurface.control.defaultSize.height)
#else
        WindowGroup(id: ApeSimulationWindow.id) {
            ContentView()
        }
#endif
    }
}

// MARK: - AppDelegate Replacement
class AppDelegate: NSObject, UIApplicationDelegate {
    func application(_ application: UIApplication,
                     didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey : Any]? = nil) -> Bool {
        application.isIdleTimerDisabled = true
        return true
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        application.isIdleTimerDisabled = false
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        application.isIdleTimerDisabled = true
    }

    func applicationWillTerminate(_ application: UIApplication) {
        application.isIdleTimerDisabled = false
    }
}

// MARK: - Main ContentView
struct ContentView: View {
    @Environment(\.openWindow) private var openWindow
    @Environment(\.horizontalSizeClass) private var horizontalSizeClass
    @StateObject private var sceneModel = ApeSimulationSceneModel()

    private var isWideLayout: Bool {
        horizontalSizeClass == .regular
    }

    var body: some View {
        ASiOSViewRepresentable(sceneModel: sceneModel,
                               surface: .terrain,
                               rotatesForUIKit: true)
            .ignoresSafeArea()
            .overlay(alignment: isWideLayout ? .trailing : .bottomTrailing) {
                CommandPanel(isWideLayout: isWideLayout,
                             sceneModel: sceneModel,
                             canOpenNewWindow: isWideLayout && ApeSimulationWindow.supportsAdditionalWindows) {
                    openWindow(id: ApeSimulationWindow.id)
                }
                    .padding(isWideLayout ? 24 : 16)
            }
    }
}

#if targetEnvironment(macCatalyst)
// MARK: - Catalyst Multi-Window Surface
struct CatalystSimulationWindow: View {
    @Environment(\.openWindow) private var openWindow
    @ObservedObject var sceneModel: ApeSimulationSceneModel
    let surface: ApeSimulationSurface
    let opensAuxiliaryWindows: Bool
    @State private var didOpenAuxiliaryWindows = false

    var body: some View {
        ASiOSViewRepresentable(sceneModel: sceneModel,
                               surface: surface,
                               rotatesForUIKit: false)
            .frame(minWidth: surface.minimumSize.width,
                   idealWidth: surface.defaultSize.width,
                   minHeight: surface.minimumSize.height,
                   idealHeight: surface.defaultSize.height)
            .ignoresSafeArea()
            .accessibilityIdentifier("Catalyst\(surface.title)Surface")
            .onAppear {
                openAuxiliaryWindowsIfNeeded()
            }
    }

    private func openAuxiliaryWindowsIfNeeded() {
        guard opensAuxiliaryWindows, didOpenAuxiliaryWindows == false else {
            return
        }

        didOpenAuxiliaryWindows = true
        openWindow(id: ApeCatalystWindow.terrainID)
        openWindow(id: ApeCatalystWindow.controlID)
    }
}
#endif

// MARK: - Scene-owned Simulation Model
final class ApeSimulationSceneModel: ObservableObject {
    private let session = shared_session_create(n_uint(CFAbsoluteTimeGetCurrent()))
    private let startsSessionAutomatically: Bool
    private var hasStartedSession = false
    private var timer: Timer?
    private let renderViews = NSHashTable<ASiOSView>.weakObjects()
    private var renderDimensions: [n_int: (width: Int, height: Int)] = [:]

    init(startsSessionAutomatically: Bool = !ApeSimulationSceneModel.isRunningUnderUnitTests) {
        self.startsSessionAutomatically = startsSessionAutomatically
        startSessionIfNeeded()
    }

    deinit {
        timer?.invalidate()
        shared_session_destroy(session)
    }

    func attach(view: ASiOSView) {
        renderViews.add(view)
        startTimerIfNeeded()
    }

    func detach(view: ASiOSView) {
        renderViews.remove(view)
    }

    func updateRenderDimensions(width: Int, height: Int, surface: ApeSimulationSurface) {
        renderDimensions[surface.identifier] = (width, height)
    }

    func draw(into outputBuffer: UnsafeMutablePointer<n_byte4>?,
              width: Int,
              height: Int,
              surface: ApeSimulationSurface,
              rotatesForUIKit: Bool) {
        startSessionIfNeeded()
        guard width > 0, height > 0 else { return }

        if rotatesForUIKit {
            shared_session_draw_ios(session, outputBuffer, n_int(Int32(width)), n_int(Int32(height)))
            return
        }

        guard let outputBuffer,
              let source = shared_session_draw(session,
                                               surface.identifier,
                                               n_int(Int32(width)),
                                               n_int(Int32(height)),
                                               0) else {
            return
        }

        let byteCount = width * height * MemoryLayout<n_byte4>.stride
        outputBuffer.withMemoryRebound(to: n_byte.self, capacity: byteCount) { destination in
            _ = memcpy(destination, source, byteCount)
        }
    }

    func cycle(surface: ApeSimulationSurface, rotatesForUIKit: Bool) {
        startSessionIfNeeded()
        let ticks = n_uint(CFAbsoluteTimeGetCurrent())
        if rotatesForUIKit {
            _ = shared_session_cycle_ios(session, ticks)
        } else {
            _ = shared_session_cycle(session, ticks, surface.identifier)
        }
    }

    func newSimulation() {
        startSessionIfNeeded()
        shared_session_new(session, n_uint(CFAbsoluteTimeGetCurrent()))
    }

    func menu(_ menuValue: n_int) -> n_int {
        startSessionIfNeeded()
        return shared_session_menu(session, menuValue)
    }

    func previousApe() {
        shared_session_menu(session, NA_MENU_PREVIOUS_APE)
    }

    func nextApe() {
        shared_session_menu(session, NA_MENU_NEXT_APE)
    }

    func mouseReceived(at location: CGPoint,
                       in bounds: CGRect,
                       surface: ApeSimulationSurface,
                       rotatesForUIKit: Bool) {
        startSessionIfNeeded()
        let simulationLocation = simulationPoint(for: location,
                                                 in: bounds,
                                                 surface: surface,
                                                 rotatesForUIKit: rotatesForUIKit)
        shared_session_mouseReceived(session,
                                     n_double(Float(simulationLocation.x)),
                                     n_double(Float(simulationLocation.y)),
                                     surface.identifier)
    }

    func mouseUp() {
        startSessionIfNeeded()
        shared_session_mouseUp(session)
    }

    private static var isRunningUnderUnitTests: Bool {
        let environment = ProcessInfo.processInfo.environment
        return environment["XCTestConfigurationFilePath"] != nil ||
               environment["XCTestBundlePath"] != nil
    }

    private func startSessionIfNeeded() {
        guard startsSessionAutomatically, hasStartedSession == false else {
            return
        }

        shared_session_init(session, n_int(NUM_CONTROL), n_uint(CFAbsoluteTimeGetCurrent()))
        hasStartedSession = true
    }

    private func startTimerIfNeeded() {
        guard timer == nil else { return }

        timer = Timer.scheduledTimer(withTimeInterval: 1.0 / 60.0, repeats: true) { [weak self] _ in
            DispatchQueue.main.async {
                self?.renderViews.allObjects.forEach { view in
                    view.setNeedsDisplay()
                }
            }
        }
    }

    func simulationPoint(for location: CGPoint,
                         in bounds: CGRect,
                         surface: ApeSimulationSurface,
                         rotatesForUIKit: Bool,
                         displayRect: CGRect? = nil) -> CGPoint {
        let dimensions = renderDimensions[surface.identifier]
        guard let dimensions,
              bounds.width > 0,
              bounds.height > 0,
              dimensions.width > 0,
              dimensions.height > 0 else {
            return location
        }

        let inputRect = displayRect ?? bounds
        guard inputRect.width > 0, inputRect.height > 0 else {
            return location
        }

        let localX = min(max(location.x - inputRect.minX, 0), inputRect.width)
        let localY = min(max(location.y - inputRect.minY, 0), inputRect.height)
        let scaleX = CGFloat(dimensions.width) / inputRect.width
        let scaleY = CGFloat(dimensions.height) / inputRect.height

        if rotatesForUIKit {
            let rotatedX = CGFloat(dimensions.width - 1) - (localX * scaleX)
            let rotatedY = CGFloat(dimensions.height - 1) - (localY * scaleY)

            return CGPoint(x: min(max(rotatedX, 0), CGFloat(dimensions.width - 1)),
                           y: min(max(rotatedY, 0), CGFloat(dimensions.height - 1)))
        }

        let scaledX = localX * scaleX
#if targetEnvironment(macCatalyst)
        let scaledY = (inputRect.height - localY) * scaleY
#else
        let scaledY = localY * scaleY
#endif

        return CGPoint(x: min(max(scaledX, 0), CGFloat(dimensions.width - 1)),
                       y: min(max(scaledY, 0), CGFloat(dimensions.height - 1)))
    }
}

// MARK: - Command Panel
struct CommandPanel: View {
    let isWideLayout: Bool
    let sceneModel: ApeSimulationSceneModel
    let canOpenNewWindow: Bool
    let openNewWindow: () -> Void

    var body: some View {
        Group {
            if isWideLayout {
                VStack(spacing: 10) {
                    if canOpenNewWindow {
                        commandButton(title: "New Window",
                                      systemImage: "plus.rectangle.on.rectangle",
                                      accessibilityIdentifier: "NewSimulationWindowButton",
                                      action: openNewWindow)
                    }
                    commandButton(title: "Previous Ape", systemImage: "chevron.left") {
                        sceneModel.previousApe()
                    }
                    commandButton(title: "New Simulation", systemImage: "arrow.triangle.2.circlepath", isPrimary: true) {
                        sceneModel.newSimulation()
                    }
                    commandButton(title: "Next Ape", systemImage: "chevron.right") {
                        sceneModel.nextApe()
                    }
                }
            } else {
                HStack(spacing: 8) {
                    commandButton(title: "Previous Ape", systemImage: "chevron.left") {
                        sceneModel.previousApe()
                    }
                    commandButton(title: "New Simulation", systemImage: "arrow.triangle.2.circlepath", isPrimary: true) {
                        sceneModel.newSimulation()
                    }
                    commandButton(title: "Next Ape", systemImage: "chevron.right") {
                        sceneModel.nextApe()
                    }
                }
            }
        }
        .padding(isWideLayout ? 10 : 8)
        .background(.ultraThinMaterial, in: Capsule())
    }

    private func commandButton(title: String,
                               systemImage: String,
                               isPrimary: Bool = false,
                               accessibilityIdentifier: String? = nil,
                               action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: systemImage)
                .font(.system(size: isWideLayout ? 22 : 19, weight: .semibold))
                .frame(width: isWideLayout ? 50 : 44, height: isWideLayout ? 50 : 44)
                .foregroundStyle(isPrimary ? .white : .primary)
                .background(isPrimary ? Color.accentColor : Color(uiColor: .secondarySystemBackground), in: Circle())
                .overlay {
                    Circle()
                        .stroke(Color.primary.opacity(isPrimary ? 0 : 0.16), lineWidth: 1)
                }
        }
        .buttonStyle(.plain)
        .accessibilityLabel(Text(title))
        .accessibilityIdentifier(accessibilityIdentifier ?? title.replacingOccurrences(of: " ", with: ""))
        .help(title)
    }
}

// MARK: - ASiOSView Wrapper
struct ASiOSViewRepresentable: UIViewRepresentable {
    let sceneModel: ApeSimulationSceneModel
    let surface: ApeSimulationSurface
    let rotatesForUIKit: Bool

    func makeUIView(context: Context) -> ASiOSView {
        let view = ASiOSView(sceneModel: sceneModel,
                             surface: surface,
                             rotatesForUIKit: rotatesForUIKit)
        view.isMultipleTouchEnabled = true
        sceneModel.attach(view: view)
        return view
    }

    func updateUIView(_ uiView: ASiOSView, context: Context) { }

    static func dismantleUIView(_ uiView: ASiOSView, coordinator: ()) {
        uiView.detachFromSceneModel()
    }
}

// MARK: - ASiOSView (Original UIView Logic)
class ASiOSView: UIView {
#if targetEnvironment(macCatalyst)
    private static let catalystMapRenderDimension = 512
#endif

    private let sceneModel: ApeSimulationSceneModel
    private let surface: ApeSimulationSurface
    private let rotatesForUIKit: Bool
    private var offscreenBuffer = [UInt32](repeating: 0, count: 512 * 512)
    private var drawRef: CGContext?
    private var oldDimensionX: Int = -1
    private var oldDimensionY: Int = -1

    init(sceneModel: ApeSimulationSceneModel,
         surface: ApeSimulationSurface,
         rotatesForUIKit: Bool,
         frame: CGRect = .zero) {
        self.sceneModel = sceneModel
        self.surface = surface
        self.rotatesForUIKit = rotatesForUIKit
        super.init(frame: frame)
    }

    required init?(coder: NSCoder) {
        self.sceneModel = ApeSimulationSceneModel()
        self.surface = .terrain
        self.rotatesForUIKit = true
        super.init(coder: coder)
        self.sceneModel.attach(view: self)
    }

#if targetEnvironment(macCatalyst)
    override func didMoveToWindow() {
        super.didMoveToWindow()

        guard let windowScene = window?.windowScene else {
            return
        }

        windowScene.title = surface.title
        windowScene.sizeRestrictions?.minimumSize = surface.minimumSize
    }
#endif

    func detachFromSceneModel() {
        sceneModel.detach(view: self)
    }

    func simulationPoint(for location: CGPoint) -> CGPoint {
        let renderDimensions = renderDimensions(for: bounds)
        return sceneModel.simulationPoint(for: location,
                                          in: bounds,
                                          surface: surface,
                                          rotatesForUIKit: rotatesForUIKit,
                                          displayRect: displayRect(renderDimensions: renderDimensions,
                                                                   in: bounds))
    }

    private func renderDimensions(for bounds: CGRect) -> (width: Int, height: Int) {
#if targetEnvironment(macCatalyst)
        if surface == .view {
            return (Self.catalystMapRenderDimension, Self.catalystMapRenderDimension)
        }
#endif

        return (max(1, Int(bounds.width)), max(1, Int(bounds.height)))
    }

    private func displayRect(renderDimensions: (width: Int, height: Int), in bounds: CGRect) -> CGRect {
        let renderWidth = CGFloat(renderDimensions.width)
        let renderHeight = CGFloat(renderDimensions.height)
        guard renderWidth > 0, renderHeight > 0, bounds.width > 0, bounds.height > 0 else {
            return bounds
        }

        let scale = min(bounds.width / renderWidth, bounds.height / renderHeight)
        let width = renderWidth * scale
        let height = renderHeight * scale
        return CGRect(x: bounds.midX - (width / 2),
                      y: bounds.midY - (height / 2),
                      width: width,
                      height: height)
    }

    override func draw(_ rect: CGRect) {
        let scaleFactor: CGFloat = 1.0
        let renderBounds = bounds
        let renderDimensions = renderDimensions(for: renderBounds)
        let displayRect = displayRect(renderDimensions: renderDimensions, in: renderBounds)
        let dimensionX = Int(CGFloat(renderDimensions.width) * scaleFactor)
        let dimensionY = Int(CGFloat(renderDimensions.height) * scaleFactor)

        guard dimensionX > 0, dimensionY > 0 else { return }
        guard let context = UIGraphicsGetCurrentContext() else { return }

        let pixelCount = dimensionX * dimensionY
        if offscreenBuffer.count < pixelCount {
            offscreenBuffer = [UInt32](repeating: 0, count: pixelCount)
            drawRef = nil
        }

        if drawRef == nil || oldDimensionX != dimensionX || oldDimensionY != dimensionY {
            let colorSpace = CGColorSpaceCreateDeviceRGB()
            drawRef = offscreenBuffer.withUnsafeMutableBufferPointer { buffer in
                CGContext(data: buffer.baseAddress,
                          width: dimensionX,
                          height: dimensionY,
                          bitsPerComponent: 8,
                          bytesPerRow: dimensionX * 4,
                          space: colorSpace,
                          bitmapInfo: CGImageAlphaInfo.noneSkipFirst.rawValue | CGBitmapInfo.byteOrder32Big.rawValue)
            }

            oldDimensionX = dimensionX
            oldDimensionY = dimensionY
        }

        sceneModel.updateRenderDimensions(width: dimensionX, height: dimensionY, surface: surface)

        context.saveGState()
        context.setFillColor(UIColor.black.cgColor)
        context.fill(renderBounds)
        offscreenBuffer.withUnsafeMutableBufferPointer { buffer in
            sceneModel.draw(into: buffer.baseAddress,
                            width: dimensionX,
                            height: dimensionY,
                            surface: surface,
                            rotatesForUIKit: rotatesForUIKit)
        }

        if let image = drawRef?.makeImage() {
            context.setBlendMode(.copy)
#if targetEnvironment(macCatalyst)
            if rotatesForUIKit == false {
                context.translateBy(x: 0, y: renderBounds.height)
                context.scaleBy(x: 1, y: -1)
            }
#endif
            context.draw(image, in: displayRect)
        }

        context.restoreGState()
        sceneModel.cycle(surface: surface, rotatesForUIKit: rotatesForUIKit)
        DispatchQueue.main.async { [weak self] in
            self?.setNeedsDisplay()
        }
    }

    // MARK: - Touch Handling

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        touchesMoved(touches, with: event)
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard let allTouches = event?.touches(for: self) else { return }

        for touch in allTouches {
            let location = touch.location(in: self)
            print("Touch moved to: \(location)")
            sceneModel.mouseReceived(at: location,
                                     in: bounds,
                                     surface: surface,
                                     rotatesForUIKit: rotatesForUIKit)
        }
    }

    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard let allTouches = event?.touches(for: self) else { return }

        for touch in allTouches {
            let location = touch.location(in: self)
            print("Touch cancelled at: \(location)")
        }
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        sceneModel.mouseUp()
    }

    // MARK: - Motion Events
    override func motionEnded(_ motion: UIEvent.EventSubtype, with event: UIEvent?) {
        if motion == .motionShake {
            sceneModel.nextApe()
        }
    }
}
