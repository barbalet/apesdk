import Foundation

enum BridgeFacadeCall: String, CaseIterable, Identifiable {
    case createContext
    case destroyContext
    case loadScenario
    case stepCycles
    case readUniverseSnapshot
    case readBeingSnapshot
    case sampleTerrain
    case readMemorySnapshots
    case readSocialSnapshots
    case submitCommand
    case readCapabilities
    case readVersion

    var id: String { rawValue }
}

enum BridgeOwnedResource: String, CaseIterable, Identifiable {
    case simulationContext
    case transferFile
    case copiedString
    case copiedBytes
    case callbackPointer

    var id: String { rawValue }

    var lifetimeRule: String {
        switch self {
        case .simulationContext:
            return "Created by the facade, closed by an explicit destroy call, and never retained by SwiftUI views."
        case .transferFile:
            return "Owned by the facade until copied into Swift data or written through a persistence operation."
        case .copiedString:
            return "Copied into Swift String immediately, then released by the bridge."
        case .copiedBytes:
            return "Copied into Swift Data or value arrays immediately, then released by the bridge."
        case .callbackPointer:
            return "Installed and removed by the bridge during runtime setup and teardown only."
        }
    }
}

enum BridgeErrorCode: Int, Codable, CaseIterable {
    case none = 0
    case initializationFailed = 1
    case simulationUnavailable = 2
    case invalidSelection = 3
    case invalidTerrainSample = 4
    case transferFailed = 5
    case scriptFailed = 6
    case unsupportedCapability = 7
    case snapshotContractMismatch = 8
    case commandRejected = 9
    case memoryOwnershipViolation = 10
}

enum UniverseBridgeError: Error, Equatable {
    case initializationFailed(String)
    case simulationUnavailable
    case invalidSelection(BeingIdentifier?)
    case invalidTerrainSample(MapCoordinate)
    case transferFailed(String)
    case scriptFailed(String)
    case unsupportedCapability(BridgeCapability)
    case snapshotContractMismatch(expected: Int, actual: Int)
    case commandRejected(SimulationCommand)
    case memoryOwnershipViolation(String)

    var code: BridgeErrorCode {
        switch self {
        case .initializationFailed:
            return .initializationFailed
        case .simulationUnavailable:
            return .simulationUnavailable
        case .invalidSelection:
            return .invalidSelection
        case .invalidTerrainSample:
            return .invalidTerrainSample
        case .transferFailed:
            return .transferFailed
        case .scriptFailed:
            return .scriptFailed
        case .unsupportedCapability:
            return .unsupportedCapability
        case .snapshotContractMismatch:
            return .snapshotContractMismatch
        case .commandRejected:
            return .commandRejected
        case .memoryOwnershipViolation:
            return .memoryOwnershipViolation
        }
    }
}

enum BridgeCapability: String, Codable, CaseIterable, Identifiable {
    case apeScript
    case brain
    case braincode
    case immune
    case territory
    case alphaWeatherDraw
    case highResolutionTerrain
    case transferBinary
    case transferJSON

    var id: String { rawValue }
}

struct BridgeVersionHandshake: Codable, Equatable {
    var appSnapshotContractVersion: Int
    var facadeContractVersion: Int
    var apeSDKVersionNumber: Int
    var apeSDKShortVersionName: String
    var enabledCapabilities: Set<BridgeCapability>

    var isSnapshotContractCompatible: Bool {
        appSnapshotContractVersion == facadeContractVersion
    }

    static let developmentDefault = BridgeVersionHandshake(
        appSnapshotContractVersion: SnapshotContract.currentVersion,
        facadeContractVersion: SnapshotContract.currentVersion,
        apeSDKVersionNumber: 711,
        apeSDKShortVersionName: "Simulated Ape 0.711",
        enabledCapabilities: [.braincode, .immune, .territory, .transferBinary, .transferJSON]
    )
}

struct BridgeFixtureProfile: Codable, Equatable, Identifiable {
    var id: String
    var scenario: DeterministicScenarioProfile
    var cycleCount: Int
    var expectedPopulationRange: ClosedRange<Int>
    var enabledCapabilities: Set<BridgeCapability>

    static let deterministicMiniWorld = BridgeFixtureProfile(
        id: "deterministic-mini-world",
        scenario: .defaultObservation,
        cycleCount: 200,
        expectedPopulationRange: 1...256,
        enabledCapabilities: [.braincode, .immune, .territory, .transferBinary]
    )
}

struct BridgeTestPlan: Equatable {
    var fixture: BridgeFixtureProfile
    var stepCounts: [Int]
    var stableFields: [StableSnapshotField]
    var failureCases: [BridgeErrorCode]

