import Foundation

struct SavedRunMetadataV1: Codable, Equatable, Identifiable {
    var id: UUID
    var engineVersion: String
    var appVersion: String
    var seed: UInt64
    var compileFlags: Set<BridgeCapability>
    var scenarioName: String
    var snapshotContractVersion: Int
    var interventionLog: [InterventionLogRecord]

    static func make(metadata: RunMetadata, handshake: BridgeVersionHandshake, interventions: [InterventionLogRecord]) -> SavedRunMetadataV1 {
        SavedRunMetadataV1(
            id: metadata.id,
            engineVersion: handshake.apeSDKShortVersionName,
            appVersion: "0.1",
            seed: metadata.seed,
            compileFlags: handshake.enabledCapabilities,
            scenarioName: metadata.scenarioName,
            snapshotContractVersion: SnapshotContract.currentVersion,
            interventionLog: interventions
        )
    }
}

enum ReplayStorageStrategy: String, Codable, CaseIterable, Identifiable {
    case snapshotsOnly
    case eventLogOnly
    case transferCheckpointsOnly
    case hybrid

    var id: String { rawValue }
}

struct ReplayStorageDecision: Equatable {
    var selectedStrategy: ReplayStorageStrategy
    var rationale: String
    var storesSnapshots: Bool
    var storesEvents: Bool
    var storesTransferCheckpoints: Bool

    static let hybridDefault = ReplayStorageDecision(
        selectedStrategy: .hybrid,
        rationale: "Use sparse transfer checkpoints, event logs, and selected snapshots so replay can balance fidelity, size, and inspectability.",
        storesSnapshots: true,
        storesEvents: true,
        storesTransferCheckpoints: true
    )
}

struct TransferCheckpointPlan: Equatable {
    var intervalCycles: Int
    var usesExistingTransferSupport: Bool
    var storesChecksum: Bool
    var maximumRetainedCheckpoints: Int

    static let sparse = TransferCheckpointPlan(
        intervalCycles: 10_000,
        usesExistingTransferSupport: true,
        storesChecksum: true,
        maximumRetainedCheckpoints: 64
    )
}

enum ReplayTimelineCommand: String, Codable, CaseIterable, Identifiable {
    case scrub
    case step
    case play
    case pause
    case setSpeed
    case jumpToBookmark
    case jumpToEvent

    var id: String { rawValue }
}

struct ReplayTimelineControlState: Equatable {
    var availableCommands: Set<ReplayTimelineCommand>
    var currentCycle: Int
    var speedMultiplier: Double
    var selectedBookmarkID: UUID?
    var selectedEventID: EventIdentifier?

    static let paused = ReplayTimelineControlState(
        availableCommands: Set(ReplayTimelineCommand.allCases),
        currentCycle: 0,
        speedMultiplier: 1.0,
        selectedBookmarkID: nil,
        selectedEventID: nil
    )
}

enum ReplayFidelityLevel: String, Codable, CaseIterable, Identifiable {
    case exactDeterministicRerun
    case eventLevelReplay
    case visualOnlyReview

    var id: String { rawValue }

    var provesEngineEquivalence: Bool {
        self == .exactDeterministicRerun
    }
}

struct ReplayInvalidationRule: Identifiable, Equatable {
    var id: String
    var fieldName: String
    var invalidatesExactReplay: Bool
    var fallbackFidelity: ReplayFidelityLevel

    static let defaults = [
        ReplayInvalidationRule(id: "engine-version", fieldName: "engineVersion", invalidatesExactReplay: true, fallbackFidelity: .eventLevelReplay),
        ReplayInvalidationRule(id: "compile-flags", fieldName: "compileFlags", invalidatesExactReplay: true, fallbackFidelity: .visualOnlyReview),
        ReplayInvalidationRule(id: "snapshot-contract", fieldName: "snapshotContractVersion", invalidatesExactReplay: true, fallbackFidelity: .visualOnlyReview),
        ReplayInvalidationRule(id: "app-version", fieldName: "appVersion", invalidatesExactReplay: false, fallbackFidelity: .eventLevelReplay)
    ]
}

struct ReplayExportSelection: Equatable {
    var includesRunLog: Bool
    var includesSelectionHistory: Bool
    var includesSocialGraphSlices: Bool
    var includesMemoryEvidence: Bool

    static let researchDefault = ReplayExportSelection(
        includesRunLog: true,
        includesSelectionHistory: true,
        includesSocialGraphSlices: true,
        includesMemoryEvidence: true
    )
}

enum RunImportValidationError: String, Codable, CaseIterable, Identifiable {
    case corruptedFile
    case incompatibleEngineVersion
    case incompatibleCompileFlags
    case unsupportedSnapshotContract
    case checksumMismatch

    var id: String { rawValue }

    var userFacingMessage: String {
        switch self {
        case .corruptedFile:
            return "The run file could not be read."
        case .incompatibleEngineVersion:
            return "This run was created with a different ApeSDK engine version."
        case .incompatibleCompileFlags:
            return "This run requires engine capabilities that are not enabled."
        case .unsupportedSnapshotContract:
            return "This run uses an unsupported snapshot contract."
        case .checksumMismatch:
            return "The run file checksum does not match its metadata."
        }
    }
}

struct LocalPreferenceRecord: Equatable {
    var windowFrameDescription: String
    var lastProjectURL: URL?
    var recentScriptURLs: [URL]
    var uiPreferences: UserPreferenceRecord

    static let empty = LocalPreferenceRecord(
        windowFrameDescription: "default",
        lastProjectURL: nil,
        recentScriptURLs: [],
        uiPreferences: UserPreferenceRecord(mode: .explore, enabledOverlays: [], expandedPanels: [], followsSelection: false)
    )
}

