import SwiftUI
import CryptoKit
import UniformTypeIdentifiers

// This file belongs only to bronze-mac; shared shell call sites are #if BRONZE_SIM.
@MainActor
final class BronzeScenarioController: ObservableObject {
    static let names = ["first_slice", "vertical_slice", "bronze_age_full"]
    private let bundle: Bundle
    private let storage = bronze_scenario_create()
    @Published private(set) var activeName = "No scenario loaded"
    @Published private(set) var diagnostics = ""
    @Published private(set) var lastAttemptFailed = false
    @Published var showDiagnostics = false
    private(set) var digest: [UInt8] = []
    func reportFileError(_ message: String) {
        diagnostics = message
        showDiagnostics = true
    }
    var landPolicy: BronzeLandPolicy { bronze_scenario_land_policy(storage) }
    var professions: UnsafePointer<BronzeProfessionPolicy>? { bronze_scenario_professions(storage) }
    var program: OpaquePointer? { bronze_scenario_program(storage) }
    var seed: UInt { UInt(bronze_scenario_seed(storage)) }

    init(bundle: Bundle = .main) { self.bundle = bundle; _ = load("bronze_age_full") }
    deinit { bronze_scenario_destroy(storage) }

    @discardableResult
    func load(_ name: String, presentsDiagnostics: Bool = true, onCommit: ((UInt) -> Void)? = nil) -> Bool {
        let filename = name + ".bronze"
        let path = bundle.url(forResource: name, withExtension: "bronze")?.path
        let accepted: Bool = filename.withCString { label in
            guard let path else {
                return bronze_scenario_load(storage, nil, label) != 0
            }
            return path.withCString { bronze_scenario_load(storage, $0, label) != 0 }
        }
        diagnostics = String(cString: bronze_scenario_diagnostics(storage))
        if accepted {
            digest = path.flatMap { try? Data(contentsOf: URL(fileURLWithPath: $0)) }
                .map { Array(SHA256.hash(data: $0)) } ?? []
            activeName = String(cString: bronze_scenario_name(storage))
            showDiagnostics = false
            onCommit?(seed)
        }
        lastAttemptFailed = !accepted
        if !accepted && presentsDiagnostics { showDiagnostics = true }
        return accepted
    }
}

struct BronzeScenarioCommands: Commands {
    @ObservedObject var controller: BronzeScenarioController
    let select: (String) -> Void
    var body: some Commands {
        CommandMenu("Bronze Scenario") {
            ForEach(BronzeScenarioController.names, id: \.self) { name in
                Button("Load \(name).bronze — Reset World") { select(name) }
            }
            Divider()
            Button("Scenario Details and Diagnostics…") { controller.showDiagnostics = true }
        }
    }
}

struct BronzeScenarioPanel: View {
    @ObservedObject var controller: BronzeScenarioController
    var actorSummary = "No registered being selected"
    var body: some View {
        HStack {
            VStack(alignment: .leading) {
                Text(controller.activeName).font(.caption.bold())
                Text(controller.lastAttemptFailed ? "Load failed; active scenario unchanged." :
                     "Profession tasks run on the native simulation clock.")
                    .font(.caption)
                ScrollView {
                    Text(actorSummary)
                        .font(.caption)
                        .textSelection(.enabled)
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .accessibilityIdentifier("BronzeActorIdentity")
                }
                .frame(maxHeight: 88)
                Text("Map legend · Roof township · Cross copper · Dot tin · Rows field · Tree woodland")
                    .font(.caption2)
            }
            Spacer()
            Button("Details") { controller.showDiagnostics = true }
        }
        .padding(8)
        .background(.regularMaterial)
        .accessibilityIdentifier("BronzeScenarioStatus")
        .sheet(isPresented: $controller.showDiagnostics) {
            BronzeScenarioDetails(controller: controller) { controller.showDiagnostics = false }
        }
    }
}


