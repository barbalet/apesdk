import Foundation
import CoreGraphics

enum UniverseMode: String, CaseIterable, Identifiable {
    case explore
    case inspect
    case experiment
    case replay

    var id: String { rawValue }

    var title: String {
        switch self {
        case .explore:
            return "Explore"
        case .inspect:
            return "Inspect"
        case .experiment:
            return "Experiment"
        case .replay:
            return "Replay"
        }
    }
}

enum UniverseOverlay: String, CaseIterable, Identifiable {
    case social
    case memory
    case weather
    case resources

    var id: String { rawValue }

    var title: String {
        switch self {
        case .social:
            return "Social"
        case .memory:
            return "Memory"
        case .weather:
            return "Weather"
        case .resources:
            return "Resources"
        }
    }
}

struct UniverseCameraState: Equatable {
    var center: CGPoint
    var zoom: CGFloat
    var followedBeingID: BeingIdentifier?

    static let initial = UniverseCameraState(center: .zero, zoom: 1.0, followedBeingID: nil)
}

struct BeingStableKey: Hashable, Codable {
    var genderName: UInt16
    var familyName: UInt16
    var dateOfBirth: Int
}

struct BeingIdentifier: Hashable, Codable {
    let value: Int
    let stableKey: BeingStableKey?

    init(value: Int, stableKey: BeingStableKey? = nil) {
        self.value = value
        self.stableKey = stableKey
    }
}

struct UniverseUIState: Equatable {
    var mode: UniverseMode
    var camera: UniverseCameraState
    var enabledOverlays: Set<UniverseOverlay>
    var selectedBeingID: BeingIdentifier?
    var pinnedBeingIDs: Set<BeingIdentifier>
    var expandedPanels: Set<InspectorPanelIdentifier>

    static let initial = UniverseUIState(
        mode: .explore,
        camera: .initial,
        enabledOverlays: [],
        selectedBeingID: nil,
        pinnedBeingIDs: [],
        expandedPanels: []
    )
}

enum InspectorPanelIdentifier: String, Codable {
    case profile
    case memory
    case socialGraph
    case drives
    case environment
    case runLog
    case script
}

enum SimulationCommand: Equatable {
    case pause
    case resume
    case step(cycles: Int)
    case resetScenario(ScenarioDescriptor)
    case selectBeing(BeingIdentifier?)
    case sampleTerrain(MapCoordinate)
    case setWeather(Int)
    case injectResource(ResourceInjection)
    case loadScript(URL)
    case loadReplay(URL)
}

struct ScenarioDescriptor: Equatable {
    var seed: UInt64?
    var populationCount: Int?
    var startsPaused: Bool
}

struct MapCoordinate: Equatable, Codable {
    var x: Int
    var y: Int
}

struct ResourceInjection: Equatable {
    var coordinate: MapCoordinate
    var resourceKind: String
    var amount: Int
}

enum ApeSDKModule: String, CaseIterable, Identifiable {
    case toolkit
    case script
    case sim
    case entity
    case universe
    case render
    case gui

    var id: String { rawValue }

    var interfaceRole: String {
        switch self {
        case .toolkit:
            return "Memory, strings, math, files, objects, and shared low-level types."
        case .script:
            return "ApeScript parsing, diagnostics, and controlled experiment execution."
        case .sim:
            return "Terrain, weather, time, resources, territory, and environmental constants."
        case .entity:
            return "Being state, braincode, body, drives, immune state, memories, and social graph."
        case .universe:
            return "Simulation lifecycle, stepping, selection, transfer, commands, and run authority."
        case .render:
            return "Graph primitives and future reusable drawing support."
        case .gui:
            return "Legacy draw/shared behavior used as reference while the new interface is built."
        }
    }
}

enum SourceBoundary: String, CaseIterable, Identifiable {
    case swiftUIViews
    case renderingModels
    case simulationBridge
    case runtimeController
    case persistence

    var id: String { rawValue }

    var responsibility: String {
        switch self {
        case .swiftUIViews:
            return "Display state, collect intent, and avoid direct ApeSDK structure access."
        case .renderingModels:
            return "Convert snapshots into drawable terrain, beings, overlays, and glyphs."
        case .simulationBridge:
            return "Own C interop, memory ownership, command serialization, and snapshot extraction."
        case .runtimeController:
            return "Own cycle timing, pause state, snapshot cadence, and mode-safe command routing."
        case .persistence:
            return "Save preferences, scenarios, run metadata, replay references, and exports."
        }
    }
}
