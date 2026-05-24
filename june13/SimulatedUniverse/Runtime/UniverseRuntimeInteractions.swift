import Foundation

enum SelectionLossReason: String, Codable, CaseIterable, Identifiable {
    case userCleared
    case timeAdvanced
    case selectedBeingDied
    case populationReloaded
    case scenarioReloaded

    var id: String { rawValue }
}

enum SelectionPersistenceDecision: String, Codable, CaseIterable, Identifiable {
    case keepSelection
    case clearSelection
    case keepPinnedOnly

    var id: String { rawValue }
}

struct SelectionResolution: Codable, Equatable {
    var decision: SelectionPersistenceDecision
    var reason: SelectionLossReason?
    var cycle: Int
}

struct BeingSelectionState: Codable, Equatable {
    var selectedBeingID: BeingIdentifier?
    var followedBeingID: BeingIdentifier?
    var pinnedBeingIDs: Set<BeingIdentifier>
    var lastResolvedCycle: Int
    var lastLossReason: SelectionLossReason?

    static let empty = BeingSelectionState.empty(at: 0)

    static func empty(at cycle: Int) -> BeingSelectionState {
        BeingSelectionState(
            selectedBeingID: nil,
            followedBeingID: nil,
            pinnedBeingIDs: [],
            lastResolvedCycle: cycle,
            lastLossReason: nil
        )
    }

    mutating func select(_ id: BeingIdentifier, follow: Bool, cycle: Int) {
        selectedBeingID = id
        if follow {
            followedBeingID = id
        }
        lastResolvedCycle = cycle
        lastLossReason = nil
    }

    mutating func follow(_ id: BeingIdentifier, cycle: Int) {
        selectedBeingID = id
        followedBeingID = id
        lastResolvedCycle = cycle
        lastLossReason = nil
    }

    mutating func clear(reason: SelectionLossReason, cycle: Int) {
        selectedBeingID = nil
        followedBeingID = nil
        lastResolvedCycle = cycle
        lastLossReason = reason
    }

    mutating func advance(to cycle: Int) {
        lastResolvedCycle = cycle
    }

    mutating func resolve(populationReloaded: Bool, knownBeingIDs: Set<BeingIdentifier>?, cycle: Int) -> SelectionResolution {
        guard let selectedBeingID else {
            lastResolvedCycle = cycle
            return SelectionResolution(decision: .keepSelection, reason: nil, cycle: cycle)
        }

        if populationReloaded {
            if let knownBeingIDs, knownBeingIDs.contains(selectedBeingID) {
                lastResolvedCycle = cycle
                return SelectionResolution(decision: .keepSelection, reason: nil, cycle: cycle)
            }

            clear(reason: .populationReloaded, cycle: cycle)
            return SelectionResolution(decision: .clearSelection, reason: .populationReloaded, cycle: cycle)
        }

        if let knownBeingIDs, knownBeingIDs.contains(selectedBeingID) == false {
            clear(reason: .selectedBeingDied, cycle: cycle)
            return SelectionResolution(decision: pinnedBeingIDs.isEmpty ? .clearSelection : .keepPinnedOnly, reason: .selectedBeingDied, cycle: cycle)
        }

        lastResolvedCycle = cycle
        return SelectionResolution(decision: .keepSelection, reason: .timeAdvanced, cycle: cycle)
    }
}

enum RuntimePauseReason: String, Codable, CaseIterable, Identifiable {
    case scriptFailure
    case invalidTransferLoad
    case snapshotContractMismatch
    case bridgeError

    var id: String { rawValue }
}

struct PauseOnErrorPolicy: Equatable {
    var pausingErrorCodes: Set<BridgeErrorCode>

    func pauseReason(for code: BridgeErrorCode) -> RuntimePauseReason? {
        guard pausingErrorCodes.contains(code) else {
            return nil
        }

        switch code {
        case .scriptFailed:
            return .scriptFailure
        case .transferFailed:
            return .invalidTransferLoad
        case .snapshotContractMismatch:
            return .snapshotContractMismatch
        default:
            return .bridgeError
        }
    }

    static let deterministic = PauseOnErrorPolicy(
        pausingErrorCodes: [.scriptFailed, .transferFailed, .snapshotContractMismatch, .initializationFailed, .simulationUnavailable]
    )
}

struct TimeScalePresentation: Equatable {
    var cycle: Int
    var cyclesPerApeMinute: Int
    var targetCyclesPerSecond: Int
    var wallClockElapsed: TimeInterval

    var simulatedMinutes: Int {
        cycle / max(1, cyclesPerApeMinute)
    }

    var wallClockSecondsDescription: String {
        String(format: "%.1fs", wallClockElapsed)
    }

    var simulationDescription: String {
        "\(simulatedMinutes) ape minutes from \(cycle) cycles"
    }
}

enum PanelInvalidationCause: String, Codable, CaseIterable, Identifiable {
    case selectionChanged
    case snapshotStale
    case scenarioReloaded
    case bridgeContractChanged

    var id: String { rawValue }
}