private struct BronzeScenarioDetails: View {
    @ObservedObject var controller: BronzeScenarioController
    let close: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Active: \(controller.activeName)").font(.headline)
            if controller.lastAttemptFailed {
                Text("The scenario could not load. The current world is still active.")
                    .foregroundStyle(.secondary)
            }
            ScrollView {
                Text(controller.diagnostics.isEmpty ? "No diagnostics." : controller.diagnostics)
                    .font(.system(.body, design: .monospaced))
                    .textSelection(.enabled).frame(maxWidth: .infinity, alignment: .leading)
            }
            Button("Close", action: close)
        }
        .padding().frame(minWidth: 520, minHeight: 320)
        .accessibilityIdentifier("BronzeScenarioDiagnostics")
    }
}

#if BRONZE_SIM
struct BronzeWorkspaceActor: Equatable {
    let id: UInt64
    let workplaceID: UInt64
    let remainingMinutes: UInt64
    let vocation: String
    let task: String
    let status: String
    let action: String
    let cargo: String
    let items: String
    let energy: String
    let energyFraction: Double
    let hunger: UInt32
    let fatigue: UInt32
    let exchange: String
    let site: String

    init?(_ actor: BronzeInspectorActor) {
        guard actor.actor_id != 0 else { return nil }
        id = actor.actor_id
        workplaceID = actor.workplace_id
        remainingMinutes = actor.remaining_minutes
        vocation = Self.decode(actor.vocation)
        task = Self.decode(actor.task)
        status = Self.decode(actor.status)
        action = Self.decode(actor.action)
        cargo = Self.decode(actor.cargo)
        items = Self.decode(actor.items)
        energy = "\(actor.energy) / \(actor.energy_full)"
        energyFraction = actor.energy_full > 0 ? min(1, max(0, Double(actor.energy) / Double(actor.energy_full))) : 0
        hunger = actor.hunger
        fatigue = actor.fatigue
        let outcome = Self.decode(actor.exchange)
        exchange = actor.exchange_partner_id == 0 ? outcome : "\(outcome) · Partner #\(actor.exchange_partner_id)"
        site = Self.decode(actor.site)
    }

    private static func decode<T>(_ characters: T) -> String {
        withUnsafeBytes(of: characters) {
            String(decoding: $0.prefix { $0 != 0 }, as: UTF8.self)
        }
    }
}

struct BronzeWorkspaceSnapshot: Equatable {
    let scenario: String
    let actor: BronzeWorkspaceActor?
    let actorSummary: String
    let paused: Bool
    let day: UInt64

    init(scenario: String = "No scenario loaded", actor: BronzeWorkspaceActor? = nil,
         actorSummary: String = "No registered being selected", paused: Bool = false, day: UInt64 = 0) {
        self.scenario = scenario
        self.actor = actor
        self.actorSummary = actorSummary
        self.paused = paused
        self.day = day
    }
}
#endif

#if BRONZE_SIM && targetEnvironment(macCatalyst)
struct BronzeWorldCommands: Commands {
    let sceneModel: ApeSimulationSceneModel
    @FocusedValue(\.bronzeWorkspaceFiles) private var workspaceFiles


    var body: some Commands {
        CommandGroup(replacing: .newItem) {
            Button("New Simulation") { sceneModel.newSimulation() }
                .keyboardShortcut("n", modifiers: [.command])
            Button("Save Bronze World…") {
                if let workspaceFiles { workspaceFiles.save(); return }
                sceneModel.prepareBronzeSave()
            }
            .keyboardShortcut("s", modifiers: [.command])
            Button("Open Bronze World…") {
                if let workspaceFiles { workspaceFiles.open(); return }
                sceneModel.chooseBronzeOpen(nativeOnly: false)
            }
            .keyboardShortcut("o", modifiers: [.command])
            Button("Import Native World…") {
                if let workspaceFiles { workspaceFiles.importNative(); return }
                sceneModel.chooseBronzeOpen(nativeOnly: true)
            }
        }
    }
}
#endif

