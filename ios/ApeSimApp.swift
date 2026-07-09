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

@main
struct ApeSimApp: App {
    @UIApplicationDelegateAdaptor(AppDelegate.self) var appDelegate

    var body: some Scene {
        WindowGroup(id: ApeSimulationWindow.id) {
            ContentView()
        }
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
        ASiOSViewRepresentable(sceneModel: sceneModel)
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

// MARK: - Scene-owned Simulation Model
final class ApeSimulationSceneModel: ObservableObject {
    private let session = shared_session_create(n_uint(CFAbsoluteTimeGetCurrent()))
    private var timer: Timer?
    private weak var renderView: ASiOSView?
    private var renderWidth: Int = -1
    private var renderHeight: Int = -1

    init() {
        shared_session_init(session, n_int(NUM_CONTROL), n_uint(CFAbsoluteTimeGetCurrent()))
    }

    deinit {
        timer?.invalidate()
        shared_session_destroy(session)
    }

    func attach(view: ASiOSView) {
        renderView = view
        startTimerIfNeeded()
    }

    func detach(view: ASiOSView) {
        if renderView === view {
            renderView = nil
        }
    }

    func updateRenderDimensions(width: Int, height: Int) {
        renderWidth = width
        renderHeight = height
    }

    func draw(into outputBuffer: UnsafeMutablePointer<n_byte4>?, width: Int, height: Int) {
        shared_session_draw_ios(session, outputBuffer, n_int(Int32(width)), n_int(Int32(height)))
    }

    func cycle() {
        _ = shared_session_cycle_ios(session, n_uint(CFAbsoluteTimeGetCurrent()))
    }

    func newSimulation() {
        shared_session_new(session, n_uint(CFAbsoluteTimeGetCurrent()))
    }

    func previousApe() {
        shared_session_menu(session, NA_MENU_PREVIOUS_APE)
    }

    func nextApe() {
        shared_session_menu(session, NA_MENU_NEXT_APE)
    }

    func mouseReceived(at location: CGPoint, in bounds: CGRect) {
        let simulationLocation = simulationPoint(for: location, in: bounds)
        shared_session_mouseReceived(session,
                                     n_double(Float(simulationLocation.x)),
                                     n_double(Float(simulationLocation.y)),
                                     n_int(NUM_TERRAIN))
    }

    func mouseUp() {
        shared_session_mouseUp(session)
    }

    private func startTimerIfNeeded() {
        guard timer == nil else { return }

        timer = Timer.scheduledTimer(withTimeInterval: 1.0 / 60.0, repeats: true) { [weak self] _ in
            DispatchQueue.main.async {
                self?.renderView?.setNeedsDisplay()
            }
        }
    }

    func simulationPoint(for location: CGPoint, in bounds: CGRect) -> CGPoint {
        guard bounds.width > 0, bounds.height > 0, renderWidth > 0, renderHeight > 0 else {
            return location
        }

        let scaleX = CGFloat(renderWidth) / bounds.width
        let scaleY = CGFloat(renderHeight) / bounds.height
        let rotatedX = CGFloat(renderWidth - 1) - (location.x * scaleX)
        let rotatedY = CGFloat(renderHeight - 1) - (location.y * scaleY)

        return CGPoint(x: min(max(rotatedX, 0), CGFloat(renderWidth - 1)),
                       y: min(max(rotatedY, 0), CGFloat(renderHeight - 1)))
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

    func makeUIView(context: Context) -> ASiOSView {
        let view = ASiOSView(sceneModel: sceneModel)
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
    private let sceneModel: ApeSimulationSceneModel
    private var offscreenBuffer = [UInt32](repeating: 0, count: 2000 * 3000)
    private var drawRef: CGContext?
    private var oldDimensionX: Int = -1
    private var oldDimensionY: Int = -1

    init(sceneModel: ApeSimulationSceneModel, frame: CGRect = .zero) {
        self.sceneModel = sceneModel
        super.init(frame: frame)
    }

    required init?(coder: NSCoder) {
        self.sceneModel = ApeSimulationSceneModel()
        super.init(coder: coder)
        self.sceneModel.attach(view: self)
    }

    func detachFromSceneModel() {
        sceneModel.detach(view: self)
    }

    func simulationPoint(for location: CGPoint) -> CGPoint {
        sceneModel.simulationPoint(for: location, in: bounds)
    }

    override func draw(_ rect: CGRect) {
        let scaleFactor: CGFloat = 1.0
        let dimensionX = Int(rect.width * scaleFactor)
        let dimensionY = Int(rect.height * scaleFactor)

        guard let context = UIGraphicsGetCurrentContext() else { return }

        if drawRef == nil || oldDimensionX != dimensionX || oldDimensionY != dimensionY {
            let colorSpace = CGColorSpaceCreateDeviceRGB()
            drawRef = CGContext(data: &offscreenBuffer,
                                width: dimensionX,
                                height: dimensionY,
                                bitsPerComponent: 8,
                                bytesPerRow: dimensionX * 4,
                                space: colorSpace,
                                bitmapInfo: CGImageAlphaInfo.noneSkipFirst.rawValue | CGBitmapInfo.byteOrder32Big.rawValue)

            oldDimensionX = dimensionX
            oldDimensionY = dimensionY
        }

        sceneModel.updateRenderDimensions(width: dimensionX, height: dimensionY)

        context.saveGState()
        offscreenBuffer.withUnsafeMutableBufferPointer { buffer in
            sceneModel.draw(into: buffer.baseAddress, width: dimensionX, height: dimensionY)
        }

        if let image = drawRef?.makeImage() {
            context.setBlendMode(.copy)
            context.draw(image, in: rect)
        }

        context.restoreGState()
        sceneModel.cycle()
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
            sceneModel.mouseReceived(at: location, in: bounds)
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