struct SnapshotArchivePruningPolicy: Equatable {
    var maximumSnapshotCount: Int
    var maximumArchiveBytes: Int
    var preservesBookmarks: Bool
    var preservesCheckpoints: Bool

    func shouldPrune(snapshotCount: Int, archiveBytes: Int) -> Bool {
        snapshotCount > maximumSnapshotCount || archiveBytes > maximumArchiveBytes
    }

    static let longSession = SnapshotArchivePruningPolicy(
        maximumSnapshotCount: 20_000,
        maximumArchiveBytes: 512_000_000,
        preservesBookmarks: true,
        preservesCheckpoints: true
    )
}

struct RunChecksum: Codable, Equatable {
    var algorithm: String
    var value: String

    static func fnv1a(_ strings: [String]) -> RunChecksum {
        let joined = strings.joined(separator: "\n")
        let digest = joined.unicodeScalars.reduce(UInt64(14_695_981_039_346_656_037)) { partial, scalar in
            (partial ^ UInt64(scalar.value)).multipliedReportingOverflow(by: 1_099_511_628_211).partialValue
        }
        return RunChecksum(algorithm: "fnv1a64", value: String(digest, radix: 16, uppercase: false))
    }
}

struct DeterministicReplayTestCase: Identifiable, Equatable {
    var id: String
    var fixedSeed: UInt64
    var interventionCount: Int
    var expectedSnapshotDigest: RunChecksum

    static let noInterventionSmoke = DeterministicReplayTestCase(
        id: "fixed-seed-no-interventions",
        fixedSeed: DeterministicScenarioProfile.defaultObservation.seed,
        interventionCount: 0,
        expectedSnapshotDigest: .fnv1a(["cycle:1000", "population:64", "contract:1"])
    )
}

struct InterventionReplayTestCase: Identifiable, Equatable {
    var id: String
    var expectedLogOrder: [String]
    var expectedChangedFields: Set<String>

    static let weatherIntervention = InterventionReplayTestCase(
        id: "weather-intervention-order",
        expectedLogOrder: ["loadScenario", "setWeather", "snapshotAfterMutation"],
        expectedChangedFields: ["weather.condition", "runMetadata.interventionCount"]
    )
}

struct UIReplayRestorationTestCase: Identifiable, Equatable {
    var id: String
    var selectedEventID: EventIdentifier
    var restoresCamera: Bool
    var restoresPanelContext: Bool

    static let eventSelection = UIReplayRestorationTestCase(
        id: "event-selection-restores-context",
        selectedEventID: EventIdentifier(cycle: 100, source: .runLog, localIndex: 0),
        restoresCamera: true,
        restoresPanelContext: true
    )
}

struct ReplayFailureTestCase: Identifiable, Equatable {
    var id: String
    var error: RunImportValidationError
    var expectedRecovery: String

    static let defaults = [
        ReplayFailureTestCase(id: "corrupted-run", error: .corruptedFile, expectedRecovery: "Keep the current run open."),
        ReplayFailureTestCase(id: "missing-script", error: .checksumMismatch, expectedRecovery: "Disable script replay and show provenance."),
        ReplayFailureTestCase(id: "engine-mismatch", error: .incompatibleEngineVersion, expectedRecovery: "Fall back to visual review when possible.")
    ]
}

struct ExportBundleStructure: Equatable {
    var rootDirectoryName: String
    var manifestFileName: String
    var runLogFileName: String
    var snapshotsDirectoryName: String
    var notesFileName: String
    var includesPrivacyNote: Bool

    static let demonstrationBundle = ExportBundleStructure(
        rootDirectoryName: "SimulatedUniverseExport",
        manifestFileName: "manifest.json",
        runLogFileName: "run-log.jsonl",
        snapshotsDirectoryName: "snapshots",
        notesFileName: "notes.md",
        includesPrivacyNote: true
    )
}

struct ReplayNote: Identifiable, Equatable {
    var id: UUID
    var cycle: Int
    var eventID: EventIdentifier?
    var text: String
    var createdAt: Date
}

struct ReplayProofDocumentation: Equatable {
    var canProve: [String]
    var cannotProve: [String]

    static let initial = ReplayProofDocumentation(
        canProve: [
            "A fixed seed with matching engine version and compile flags reproduces expected snapshots.",
            "Intervention log order is preserved.",
            "Saved UI context can restore review state."
        ],
        cannotProve: [
            "Visual-only replay proves engine equivalence.",
            "A replay remains exact across incompatible compile flags.",
            "Missing script sources can be reconstructed from a hash."
        ]
    )
}

struct PersistenceReview: Equatable {
    var fileSizeRisk: Int
    var determinismRisk: Int
    var humanReadable: Bool
    var migrationPlanRequired: Bool
    var notes: [String]

    static let initial = PersistenceReview(
        fileSizeRisk: 4,
        determinismRisk: 5,
        humanReadable: true,
        migrationPlanRequired: true,
        notes: [
            "Hybrid replay balances size and fidelity.",
            "Checksums are required for metadata and script inputs.",
            "Schema migrations should be explicit after version 1."
        ]
    )
}

struct RunMetadataSchemaFreeze: Equatable {
    var schemaVersion: Int
    var frozenFields: Set<String>
    var migrationWindowOpen: Bool

    static let version1 = RunMetadataSchemaFreeze(
        schemaVersion: 1,
        frozenFields: ["engineVersion", "appVersion", "seed", "compileFlags", "scenarioName", "snapshotContractVersion", "interventionLog"],
        migrationWindowOpen: false
    )
}