#if BRONZE_SIM && targetEnvironment(macCatalyst)
struct BronzeWorkspaceCommands: Commands {
    @Environment(\.openWindow) private var openWindow

    var body: some Commands {
        CommandGroup(after: .windowArrangement) {
            Button("Open Bronze Workspace") {
                openWindow(id: BronzeWorkspacePreview.windowID)
            }
            .keyboardShortcut("u", modifiers: [.command, .shift])
            Button("Open Separate Simulation Windows") {
                openWindow(id: ApeCatalystWindow.viewID)
                openWindow(id: ApeCatalystWindow.terrainID)
                openWindow(id: ApeCatalystWindow.controlID)
            }
        }
    }
}

struct BronzeWorkspaceFileActions {
    let save: () -> Void
    let open: () -> Void
    let importNative: () -> Void
}

private struct BronzeWorkspaceFilesKey: FocusedValueKey {
    typealias Value = BronzeWorkspaceFileActions
}

extension FocusedValues {
    var bronzeWorkspaceFiles: BronzeWorkspaceFileActions? {
        get { self[BronzeWorkspaceFilesKey.self] }
        set { self[BronzeWorkspaceFilesKey.self] = newValue }
    }
}

private let bronzeWorkspaceAccent = Color(red: 0.92, green: 0.65, blue: 0.36)

struct BronzeWorkspacePreview: View {
    static let windowID = "bronze-workspace-preview"

    @Environment(\.openWindow) private var openWindow
    @ObservedObject var sceneModel: ApeSimulationSceneModel
    let controlWindowID: String
    @State private var pendingScenario = ""
    @State private var confirmsScenarioReset = false
    @State private var showsDiagnostics = false
    @AppStorage("bronze.workspace.showInspector") private var showsInspector = true
    @AppStorage("bronze.workspace.showTerrain") private var showsTerrain = true
    @State private var saveDocument: BronzeSaveDocument?
    @State private var showsExporter = false
    @State private var showsImporter = false
    @State private var importingNative = false
    @State private var fileError = ""
    @State private var showsFileError = false
    @FocusState private var dividerFocused: Bool
    private static let defaultWideShare = 0.53
    private static let defaultCompactShare = 0.50
    @AppStorage("bronze.workspace.wideShare") private var wideShare = defaultWideShare
    @AppStorage("bronze.workspace.compactShare") private var compactShare = defaultCompactShare
    @State private var mapCenter: BronzeWorkspacePoint?
    @State private var mapSpan = Double(shared_bronze_workspace_map_dimension()) / 2
    @State private var selectedSite = BronzeWorkspaceSite(id: 0, kind: 0)

    private var siteCaption: String? {
        if selectedSite.id != 0 {
            let kinds = ["Township", "Field", "Copper mine", "Tin mine", "Woodland"]
            if kinds.indices.contains(Int(selectedSite.kind)) {
                return "\(kinds[Int(selectedSite.kind)]) #\(selectedSite.id)"
            }
        }
        return snapshot.actor?.site
    }
    @State private var terrainBearing = 0.7
    @GestureState private var dividerOffset: CGFloat = 0
    @FocusState private var focusedSurface: ApeSimulationSurface?

    private var snapshot: BronzeWorkspaceSnapshot { sceneModel.bronzeWorkspaceSnapshot }

