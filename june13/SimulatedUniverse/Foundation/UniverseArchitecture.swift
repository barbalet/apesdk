import Foundation

enum RendererStrategy: String, CaseIterable, Identifiable {
    case cartographic2D
    case deferred3D

    var id: String { rawValue }

    var title: String {
        switch self {
        case .cartographic2D:
            return "Layered 2D cartographic world"
        case .deferred3D:
            return "Deferred 3D world"
        }
    }

    var role: String {
        switch self {
        case .cartographic2D:
            return "First renderer: terrain, water, weather, beings, paths, social overlays, memory locations, and event glyphs."
        case .deferred3D:
            return "Later renderer once bridge snapshots, camera semantics, and behavior evidence are stable."
        }
    }
}

struct FrameBudget: Equatable {
    var targetFramesPerSecond: Int
    var simulationStepMilliseconds: Double
    var snapshotMilliseconds: Double
    var worldRenderMilliseconds: Double
    var panelUpdateMilliseconds: Double
    var loggingMilliseconds: Double

    var totalAllocatedMilliseconds: Double {
        simulationStepMilliseconds + snapshotMilliseconds + worldRenderMilliseconds + panelUpdateMilliseconds + loggingMilliseconds
    }

    static let initialInteractive = FrameBudget(
        targetFramesPerSecond: 60,
        simulationStepMilliseconds: 4.0,
        snapshotMilliseconds: 2.0,
        worldRenderMilliseconds: 7.0,
        panelUpdateMilliseconds: 2.0,
        loggingMilliseconds: 1.0
    )
}

enum RuntimeOwnership: String, CaseIterable, Identifiable {
    case simulationLoop
    case uiState
    case renderCache
    case fileOperations

    var id: String { rawValue }

    var owner: SourceBoundary {
        switch self {
        case .simulationLoop:
            return .runtimeController
        case .uiState:
            return .swiftUIViews
        case .renderCache:
            return .renderingModels
        case .fileOperations:
            return .persistence
        }
    }

    var rule: String {
        switch self {
        case .simulationLoop:
            return "The runtime controller serializes engine commands and publishes immutable snapshots."
        case .uiState:
            return "SwiftUI owns view selection, camera, overlays, panel expansion, and preference-facing state."
        case .renderCache:
            return "Rendering code owns draw-ready caches derived from snapshots, never live engine memory."
        case .fileOperations:
            return "Persistence owns scenario, replay, export, and preference reads and writes."
        }
    }
}

enum SimulationClockControl: Equatable {
    case paused
    case runRealtime
    case step(cycles: Int)
    case fastForward(multiplier: Int)
    case runFixed(cycles: Int)

    var command: SimulationCommand {
        switch self {
        case .paused:
            return .pause
        case .runRealtime:
            return .resume
        case .step(let cycles):
            return .step(cycles: cycles)
        case .fastForward(let multiplier):
            return .step(cycles: max(1, multiplier))
        case .runFixed(let cycles):
            return .step(cycles: cycles)
        }
    }
}

enum FailureSurface: String, CaseIterable, Identifiable {
    case engineError
    case scriptError
    case transferError
    case corruptedReplay
    case missingAsset

    var id: String { rawValue }

    var recoveryPolicy: String {
        switch self {
        case .engineError:
            return "Pause the runtime, preserve the latest snapshot, and present diagnostic context."
        case .scriptError:
            return "Show parser or execution diagnostics, log the script hash, and pause only when configured."
        case .transferError:
            return "Reject the load, keep the current run intact, and explain the incompatible field or version."
        case .corruptedReplay:
            return "Open no replay state, report validation failure, and keep the current live run untouched."
        case .missingAsset:
            return "Use a fallback visual surface and report the missing asset in diagnostics."
        }
    }
}

enum InterventionCapability: String, Codable, CaseIterable, Identifiable {
    case weather
    case resources
    case scripts
    case annotations

    var id: String { rawValue }
}