struct PanelInvalidationRule: Identifiable, Equatable {
    var id: InspectorPanelIdentifier { panel }
    var panel: InspectorPanelIdentifier
    var invalidatesOnSelectionChange: Bool
    var requiredSnapshotKinds: [SnapshotKind]
    var maximumStaleCycles: Int

    func shouldInvalidate(selectionChanged: Bool, lastSnapshotCycle: Int, currentCycle: Int) -> Bool {
        if selectionChanged && invalidatesOnSelectionChange {
            return true
        }

        return currentCycle - lastSnapshotCycle > maximumStaleCycles
    }

    static let defaults = [
        PanelInvalidationRule(panel: .profile, invalidatesOnSelectionChange: true, requiredSnapshotKinds: [.being], maximumStaleCycles: 8),
        PanelInvalidationRule(panel: .memory, invalidatesOnSelectionChange: true, requiredSnapshotKinds: [.memory, .event], maximumStaleCycles: 32),
        PanelInvalidationRule(panel: .socialGraph, invalidatesOnSelectionChange: true, requiredSnapshotKinds: [.social, .event], maximumStaleCycles: 32),
        PanelInvalidationRule(panel: .drives, invalidatesOnSelectionChange: true, requiredSnapshotKinds: [.drive], maximumStaleCycles: 4),
        PanelInvalidationRule(panel: .environment, invalidatesOnSelectionChange: false, requiredSnapshotKinds: [.terrainSample], maximumStaleCycles: 16),
        PanelInvalidationRule(panel: .runLog, invalidatesOnSelectionChange: false, requiredSnapshotKinds: [.event], maximumStaleCycles: 64),
        PanelInvalidationRule(panel: .script, invalidatesOnSelectionChange: false, requiredSnapshotKinds: [.event], maximumStaleCycles: 64)
    ]
}

enum PersistenceArtifactScope: String, CaseIterable, Identifiable {
    case userPreferences
    case runManifest
    case transferFile
    case replay
    case debugExport

    var id: String { rawValue }

    var savedStateRule: String {
        switch self {
        case .userPreferences:
            return "Stores UI choices such as mode, overlays, panels, and window layout without engine state."
        case .runManifest:
            return "Stores reproducibility metadata: seed, build version, cycle, scenario, interventions, and snapshot contract."
        case .transferFile:
            return "Stores authoritative ApeSDK simulation state for loading into another runtime."
        case .replay:
            return "Stores ordered snapshots, commands, and intervention records for later playback."
        case .debugExport:
            return "Stores diagnostics and metrics that are useful to developers but not required for normal restoration."
        }
    }
}

struct UserPreferenceRecord: Equatable {
    var mode: UniverseMode
    var enabledOverlays: Set<UniverseOverlay>
    var expandedPanels: Set<InspectorPanelIdentifier>
    var followsSelection: Bool
}

struct SavedRunManifest: Codable, Equatable, Identifiable {
    var id: UUID
    var scenarioName: String
    var seed: UInt64
    var buildVersion: String
    var snapshotContractVersion: Int
    var currentCycle: Int
    var interventionCount: Int
    var transferFileName: String?

    init(metadata: RunMetadata, transferFileName: String? = nil) {
        id = metadata.id
        scenarioName = metadata.scenarioName
        seed = metadata.seed
        buildVersion = metadata.buildVersion
        snapshotContractVersion = SnapshotContract.currentVersion
        currentCycle = metadata.currentCycle
        interventionCount = metadata.interventionCount
        self.transferFileName = transferFileName
    }
}

enum RuntimeSelectionCommand: Equatable {
    case selectBeing(BeingIdentifier)
    case followBeing(BeingIdentifier)
    case clearSelection
    case sampleTerrain(MapCoordinate)

    var simulationCommand: SimulationCommand {
        switch self {
        case .selectBeing(let id), .followBeing(let id):
            return .selectBeing(id)
        case .clearSelection:
            return .selectBeing(nil)
        case .sampleTerrain(let coordinate):
            return .sampleTerrain(coordinate)
        }
    }
}

enum RuntimeControlCommand: Equatable {
    case pause
    case resume
    case step(Int)
    case reset(ScenarioLoadRequest)
    case loadScenario(ScenarioLoadRequest)

    var simulationCommand: SimulationCommand {
        switch self {
        case .pause:
            return .pause
        case .resume:
            return .resume
        case .step(let cycles):
            return .step(cycles: cycles)
        case .reset(let request), .loadScenario(let request):
            return .resetScenario(request.descriptor)
        }
    }
}

enum RuntimeInterventionCommand: Equatable {
    case setWeather(Int)
    case injectResource(ResourceInjection)
    case loadScript(URL)
    case addAnnotation(MapCoordinate, String)

    var simulationCommand: SimulationCommand? {
        switch self {
        case .setWeather(let weather):
            return .setWeather(weather)
        case .injectResource(let injection):
            return .injectResource(injection)
        case .loadScript(let url):
            return .loadScript(url)
        case .addAnnotation:
            return nil
        }
    }