    var body: some View {
        VStack(alignment: .leading, spacing: 5) {
            ViewThatFits(in: .horizontal) {
                toolbar(compact: false).fixedSize(horizontal: true, vertical: false)
                    .frame(maxWidth: .infinity, alignment: .leading)
                toolbar(compact: true)
            }
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
                .background(.white.opacity(0.045), in: RoundedRectangle(cornerRadius: 5))
            GeometryReader { geometry in
                let wide = geometry.size.width >= 960
                let layout = wide ? AnyLayout(HStackLayout(spacing: 0)) : AnyLayout(VStackLayout(spacing: 0))
                let available = max(1, (wide ? geometry.size.width : geometry.size.height) - 12)
                let share = bounded(CGFloat(wide ? wideShare : compactShare))
                let mapSize = available * bounded(share + dividerOffset / available)
                layout {
                    VStack(spacing: 0) {
                        panel("Overhead Map", surface: .view)
                            .overlay(alignment: .bottomLeading) {
                                BronzeWorkspaceLegend(site: siteCaption)
                                    .padding(7)
                                    .allowsHitTesting(false)
                            }
                        mapFooter(width: wide ? mapSize : geometry.size.width)
                    }
                    .background(.white.opacity(0.045), in: RoundedRectangle(cornerRadius: 5))
                    .frame(width: wide ? mapSize : nil, height: wide ? nil : mapSize)
                    divider(wide: wide, available: available, share: share)
                    VStack(spacing: 8) {
                        if showsTerrain {
                            panel("Terrain", surface: .terrain)
                        } else {
                            Spacer(minLength: 0)
                        }
                        if showsInspector {
                            BronzeWorkspaceInspector(actor: snapshot.actor)
                                .frame(height: showsTerrain ? max(wide ? 175 : 104, geometry.size.height * (wide ? 0.40 : 0.22)) : nil)
                        }
                    }
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                }
            }
        }
        .onChange(of: showsTerrain) { _, visible in
            if !visible && focusedSurface == .terrain { focusedSurface = .view }
        }
        .onChange(of: snapshot.scenario) { _, _ in
            selectedSite = BronzeWorkspaceSite(id: 0, kind: 0)
            mapCenter = sceneModel.bronzeWorkspaceCenter(selected: false)
        }
        .onAppear {
            if mapCenter == nil { mapCenter = sceneModel.bronzeWorkspaceCenter(selected: false) }
        }
        .padding(5)
        .frame(minWidth: 520, minHeight: 480)
        .background(Color(red: 0.09, green: 0.10, blue: 0.11))
        .environment(\.colorScheme, .dark)
        .tint(bronzeWorkspaceAccent)
        .accessibilityIdentifier("BronzeWorkspacePreview")
        .focusedSceneValue(\.bronzeWorkspaceFiles, BronzeWorkspaceFileActions(
            save: saveWorld, open: { openWorld(nativeOnly: false) },
            importNative: { openWorld(nativeOnly: true) }
        ))
        .fileExporter(isPresented: $showsExporter, document: saveDocument,
                      contentType: .data, defaultFilename: "World.bronzesave") { result in
            saveDocument = nil
            if case .failure(let error) = result, !BronzeSaveDocument.isCancellation(error) {
                reportFileError(error.localizedDescription)
            }
        }
        .fileImporter(isPresented: $showsImporter, allowedContentTypes: [.data]) { result in
            sceneModel.openBronzeSave(result, nativeOnly: importingNative, reportError: reportFileError)
        }
        .alert("Bronze World File", isPresented: $showsFileError) {
            Button("OK", role: .cancel) {}
        } message: {
            Text(fileError)
        }
        .alert("Reset world?", isPresented: $confirmsScenarioReset, presenting: pendingScenario) { name in
            Button("Load and Reset", role: .destructive) {
                if !sceneModel.selectBronzeScenario(name, presentsDiagnostics: false) {
                    showsDiagnostics = true
                }
            }
            Button("Cancel", role: .cancel) {}
        } message: { name in
            Text("Loading \(name).bronze replaces the current world. Save it first if you want to return to it.")
        }
        .sheet(isPresented: $showsDiagnostics) {
            BronzeScenarioDetails(controller: sceneModel.bronzeScenario) { showsDiagnostics = false }
        }
    }

    private func saveWorld() {
        saveDocument = sceneModel.makeBronzeSaveDocument(reportError: reportFileError)
        showsExporter = saveDocument != nil
    }

    private func openWorld(nativeOnly: Bool) {
        importingNative = nativeOnly
        showsImporter = true
    }