struct DeterministicScenarioProfile: Codable, Equatable {
    var name: String
    var seed: UInt64
    var startCycle: Int
    var startTimeMinutes: Int
    var populationCount: Int
    var terrainProfile: String
    var enabledInterventions: [InterventionCapability]
    var startsPaused: Bool

    static let defaultObservation = DeterministicScenarioProfile(
        name: "Default Observation",
        seed: 0xA9E5D1C0,
        startCycle: 0,
        startTimeMinutes: 0,
        populationCount: 64,
        terrainProfile: "apesdk-default",
        enabledInterventions: [.annotations],
        startsPaused: false
    )
}

enum NamingRule: String, CaseIterable, Identifiable {
    case appTypesUseUniversePrefix
    case cFacadeUsesSUAPrefix
    case snapshotTypesEndInSnapshot
    case viewModelsEndInModel
    case commandsAreExplicitVerbs

    var id: String { rawValue }

    var rule: String {
        switch self {
        case .appTypesUseUniversePrefix:
            return "App-wide Swift types use Universe or SimulatedUniverse naming when the scope is global."
        case .cFacadeUsesSUAPrefix:
            return "Future C facade functions use an sua_ prefix to avoid colliding with ApeSDK symbols."
        case .snapshotTypesEndInSnapshot:
            return "Read-only engine-derived data types end in Snapshot."
        case .viewModelsEndInModel:
            return "Observable UI adapter types end in Model."
        case .commandsAreExplicitVerbs:
            return "Simulation commands name the engine action explicitly rather than describing a button."
        }
    }
}

enum BridgeStrategy: String, CaseIterable, Identifiable {
    case directCInterop
    case wrapperLayer
    case stableFacade

    var id: String { rawValue }

    static let selected: BridgeStrategy = .stableFacade

    var decision: String {
        switch self {
        case .directCInterop:
            return "Useful for experiments, but too easy for UI code to depend on internal structures."
        case .wrapperLayer:
            return "A transition option when existing C APIs can be grouped without a formal snapshot contract."
        case .stableFacade:
            return "Selected path: a small C facade exposes commands and snapshots while ApeSDK remains authoritative."
        }
    }
}

struct TechnicalRisk: Identifiable, Equatable {
    var id: String
    var title: String
    var severity: Int
    var mitigation: String

    static let initialRisks = [
        TechnicalRisk(
            id: "bridge-lifetime",
            title: "Bridge lifetime and memory ownership",
            severity: 5,
            mitigation: "Centralize C interop in the bridge and expose copied snapshots to Swift."
        ),
        TechnicalRisk(
            id: "large-snapshots",
            title: "Large state snapshots",
            severity: 4,
            mitigation: "Throttle snapshot cadence and split world, selected being, panel, and replay snapshots."
        ),
        TechnicalRisk(
            id: "render-cost",
            title: "Rendering cost at high zoom or large population",
            severity: 4,
            mitigation: "Use layered 2D caches first and add renderer performance fixtures before 3D work."
        ),
        TechnicalRisk(
            id: "reproducibility",
            title: "Run reproducibility",
            severity: 5,
            mitigation: "Treat seed, compile flags, scenario profile, scripts, and interventions as run metadata."
        )
    ]
}

enum DeliveryMilestone: String, CaseIterable, Identifiable {
    case worldView
    case selection
    case overlays
    case memoryAndSocialPanels
    case scriptsAndRunLog
    case replay
    case releasePackaging

    var id: String { rawValue }

    var goal: String {
        switch self {
        case .worldView:
            return "Show a live world view backed by ApeSDK snapshots."
        case .selection:
            return "Select, follow, pin, and compare simulated beings."
        case .overlays:
            return "Render social, memory, weather, resource, path, and terrain evidence layers."
        case .memoryAndSocialPanels:
            return "Connect memory and relationship summaries to specific evidence."
        case .scriptsAndRunLog:
            return "Run controlled scripts and preserve interventions in the run log."
        case .replay:
            return "Save, reload, inspect, and replay deterministic or event-level runs."
        case .releasePackaging:
            return "Package a signed Catalyst build with documentation and reproducibility checks."
        }
    }
}