    var commandName: String {
        switch self {
        case .setWeather:
            return "setWeather"
        case .injectResource:
            return "injectResource"
        case .loadScript:
            return "loadScript"
        case .addAnnotation:
            return "addAnnotation"
        }
    }

    var summary: String {
        switch self {
        case .setWeather(let weather):
            return "Weather override set to \(weather)."
        case .injectResource(let injection):
            return "Injected \(injection.amount) \(injection.resourceKind) at \(injection.coordinate.x), \(injection.coordinate.y)."
        case .loadScript(let url):
            return "Loaded script \(url.lastPathComponent)."
        case .addAnnotation(let coordinate, let text):
            return "Annotation at \(coordinate.x), \(coordinate.y): \(text)"
        }
    }
}

enum RunLogRecordKind: String, Codable, CaseIterable, Identifiable {
    case runtime
    case selection
    case intervention
    case error
    case unattendedRun

    var id: String { rawValue }
}

struct RunLogRecord: Codable, Equatable, Identifiable {
    var id: UUID
    var cycle: Int
    var kind: RunLogRecordKind
    var commandName: String
    var summary: String
    var createdAt: Date

    static func intervention(_ command: RuntimeInterventionCommand, cycle: Int, createdAt: Date) -> RunLogRecord {
        RunLogRecord(
            id: UUID(),
            cycle: cycle,
            kind: .intervention,
            commandName: command.commandName,
            summary: command.summary,
            createdAt: createdAt
        )
    }

    static func runtime(commandName: String, summary: String, cycle: Int, createdAt: Date) -> RunLogRecord {
        RunLogRecord(id: UUID(), cycle: cycle, kind: .runtime, commandName: commandName, summary: summary, createdAt: createdAt)
    }
}

enum RuntimeMetricVisibility: String, Codable, CaseIterable, Identifiable {
    case hiddenInExplore
    case visibleInInspect
    case debugOnly

    var id: String { rawValue }
}

struct RuntimeMetricDescriptor: Identifiable, Equatable {
    var id: String
    var title: String
    var value: String
    var visibility: RuntimeMetricVisibility

    func isVisible(mode: UniverseMode, debugEnabled: Bool) -> Bool {
        switch visibility {
        case .hiddenInExplore:
            return mode != .explore
        case .visibleInInspect:
            return mode == .inspect || debugEnabled
        case .debugOnly:
            return debugEnabled
        }
    }
}

struct RuntimeDebugMetrics: Equatable {
    var phase: RuntimePhase
    var cycle: Int
    var submittedCommandCount: Int
    var unattendedCyclesCompleted: Int
    var health: SimulationHealthSnapshot

    var descriptors: [RuntimeMetricDescriptor] {
        [
            RuntimeMetricDescriptor(id: "phase", title: "Runtime Phase", value: phase.title, visibility: .hiddenInExplore),
            RuntimeMetricDescriptor(id: "cycle", title: "Cycle", value: "\(cycle)", visibility: .visibleInInspect),
            RuntimeMetricDescriptor(id: "commands", title: "Commands", value: "\(submittedCommandCount)", visibility: .debugOnly),
            RuntimeMetricDescriptor(id: "unattended", title: "Unattended Cycles", value: "\(unattendedCyclesCompleted)", visibility: .debugOnly),
            RuntimeMetricDescriptor(id: "bridge-errors", title: "Bridge Errors", value: "\(health.bridgeErrorCount)", visibility: .visibleInInspect)
        ]
    }

    static let empty = RuntimeDebugMetrics(
        phase: .uninitialized,
        cycle: 0,
        submittedCommandCount: 0,
        unattendedCyclesCompleted: 0,
        health: .initial
    )
}

struct DeterministicSessionCheckpoint: Codable, Equatable, Identifiable {
    var id: Int { cycle }
    var cycle: Int
    var populationCount: Int
    var landDate: Int
    var landTime: Int
    var checksumSeed: UInt64
}

struct UnattendedRuntimePlan: Equatable {
    var scenario: ScenarioLoadRequest
    var targetCycles: Int
    var checkpointInterval: Int
    var maximumBatchSize: Int
    var expectedPopulationRange: ClosedRange<Int>
    var acceptanceRules: [String]

    func checkpointCycles() -> [Int] {
        guard checkpointInterval > 0, targetCycles > 0 else {
            return []
        }

        return stride(from: checkpointInterval, through: targetCycles, by: checkpointInterval).map { $0 }
    }

    static let deterministicSmoke = UnattendedRuntimePlan(
        scenario: .defaultObservation,
        targetCycles: 10_000,
        checkpointInterval: 1_000,
        maximumBatchSize: RuntimeExecutionModel.catalystDefault.maximumCyclesPerBatch,
        expectedPopulationRange: 1...256,
        acceptanceRules: [
            "No bridge errors are accepted.",
            "Population must remain in the configured fixture range.",
            "Cycle count must equal the requested deterministic target.",
            "Snapshots must continue to satisfy the current snapshot contract version."
        ]
    )
}