    private func reportFileError(_ message: String) {
        fileError = message
        showsFileError = true
    }

    private func toolbar(compact: Bool) -> some View {
        HStack(spacing: compact ? 4 : 8) {
            if !compact { Text("BronzeSim").font(.headline) }
            Button(snapshot.paused ? "Resume" : "Pause",
                   systemImage: snapshot.paused ? "play.fill" : "pause.fill") {
                _ = sceneModel.menu(n_int(NA_MENU_PAUSE))
            }
            .labelStyle(BronzeToolbarLabelStyle(compact: compact))
            .accessibilityLabel(snapshot.paused ? "Resume" : "Pause")
            Button("Step", systemImage: "forward.end") { sceneModel.stepBronzeWorkspace() }
                .labelStyle(BronzeToolbarLabelStyle(compact: compact))
                .accessibilityLabel("Step")
                .disabled(!snapshot.paused)
            Button("Previous Ape", systemImage: "chevron.left") {
                _ = sceneModel.menu(n_int(NA_MENU_PREVIOUS_APE))
            }
            .labelStyle(.iconOnly)
            Button("Next Ape", systemImage: "chevron.right") {
                _ = sceneModel.menu(n_int(NA_MENU_NEXT_APE))
            }
            .labelStyle(.iconOnly)
            Menu {
                ForEach(BronzeScenarioController.names, id: \.self) { name in
                    Button("Load \(name).bronze…") {
                        pendingScenario = name
                        confirmsScenarioReset = true
                    }
                }
                Divider()
                Button("Scenario Details and Diagnostics…") { showsDiagnostics = true }
            } label: {
                Text(snapshot.scenario.replacingOccurrences(of: ".bronze", with: "")
                    .replacingOccurrences(of: "_", with: " ").capitalized).lineLimit(1)
            }
            .accessibilityLabel("Scenario")
            .accessibilityValue(snapshot.scenario)
            Button("Open Control Window", systemImage: "person.text.rectangle") {
                openWindow(id: controlWindowID)
            }
            .labelStyle(.iconOnly)
            .help("Open the native Control window for the selected being")
            Spacer(minLength: 4)
            if !compact {
                Button("Open", systemImage: "folder") { openWorld(nativeOnly: false) }
                    .accessibilityLabel("Open Bronze World")
                Button("Save", systemImage: "square.and.arrow.down", action: saveWorld)
                    .accessibilityLabel("Save Bronze World")
            }
            fileMenu
            Menu {
                Toggle("Show Terrain", isOn: $showsTerrain)
                Toggle("Show Inspector", isOn: $showsInspector)
                Divider()
                Button("Reset Workspace Layout", action: resetLayout)
            } label: {
                Label("Layout", systemImage: "rectangle.split.2x1")
                    .labelStyle(BronzeToolbarLabelStyle(compact: compact))
            }
            .accessibilityLabel("Layout")
        }
    }