    static let initialDeterministic = BridgeTestPlan(
        fixture: .deterministicMiniWorld,
        stepCounts: [0, 1, 10, 100, 200],
        stableFields: [.contractVersion, .populationCount, .selectedIdentity, .landDate, .landTime, .mapMetadata],
        failureCases: [.initializationFailed, .invalidSelection, .invalidTerrainSample, .snapshotContractMismatch]
    )
}

enum StableSnapshotField: String, CaseIterable {
    case contractVersion
    case populationCount
    case selectedIdentity
    case landDate
    case landTime
    case mapMetadata
}

enum BridgeStringConversionPolicy: String, CaseIterable, Identifiable {
    case nullTerminatedCString
    case fixedLengthByteBuffer
    case optionalCString

    var id: String { rawValue }

    var rule: String {
        switch self {
        case .nullTerminatedCString:
            return "Copy the C string into Swift immediately and never store the original pointer."
        case .fixedLengthByteBuffer:
            return "Copy only the explicit byte count, then decode using UTF-8 with replacement on invalid data."
        case .optionalCString:
            return "Treat nil as an absent value rather than an empty string."
        }
    }
}

enum BridgeByteConversionPolicy: String, CaseIterable, Identifiable {
    case copyToData
    case copyToTypedArray
    case exposeReadOnlyBorrowDuringCall

    var id: String { rawValue }

    var rule: String {
        switch self {
        case .copyToData:
            return "Use for transfer payloads and image buffers that outlive the facade call."
        case .copyToTypedArray:
            return "Use for compact numeric snapshot fields that the UI will inspect directly."
        case .exposeReadOnlyBorrowDuringCall:
            return "Use only inside bridge implementation helpers and never return the borrowed pointer to SwiftUI."
        }
    }
}

struct EngineCommandEnvelope: Equatable, Identifiable {
    var id: UUID
    var command: SimulationCommand
    var sourceMode: UniverseMode
    var submittedAtCycle: Int?
    var requiresSimulationMutation: Bool

    init(command: SimulationCommand, sourceMode: UniverseMode, submittedAtCycle: Int? = nil) {
        self.id = UUID()
        self.command = command
        self.sourceMode = sourceMode
        self.submittedAtCycle = submittedAtCycle
        self.requiresSimulationMutation = command.requiresSimulationMutation
    }
}

extension SimulationCommand {
    var requiresSimulationMutation: Bool {
        switch self {
        case .pause, .resume, .step, .resetScenario, .setWeather, .injectResource, .loadScript, .loadReplay:
            return true
        case .selectBeing, .sampleTerrain:
            return false
        }
    }
}

enum CommandQueueRule: String, CaseIterable, Identifiable {
    case singleWriter
    case orderedExecution
    case snapshotAfterMutation
    case rejectUnsupportedCapability
    case logMutations

    var id: String { rawValue }

    var rule: String {
        switch self {
        case .singleWriter:
            return "Only the runtime controller submits commands to the facade."
        case .orderedExecution:
            return "Commands execute in submission order against one simulation context."
        case .snapshotAfterMutation:
            return "State-changing commands request a fresh snapshot after completion."
        case .rejectUnsupportedCapability:
            return "Commands requiring disabled compile-time capabilities fail before reaching the engine."
        case .logMutations:
            return "Every state-changing command emits an intervention or run-log event."
        }
    }
}

enum ContractConsumer: String, CaseIterable, Identifiable {
    case worldViewport
    case selectedApePanel
    case memoryPanel
    case socialGraphPanel
    case driveTimelinePanel
    case environmentInspector
    case runLogPanel
    case scriptPanel
    case socialOverlay
    case memoryOverlay
    case weatherOverlay
    case resourceOverlay

    var id: String { rawValue }

    var requiredSnapshotKinds: [SnapshotKind] {
        switch self {
        case .worldViewport:
            return [.universe, .being]
        case .selectedApePanel:
            return [.being]
        case .memoryPanel:
            return [.memory, .event]
        case .socialGraphPanel:
            return [.social, .event]
        case .driveTimelinePanel:
            return [.being, .drive]
        case .environmentInspector:
            return [.terrainSample]
        case .runLogPanel:
            return [.event]
        case .scriptPanel:
            return [.event]
        case .socialOverlay:
            return [.being, .social]
        case .memoryOverlay:
            return [.memory, .event]
        case .weatherOverlay:
            return [.universe, .terrainSample]
        case .resourceOverlay:
            return [.terrainSample]
        }
    }
}

enum SnapshotKind: String, Codable, CaseIterable {
    case universe
    case being
    case terrainSample
    case event
    case social
    case memory
    case drive
}
