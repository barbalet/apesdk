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
#if BRONZE_SIM
import UniformTypeIdentifiers
#endif
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
enum ApeCatalystWindow {
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

#if targetEnvironment(macCatalyst)
    private var primaryCatalystScene: some Scene {
#if BRONZE_SIM
        WindowGroup("BronzeSim", id: BronzeWorkspacePreview.windowID) {
            BronzeWorkspacePreview(sceneModel: catalystSceneModel,
                                   controlWindowID: ApeCatalystWindow.controlID)
        }
        .defaultSize(width: 1100, height: 700)
#else
        WindowGroup("View", id: ApeCatalystWindow.viewID) {
            CatalystSimulationWindow(sceneModel: catalystSceneModel,
                                     surface: .view,
                                     opensAuxiliaryWindows: true)
        }
        .defaultSize(width: ApeSimulationSurface.view.defaultSize.width,
                     height: ApeSimulationSurface.view.defaultSize.height)
#endif
    }
#endif

    var body: some Scene {
#if targetEnvironment(macCatalyst)
        primaryCatalystScene
        .commands {
#if BRONZE_SIM
            BronzeWorldCommands(sceneModel: catalystSceneModel)
#else
            CommandGroup(replacing: CommandGroupPlacement.newItem) {
                Button("New Simulation") {
                    catalystSceneModel.newSimulation()
                }
                .keyboardShortcut("n", modifiers: [.command])
            }
#endif

#if BRONZE_SIM
            BronzeScenarioCommands(controller: catalystSceneModel.bronzeScenario) { name in
                catalystSceneModel.selectBronzeScenario(name)
            }
            BronzeWorkspaceCommands()
#endif
            CommandMenu("Control") {
#if BRONZE_SIM
                Button(catalystSceneModel.bronzePaused ? "Resume" : "Pause") {
                    _ = catalystSceneModel.menu(n_int(NA_MENU_PAUSE))
                }
                .keyboardShortcut("p", modifiers: [.command])
#else
                Button(isPaused ? "Resume" : "Pause") {
                    isPaused = catalystSceneModel.menu(n_int(NA_MENU_PAUSE)) == 1
                }
                .keyboardShortcut("p", modifiers: [.command])
#endif
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
#if BRONZE_SIM
        WindowGroup("View", id: ApeCatalystWindow.viewID) {
            CatalystSimulationWindow(sceneModel: catalystSceneModel,
                                     surface: .view,
                                     opensAuxiliaryWindows: false)
        }
        .defaultSize(width: ApeSimulationSurface.view.defaultSize.width,
                     height: ApeSimulationSurface.view.defaultSize.height)
#endif
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
#if BRONZE_SIM
            .safeAreaInset(edge: .bottom) {
                if surface == .view {
                    BronzeScenarioPanel(controller: sceneModel.bronzeScenario,
                                        actorSummary: sceneModel.bronzeSelectedActorSummary)
                        .fileExporter(isPresented: $sceneModel.showBronzeSave,
                                      document: sceneModel.bronzeSaveDocument,
                                      contentType: .data, defaultFilename: "World.bronzesave") { result in
                            if case .failure(let error) = result, !BronzeSaveDocument.isCancellation(error) {
                                sceneModel.bronzeScenario.reportFileError(error.localizedDescription)
                            }
                        }
                        .fileImporter(isPresented: $sceneModel.showBronzeOpen,
                                      allowedContentTypes: [.data]) { result in
                            sceneModel.openBronzeSave(result)
                        }
                }
            }
#endif
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
#if BRONZE_SIM
@MainActor
#endif
final class ApeSimulationSceneModel: ObservableObject {
#if BRONZE_SIM
    @Published private(set) var bronzeWorkspaceSnapshot = BronzeWorkspaceSnapshot()
    var bronzeSelectedActorSummary: String { bronzeWorkspaceSnapshot.actorSummary }
#if targetEnvironment(macCatalyst)
    private var bronzeLastDisplayTime: TimeInterval = 0
    private var bronzeLastInspectorTime: TimeInterval = 0
    private var bronzePlaybackCredit = 0.0
    @Published var bronzePlaybackSpeed = 1.0

    func bronzeWorkspaceCenter(selected: Bool) -> BronzeWorkspacePoint {
        startSessionIfNeeded()
        return shared_session_bronze_workspace_center(session, selected ? 1 : 0)
    }

    func drawBronzeWorkspace(into pixels: UnsafeMutablePointer<n_byte4>?, width: Int, height: Int,
                             surface: ApeSimulationSurface, center: BronzeWorkspacePoint,
                             span: Double, bearing: Double, selectedSite: UInt64) {
        shared_session_bronze_workspace_draw(session, pixels, n_int(width), n_int(height),
                                            surface.identifier, center, span, bearing, selectedSite)
    }

    func pickBronzeWorkspaceMap(center: BronzeWorkspacePoint, dx: Double, dy: Double,
                               radius: Double) -> BronzeWorkspaceSite {
        let site = shared_session_bronze_workspace_select(session, center, dx, dy, radius)
        refreshBronzeWorkspaceSnapshot()
        return site
    }

    func stepBronzeWorkspace() {
        if shared_session_bronze_workspace_step(session) != 0 {
            refreshBronzeWorkspaceSnapshot()
        }
    }
    private let bronzeWorkspaceViews = NSHashTable<ASiOSView>.weakObjects()
    private weak var bronzeInputView: ASiOSView?

    func attachBronzeWorkspace(view: ASiOSView) {
        bronzeWorkspaceViews.add(view)
        attach(view: view)
    }

    func bronzeSurfaceDidDraw(_ view: ASiOSView, surface: ApeSimulationSurface) {
        if bronzeWorkspaceViews.allObjects.isEmpty ||
            (surface != .control && bronzeInputView === view) {
            cycle(surface: surface, rotatesForUIKit: false)
        }
    }

    func bronzePointer(at point: CGPoint, surface: ApeSimulationSurface, view: ASiOSView) {
        if bronzeInputView !== view { mouseUp() }
        bronzeInputView = view
        shared_session_mouseReceived(session, n_double(point.x), n_double(point.y), surface.identifier)
    }

    func endBronzePointer(in view: ASiOSView) {
        guard bronzeInputView === view else { return }
        mouseUp()
        bronzeInputView = nil
    }

    func bronzeNavigate(_ key: n_int, surface: ApeSimulationSurface) {
        shared_session_bronze_surface_key(session, surface.identifier, key)
        refreshBronzeWorkspaceSnapshot()
    }
#endif

    private func refreshBronzeWorkspaceSnapshot() {
        var buffer = [CChar](repeating: 0, count: 4096)
        let summary = buffer.withUnsafeMutableBufferPointer {
            shared_session_bronze_actor_summary(session, $0.baseAddress, n_uint($0.count))
            return String(cString: $0.baseAddress!)
        }
        let actor = BronzeWorkspaceActor(shared_session_bronze_inspector_actor(session))
        let snapshot = BronzeWorkspaceSnapshot(scenario: bronzeScenario.activeName,
                                               actor: actor,
                                               actorSummary: summary, paused: bronzePaused,
                                               day: UInt64(shared_session_bronze_workspace_day(session)))
        if snapshot != bronzeWorkspaceSnapshot { bronzeWorkspaceSnapshot = snapshot }
    }
    let bronzeScenario = BronzeScenarioController()
    @Published var showBronzeOpen = false
    @Published private(set) var bronzePaused = false
    private var bronzeNativeImport = false

    func chooseBronzeOpen(nativeOnly: Bool) {
        bronzeNativeImport = nativeOnly
        showBronzeOpen = true
    }
    @Published var showBronzeSave = false
    private(set) var bronzeSaveDocument: BronzeSaveDocument?

    func prepareBronzeSave() {
        bronzeSaveDocument = makeBronzeSaveDocument(reportError: bronzeScenario.reportFileError)
        showBronzeSave = bronzeSaveDocument != nil
    }

    func makeBronzeSaveDocument(reportError: (String) -> Void) -> BronzeSaveDocument? {
        startSessionIfNeeded()
        guard bronzeScenario.digest.count == 32,
              let file = bronzeScenario.digest.withUnsafeBufferPointer({
                  shared_session_bronze_save(session, $0.baseAddress)
              }) else {
            reportError("The Bronze world could not be saved. Wait for the world to finish loading and try again.")
            return nil
        }
        let document = BronzeSaveDocument(data: Data(bytes: file.pointee.data,
                                                            count: Int(file.pointee.location)))
        var ownedFile: UnsafeMutablePointer<n_file>? = file
        io_file_free(&ownedFile)
        return document
    }

    func openBronzeSave(_ result: Result<URL, Error>) {
        openBronzeSave(result, nativeOnly: bronzeNativeImport, reportError: bronzeScenario.reportFileError)
    }

    func openBronzeSave(_ result: Result<URL, Error>, nativeOnly: Bool, reportError: (String) -> Void) {
        startSessionIfNeeded()
        do {
            let url = try result.get()
            let scoped = url.startAccessingSecurityScopedResource()
            defer { if scoped { url.stopAccessingSecurityScopedResource() } }
            let handle = try FileHandle(forReadingFrom: url)
            defer { try? handle.close() }
            let size = try handle.seekToEnd()
            guard size <= 64 * 1024 * 1024 else {
                throw CocoaError(.fileReadCorruptFile)
            }
            try handle.seek(toOffset: 0)
            if nativeOnly {
                let signature = try handle.read(upToCount: 7) ?? Data()
                if signature == Data("BRZSAVE".utf8) {
                    reportError("Use Open Bronze World to resume this save.")
                    return
                }
                let imported = url.path.withCString { shared_session_openFileName(session, $0, 0) }
                bronzePaused = shared_session_pause_state(session) != 0
                if imported == 0 {
                    reportError("The native world could not be imported.")
                }
                refreshBronzeWorkspaceSnapshot()
                return
            }
            guard bronzeScenario.digest.count == 32 else {
                throw CocoaError(.fileReadCorruptFile)
            }
            let bytes = try handle.read(upToCount: Int(size)) ?? Data()
            let restored = bronzeScenario.digest.withUnsafeBufferPointer { digest in
                bytes.withUnsafeBytes { raw in
                    shared_session_bronze_open(session, raw.bindMemory(to: UInt8.self).baseAddress,
                                               n_uint(bytes.count), digest.baseAddress)
                }
            }
            if restored != 0 {
                bronzePaused = true
                refreshBronzeWorkspaceSnapshot()
            } else {
                reportError("The Bronze save is invalid or belongs to a different scenario. Load the matching scenario and try again.")
            }
        } catch where BronzeSaveDocument.isCancellation(error) {
            return
        } catch {
            reportError(error.localizedDescription)
        }
    }

    @discardableResult
    func selectBronzeScenario(_ name: String, presentsDiagnostics: Bool = true) -> Bool {
        bronzeScenario.load(name, presentsDiagnostics: presentsDiagnostics) { [weak self] _ in
            self?.newSimulation()
        }
    }
#endif
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
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        bronzeWorkspaceViews.remove(view)
#endif
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

#if BRONZE_SIM && targetEnvironment(macCatalyst)
    private func advanceBronzeWorkspace(now: TimeInterval) {
        startSessionIfNeeded()
        let ticks = n_uint(CFAbsoluteTimeGetCurrent())
        if bronzePaused {
            bronzePlaybackCredit = 0
            _ = shared_session_cycle(session, ticks, ApeSimulationSurface.control.identifier)
        } else {
            bronzePlaybackCredit += bronzePlaybackSpeed
            while bronzePlaybackCredit >= 1 {
                _ = shared_session_cycle(session, ticks, ApeSimulationSurface.control.identifier)
                bronzePlaybackCredit -= 1
            }
        }
        if now - bronzeLastInspectorTime >= 0.1 {
            bronzeLastInspectorTime = now
            refreshBronzeWorkspaceSnapshot()
        }
    }
#endif

    func cycle(surface: ApeSimulationSurface, rotatesForUIKit: Bool) {
        startSessionIfNeeded()
        let ticks = n_uint(CFAbsoluteTimeGetCurrent())
        if rotatesForUIKit {
            _ = shared_session_cycle_ios(session, ticks)
        } else {
            _ = shared_session_cycle(session, ticks, surface.identifier)
        }
#if BRONZE_SIM
        refreshBronzeWorkspaceSnapshot()
#endif
    }

    func newSimulation() {
        startSessionIfNeeded()
#if BRONZE_SIM
        shared_session_bronze_land_policy(session, bronzeScenario.landPolicy)
        shared_session_bronze_professions(session, bronzeScenario.professions)
        shared_session_bronze_program(session, bronzeScenario.program)
        shared_session_new(session, bronzeScenario.seed)
        bronzePaused = shared_session_pause_state(session) != 0
        refreshBronzeWorkspaceSnapshot()
#else
        shared_session_new(session, n_uint(CFAbsoluteTimeGetCurrent()))
#endif
    }

    func menu(_ menuValue: n_int) -> n_int {
        startSessionIfNeeded()
        let result = shared_session_menu(session, menuValue)
#if BRONZE_SIM
        if menuValue == n_int(NA_MENU_PAUSE) { bronzePaused = result != 0 }
        refreshBronzeWorkspaceSnapshot()
#endif
        return result
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
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        if let owner = bronzeInputView { endBronzePointer(in: owner) }
#endif
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

#if BRONZE_SIM
    private nonisolated static var isRunningUnderUnitTests: Bool {
        let environment = ProcessInfo.processInfo.environment
        return environment["XCTestConfigurationFilePath"] != nil ||
               environment["XCTestBundlePath"] != nil
    }

#else
    private static var isRunningUnderUnitTests: Bool {
        let environment = ProcessInfo.processInfo.environment
        return environment["XCTestConfigurationFilePath"] != nil ||
               environment["XCTestBundlePath"] != nil
    }

#endif

    private func startSessionIfNeeded() {
        guard startsSessionAutomatically, hasStartedSession == false else {
            return
        }

#if BRONZE_SIM
        shared_session_bronze_land_policy(session, bronzeScenario.landPolicy)
        shared_session_bronze_professions(session, bronzeScenario.professions)
        shared_session_bronze_program(session, bronzeScenario.program)
        shared_session_init(session, n_int(NUM_CONTROL), bronzeScenario.seed)
#else
        shared_session_init(session, n_int(NUM_CONTROL), n_uint(CFAbsoluteTimeGetCurrent()))
#endif
        hasStartedSession = true
#if BRONZE_SIM
        refreshBronzeWorkspaceSnapshot()
#endif
    }

    private func startTimerIfNeeded() {
        guard timer == nil else { return }

#if BRONZE_SIM && targetEnvironment(macCatalyst)
        let driver = Timer(timeInterval: 1.0 / 60.0, repeats: true) { [weak self] _ in
            MainActor.assumeIsolated {
                guard let self else { return }
                let now = ProcessInfo.processInfo.systemUptime
                let workspaceOpen = !self.bronzeWorkspaceViews.allObjects.isEmpty
                if workspaceOpen {
                    self.advanceBronzeWorkspace(now: now)
                }
                let displayInterval = workspaceOpen ? 1.0 / 20.0 : 1.0 / 32.0
                if now - self.bronzeLastDisplayTime >= displayInterval {
                    self.bronzeLastDisplayTime = now
                    self.renderViews.allObjects.forEach { view in
                        if let window = view.window, !window.isHidden, !view.isHidden {
                            view.setNeedsDisplay()
                        }
                    }
                }
            }
        }
        timer = driver
        RunLoop.main.add(driver, forMode: .common)
#else
        timer = Timer.scheduledTimer(withTimeInterval: 1.0 / 60.0, repeats: true) { [weak self] _ in
            DispatchQueue.main.async {
                self?.renderViews.allObjects.forEach { view in
                    view.setNeedsDisplay()
                }
            }
        }
#endif
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
#if BRONZE_SIM && targetEnvironment(macCatalyst)
    var isBronzeWorkspaceSurface = false
    var onBronzeSurfaceFocus: (() -> Void)?
    var bronzeMapCenter = BronzeWorkspacePoint(x: 0, y: 0)
    var bronzeMapSpan = Double(shared_bronze_workspace_map_dimension()) / 2
    var bronzeTerrainBearing = 0.0
    var bronzeSelectedSite: UInt64 = 0
    var onBronzeWorkspaceSite: ((BronzeWorkspaceSite) -> Void)?
    var onBronzeWorkspaceDrag: ((Double, Double) -> Void)?
    private var bronzeWorkspaceTouchStart: CGPoint?
    private var bronzePointerLocation: CGPoint?
    private var bronzePointerEnded = false

    func enableBronzeMapScrolling() {
        let scroll = UIPanGestureRecognizer(target: self, action: #selector(scrollBronzeMap(_:)))
        scroll.allowedScrollTypesMask = [.continuous, .discrete]
        scroll.allowedTouchTypes = []
        addGestureRecognizer(scroll)
    }

    @objc private func scrollBronzeMap(_ scroll: UIPanGestureRecognizer) {
        guard scroll.state == .changed || scroll.state == .ended, bounds.width > 0 else { return }
        let translation = scroll.translation(in: self)
        let cellsPerPoint = bronzeMapSpan / Double(bounds.width)
        let x = (Double(translation.x) * cellsPerPoint).rounded(.towardZero)
        let y = (Double(translation.y) * cellsPerPoint).rounded(.towardZero)
        guard x != 0 || y != 0 else { return }
        scroll.setTranslation(CGPoint(x: translation.x - x / cellsPerPoint,
                                      y: translation.y - y / cellsPerPoint), in: self)
        onBronzeWorkspaceDrag?(x / bronzeMapSpan, y / bronzeMapSpan)
    }
#endif
    private var offscreenBuffer = [UInt32](repeating: 0, count: 4096 * 512)
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

#if BRONZE_SIM
        guard !isBronzeWorkspaceSurface else { return }
#endif
        windowScene.title = surface.title
        windowScene.sizeRestrictions?.minimumSize = surface.minimumSize
    }
#endif

    func detachFromSceneModel() {
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        sceneModel.endBronzePointer(in: self)
#endif
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
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        if isBronzeWorkspaceSurface {
            let scale = min(1, 384 / max(1, bounds.width, bounds.height))
            return (max(1, Int(bounds.width * scale)), max(1, Int(bounds.height * scale)))
        }
#endif
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

#if BRONZE_SIM && targetEnvironment(macCatalyst)
        if isBronzeWorkspaceSurface {
            let bytesPerRow = dimensionX * MemoryLayout<n_byte4>.stride
            var frame = Data(count: bytesPerRow * dimensionY)
            frame.withUnsafeMutableBytes { bytes in
                sceneModel.drawBronzeWorkspace(into: bytes.bindMemory(to: n_byte4.self).baseAddress,
                                              width: dimensionX, height: dimensionY,
                                              surface: surface, center: bronzeMapCenter,
                                              span: bronzeMapSpan, bearing: bronzeTerrainBearing,
                                              selectedSite: bronzeSelectedSite)
            }
            guard let provider = CGDataProvider(data: frame as CFData),
                  let image = CGImage(width: dimensionX, height: dimensionY, bitsPerComponent: 8,
                                      bitsPerPixel: 32, bytesPerRow: bytesPerRow,
                                      space: CGColorSpaceCreateDeviceRGB(),
                                      bitmapInfo: [.byteOrder32Big,
                                                   CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedLast.rawValue)],
                                      provider: provider, decode: nil, shouldInterpolate: true,
                                      intent: .defaultIntent) else { return }
            // Render rows run from bottom to top. UIKit images start at the top.
            UIImage(cgImage: image, scale: 1, orientation: .downMirrored)
                .draw(in: displayRect, blendMode: .copy, alpha: 1)
            return
        }
#endif

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
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        if !rotatesForUIKit {
            if let location = bronzePointerLocation {
                sceneModel.bronzePointer(at: simulationPoint(for: location), surface: surface, view: self)
                bronzePointerLocation = nil
            }
            sceneModel.bronzeSurfaceDidDraw(self, surface: surface)
            if bronzePointerEnded {
                sceneModel.endBronzePointer(in: self)
                bronzePointerEnded = false
            }
            return // The Bronze timer invalidates Catalyst surfaces.
        } else {
            sceneModel.cycle(surface: surface, rotatesForUIKit: true)
        }
#else
        sceneModel.cycle(surface: surface, rotatesForUIKit: rotatesForUIKit)
#endif
        DispatchQueue.main.async { [weak self] in
            self?.setNeedsDisplay()
        }
    }

    // MARK: - Touch Handling

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        onBronzeSurfaceFocus?()
        if isBronzeWorkspaceSurface {
            bronzeWorkspaceTouchStart = touches.first?.location(in: self)
            return
        }
#endif
        touchesMoved(touches, with: event)
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        if isBronzeWorkspaceSurface { return }
#endif
        guard let allTouches = event?.touches(for: self) else { return }

        for touch in allTouches {
            let location = touch.location(in: self)
#if BRONZE_SIM && targetEnvironment(macCatalyst)
            if !rotatesForUIKit && surface != .control {
                if !isBronzeWorkspaceSurface || displayRect(renderDimensions: renderDimensions(for: bounds), in: bounds).contains(location) {
                    bronzePointerLocation = location
                    setNeedsDisplay()
                }
                continue
            }
#endif
            print("Touch moved to: \(location)")
            sceneModel.mouseReceived(at: location,
                                     in: bounds,
                                     surface: surface,
                                     rotatesForUIKit: rotatesForUIKit)
        }
    }

    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        bronzeWorkspaceTouchStart = nil
        bronzePointerLocation = nil
        bronzePointerEnded = false
        sceneModel.endBronzePointer(in: self)
#endif
        guard let allTouches = event?.touches(for: self) else { return }

        for touch in allTouches {
            let location = touch.location(in: self)
            print("Touch cancelled at: \(location)")
        }
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
#if BRONZE_SIM && targetEnvironment(macCatalyst)
        if isBronzeWorkspaceSurface {
            defer { bronzeWorkspaceTouchStart = nil }
            guard let start = bronzeWorkspaceTouchStart, let end = touches.first?.location(in: self),
                  bounds.width > 0 else { return }
            let dx = end.x - start.x
            let dy = end.y - start.y
            if abs(dx) + abs(dy) > 5 {
                onBronzeWorkspaceDrag?(dx / bounds.width, dy / bounds.width)
            } else if surface == .view {
                let units = bronzeMapSpan / Double(bounds.width)
                let site = sceneModel.pickBronzeWorkspaceMap(center: bronzeMapCenter,
                    dx: Double(end.x - bounds.midX) * units,
                    dy: Double(bounds.midY - end.y) * units, radius: min(32, max(0.8, 12 * units)))
                onBronzeWorkspaceSite?(site)
            }
            return
        }
        if !rotatesForUIKit && surface != .control {
            bronzePointerEnded = true
            setNeedsDisplay()
            return
        }
#endif
        sceneModel.mouseUp()
    }

    // MARK: - Motion Events
    override func motionEnded(_ motion: UIEvent.EventSubtype, with event: UIEvent?) {
        if motion == .motionShake {
            sceneModel.nextApe()
        }
    }
}