    private func mapFooter(width: CGFloat) -> some View {
        let scaleWidth = min(56, max(1, width * 0.18))
        let cells = (mapSpan * Double(scaleWidth / max(1, width)))
            .formatted(.number.precision(.fractionLength(0...1)))
        return HStack(spacing: 8) {
            Menu {
                ForEach([0.5, 1, 2], id: \.self) { speed in
                    Button {
                        sceneModel.bronzePlaybackSpeed = speed
                    } label: {
                        if sceneModel.bronzePlaybackSpeed == speed {
                            Label("\(speed.formatted())×", systemImage: "checkmark")
                        } else {
                            Text("\(speed.formatted())×")
                        }
                    }
                }
            } label: {
                Label("\(sceneModel.bronzePlaybackSpeed.formatted())×", systemImage: "forward.fill")
            }
            .accessibilityLabel("Playback speed")
            .accessibilityValue("\(sceneModel.bronzePlaybackSpeed.formatted()) times")
            Text("Day \(snapshot.day % 364 + 1), Year \(snapshot.day / 364 + 1)")
                .monospacedDigit().lineLimit(1)
            Spacer(minLength: 0)
            VStack(spacing: 2) {
                Rectangle().fill(.secondary).frame(height: 1)
                Text("\(cells) cells").lineLimit(1)
            }
            .frame(width: scaleWidth)
            .accessibilityLabel("Map scale, \(cells) cells")
            Button("Recenter on selected actor", systemImage: "scope") {
                selectedSite = BronzeWorkspaceSite(id: 0, kind: 0)
                mapCenter = sceneModel.bronzeWorkspaceCenter(selected: true)
            }
            Button("Zoom out", systemImage: "minus") { mapSpan = min(Double(shared_bronze_workspace_map_dimension()), mapSpan * 2) }
                .disabled(mapSpan >= Double(shared_bronze_workspace_map_dimension()))
            Button("Zoom in", systemImage: "plus") { mapSpan = max(16, mapSpan / 2) }
                .disabled(mapSpan <= 16)
        }
        .labelStyle(.iconOnly)
        .font(.caption)
        .buttonStyle(.borderless)
        .padding(6)
    }

    private func moveCamera(_ surface: ApeSimulationSurface, dx: Double, dy: Double) {
        if surface == .terrain {
            terrainBearing = (terrainBearing - dx * 4).truncatingRemainder(dividingBy: 2 * .pi)
        } else {
            let center = mapCenter ?? sceneModel.bronzeWorkspaceCenter(selected: false)
            mapCenter = BronzeWorkspacePoint(x: center.x - n_int((dx * mapSpan).rounded()),
                                            y: center.y + n_int((dy * mapSpan).rounded()))
        }
    }

    private func resetLayout() {
        wideShare = Self.defaultWideShare
        compactShare = Self.defaultCompactShare
        showsTerrain = true
        showsInspector = true
    }

    private var fileMenu: some View {
        Menu {
            Button("Open Bronze World…") { openWorld(nativeOnly: false) }
            Button("Save Bronze World…", action: saveWorld)
            Divider()
            Button("Import Native World…") { openWorld(nativeOnly: true) }
        } label: {
            Label("World Files", systemImage: "ellipsis.circle")
                .labelStyle(.iconOnly)
        }
        .accessibilityLabel("World Files")
    }

    private func bounded(_ share: CGFloat) -> CGFloat {
        min(0.63, max(0.30, share))
    }

    private func divider(wide: Bool, available: CGFloat, share: CGFloat) -> some View {
        Rectangle()
            .fill(dividerFocused ? bronzeWorkspaceAccent : Color(uiColor: .separator))
            .frame(width: wide ? 1 : nil, height: wide ? nil : 1)
            .frame(width: wide ? 12 : nil, height: wide ? nil : 12)
            .contentShape(Rectangle())
            .gesture(DragGesture(coordinateSpace: .global)
                .updating($dividerOffset) { drag, offset, _ in
                    offset = wide ? drag.translation.width : drag.translation.height
                }
                .onEnded { drag in
                    let offset = wide ? drag.translation.width : drag.translation.height
                    resize(wide: wide, share: share + offset / available)
                })
            .focusable()
            .focused($dividerFocused)
            .onKeyPress(keys: [.leftArrow, .rightArrow, .upArrow, .downArrow], phases: [.down, .repeat]) { press in
                switch press.key {
                case .leftArrow where wide, .upArrow where !wide:
                    resize(wide: wide, share: share - 0.05)
                case .rightArrow where wide, .downArrow where !wide:
                    resize(wide: wide, share: share + 0.05)
                default: return .ignored
                }
                return .handled
            }
            .accessibilityLabel("Resize map and terrain")
            .accessibilityValue("\(Int(share * 100)) percent map")
            .accessibilityAdjustableAction { direction in
                switch direction {
                case .increment: resize(wide: wide, share: share + 0.05)
                case .decrement: resize(wide: wide, share: share - 0.05)
                @unknown default: break
                }
            }
    }

    private func resize(wide: Bool, share: CGFloat) {
        if wide { wideShare = Double(bounded(share)) }
        else { compactShare = Double(bounded(share)) }
    }

    private func panel(_ title: String, surface: ApeSimulationSurface) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            if surface == .terrain {
                HStack {
                    Label(title, systemImage: "mountain.2").font(.subheadline.weight(.semibold))
                        .accessibilityAddTraits(.isHeader)
                    Spacer()
                    if let actor = snapshot.actor {
                    Text("Actor #\(actor.id)")
                        .font(.caption.monospacedDigit().weight(.semibold))
                        .foregroundStyle(bronzeWorkspaceAccent)
                        .padding(.horizontal, 6).padding(.vertical, 3)
                        .background(bronzeWorkspaceAccent.opacity(0.12), in: Capsule())
                    }
                }
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
            }
            BronzeWorkspaceSurface(sceneModel: sceneModel, surface: surface,
                                   mapCenter: mapCenter ?? BronzeWorkspacePoint(x: 0, y: 0),
                                   mapSpan: mapSpan, terrainBearing: terrainBearing, selectedSite: selectedSite.id,
                                   pickSite: { selectedSite = $0 },
                                   focus: { focusedSurface = surface },
                                   drag: { moveCamera(surface, dx: $0, dy: $1) })
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(.black)
            .clipShape(RoundedRectangle(cornerRadius: 6))
            .overlay {
                RoundedRectangle(cornerRadius: 6)
                    .strokeBorder(focusedSurface == surface ? bronzeWorkspaceAccent : .white.opacity(0.12),
                                  lineWidth: focusedSurface == surface ? 2 : 1)
                    .allowsHitTesting(false)
            }
            .focusable()
            .focused($focusedSurface, equals: surface)
            .onKeyPress(keys: [.leftArrow, .rightArrow, .upArrow, .downArrow], phases: [.down, .repeat]) { press in
                switch press.key {
                case .leftArrow: moveCamera(surface, dx: 0.12, dy: 0)
                case .rightArrow: moveCamera(surface, dx: -0.12, dy: 0)
                case .upArrow where surface == .view: moveCamera(surface, dx: 0, dy: -0.12)
                case .downArrow where surface == .view: moveCamera(surface, dx: 0, dy: 0.12)
                default: return .ignored
                }
                return .handled
            }
            .accessibilityLabel(title)
        }
    }
}

private struct BronzeToolbarLabelStyle: LabelStyle {
    let compact: Bool

    func makeBody(configuration: Configuration) -> some View {
        if compact { configuration.icon }
        else {
            HStack(spacing: 5) {
                configuration.icon
                configuration.title
            }
        }
    }
}

private struct BronzeWorkspaceLegend: View {
    let site: String?
    private let entries = [
        ("Township", "house.fill", Color.orange),
        ("Copper", "hammer.fill", Color.orange),
        ("Tin", "hammer.fill", Color.gray),
        ("Field", "leaf.fill", Color.yellow),
        ("Woodland", "tree.fill", Color.green)
    ]

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            ForEach(entries.indices, id: \.self) { index in
                let (title, icon, color) = entries[index]
                Label {
                    Text(title)
                } icon: {
                    Image(systemName: icon).foregroundStyle(color)
                }
            }
            if let site, !site.isEmpty {
                Text(site).lineLimit(1)
            }
        }
        .font(.caption2)
        .padding(7)
        .background(Color.black.opacity(0.72), in: RoundedRectangle(cornerRadius: 5))
        .accessibilityIdentifier("BronzeWorkspaceLegend")
    }
}

private struct BronzeWorkspaceInspector: View {
    let actor: BronzeWorkspaceActor?

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 10) {
                Label("Inspector", systemImage: "list.bullet.rectangle").font(.subheadline.weight(.semibold)).accessibilityAddTraits(.isHeader)
                if let actor {
                    Grid(alignment: .leading, horizontalSpacing: 16, verticalSpacing: 6) {
                        row("Identity", "Actor #\(actor.id)")
                        row("Vocation", actor.vocation.isEmpty ? "Unassigned" : actor.vocation)
                        row("Workplace", actor.workplaceID == 0 ? "None" : "#\(actor.workplaceID)")
                        row("Current task", actor.task.isEmpty ? "None" : actor.task)
                        if !actor.status.isEmpty { row("Status", actor.status) }
                        if !actor.action.isEmpty { row("Action", actor.action) }
                        row("Remaining work", actor.task.isEmpty ? "None" : "\(actor.remainingMinutes) min")
                        row("Cargo", actor.cargo)
                        row("Items", actor.items)
                        GridRow {
                            Text("Energy").foregroundStyle(.secondary).accessibilityHidden(true)
                            ProgressView(value: actor.energyFraction) {
                                Text(actor.energyFraction.formatted(.percent.precision(.fractionLength(0))))
                            }
                            .accessibilityLabel("Energy")
                            .accessibilityValue(actor.energy)
                        }
                        row("Hunger", String(actor.hunger))
                        row("Fatigue", String(actor.fatigue))
                        row("Last exchange", actor.exchange.isEmpty ? "None recorded" : actor.exchange)
                    }
                } else {
                    Text("No registered being selected").foregroundStyle(.secondary)
                }
            }
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(10)
        }
        .background(.regularMaterial, in: RoundedRectangle(cornerRadius: 8))
        .accessibilityIdentifier("BronzeWorkspaceInspector")
    }

    private func row(_ label: String, _ value: String) -> some View {
        GridRow {
            Text(label).foregroundStyle(.secondary).accessibilityHidden(true)
            Text(value).textSelection(.enabled)
                .accessibilityLabel("\(label), \(value)")
        }
    }
}

private struct BronzeWorkspaceSurface: UIViewRepresentable {
    let sceneModel: ApeSimulationSceneModel
    let surface: ApeSimulationSurface
    let mapCenter: BronzeWorkspacePoint
    let mapSpan: Double
    let terrainBearing: Double
    let selectedSite: UInt64
    let pickSite: (BronzeWorkspaceSite) -> Void
    let focus: () -> Void
    let drag: (Double, Double) -> Void

    func makeUIView(context: Context) -> ASiOSView {
        let view = ASiOSView(sceneModel: sceneModel, surface: surface, rotatesForUIKit: false)
        view.isBronzeWorkspaceSurface = true
        if surface == .view { view.enableBronzeMapScrolling() }
        configure(view)
        sceneModel.attachBronzeWorkspace(view: view)
        return view
    }

    func updateUIView(_ view: ASiOSView, context: Context) {
        configure(view)
    }

    private func configure(_ view: ASiOSView) {
        view.onBronzeSurfaceFocus = focus
        view.onBronzeWorkspaceDrag = drag
        view.bronzeMapCenter = mapCenter
        view.bronzeMapSpan = mapSpan
        view.bronzeTerrainBearing = terrainBearing
        view.bronzeSelectedSite = selectedSite
        view.onBronzeWorkspaceSite = pickSite
    }

    static func dismantleUIView(_ view: ASiOSView, coordinator: ()) {
        view.detachFromSceneModel()
    }
}
#endif

struct BronzeSaveDocument: FileDocument {
    static let readableContentTypes: [UTType] = [.data]
    var data: Data

    static func isCancellation(_ error: Error) -> Bool {
        error is CancellationError ||
            ((error as NSError).domain == NSCocoaErrorDomain &&
             (error as NSError).code == CocoaError.userCancelled.rawValue)
    }

    init(data: Data) { self.data = data }

    init(configuration: ReadConfiguration) throws {
        guard let bytes = configuration.file.regularFileContents else {
            throw CocoaError(.fileReadCorruptFile)
        }
        data = bytes
    }

    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        FileWrapper(regularFileWithContents: data)
    }
}
