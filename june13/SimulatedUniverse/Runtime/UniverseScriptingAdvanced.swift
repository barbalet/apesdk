import Foundation

enum ScriptFailureSafety: String, Codable, CaseIterable, Identifiable {
    case safeToIgnore
    case pausesWhenConfigured
    case unsafePauseRequired

    var id: String { rawValue }
}

struct ScriptErrorHandlingPolicy: Equatable {
    var pauseOnWarnings: Bool
    var pauseOnErrors: Bool
    var unsafeDiagnosticsAlwaysPause: Bool

    func shouldPause(for diagnostic: ScriptDiagnostic, safety: ScriptFailureSafety) -> Bool {
        if safety == .unsafePauseRequired && unsafeDiagnosticsAlwaysPause {
            return true
        }

        switch diagnostic.severity {
        case .info:
            return false
        case .warning:
            return pauseOnWarnings
        case .error:
            return pauseOnErrors || safety == .pausesWhenConfigured
        }
    }

    static let conservative = ScriptErrorHandlingPolicy(
        pauseOnWarnings: false,
        pauseOnErrors: true,
        unsafeDiagnosticsAlwaysPause: true
    )
}

enum AnalysisPluginDataAccess: String, Codable, CaseIterable, Identifiable {
    case snapshotReadOnly
    case runLogReadOnly
    case exportedBundleOnly
    case noLiveEngineAccess

    var id: String { rawValue }
}

struct ExternalAnalysisBoundary: Equatable {
    var title: String
    var allowedAccess: Set<AnalysisPluginDataAccess>
    var explicitlyNotAnAPI: Bool
    var notes: [String]

    static let futureBoundary = ExternalAnalysisBoundary(
        title: "External Analysis Boundary",
        allowedAccess: [.snapshotReadOnly, .runLogReadOnly, .exportedBundleOnly, .noLiveEngineAccess],
        explicitlyNotAnAPI: true,
        notes: [
            "Do not expose live ApeSDK pointers.",
            "Prefer exported snapshot streams and run logs.",
            "Keep a plugin API out of scope until the app data contracts settle."
        ]
    )
}

enum SnapshotStreamExportFormat: String, Codable, CaseIterable, Identifiable {
    case jsonLines
    case jsonArray
    case transferReference

    var id: String { rawValue }
}

struct SnapshotStreamExportHook: Equatable {
    var format: SnapshotStreamExportFormat
    var snapshotKinds: [SnapshotKind]
    var includesContractVersion: Bool
    var maximumSnapshotsPerBatch: Int
    var redactsLocalPaths: Bool

    static let jsonLinesDefault = SnapshotStreamExportHook(
        format: .jsonLines,
        snapshotKinds: [.universe, .being, .terrainSample, .event, .social, .memory],
        includesContractVersion: true,
        maximumSnapshotsPerBatch: 1_000,
        redactsLocalPaths: true
    )
}

enum ScenarioImportValidationStep: String, Codable, CaseIterable, Identifiable {
    case fileSignature
    case schemaVersion
    case snapshotContract
    case deterministicSeed
    case capabilityRequirements
    case checksum

    var id: String { rawValue }
}

struct ExternalScenarioImportPlan: Equatable {
    var acceptedExtensions: Set<String>
    var validationSteps: [ScenarioImportValidationStep]
    var protectsDeterminism: Bool
    var rejectsUnknownMutationCommands: Bool

    static let conservative = ExternalScenarioImportPlan(
        acceptedExtensions: ["json", "simulateduniverse-run", "apesdk-transfer"],
        validationSteps: [.fileSignature, .schemaVersion, .snapshotContract, .deterministicSeed, .capabilityRequirements, .checksum],
        protectsDeterminism: true,
        rejectsUnknownMutationCommands: true
    )
}

struct ScriptExecutionLimitPolicy: Equatable {
    var maximumExecutionMilliseconds: Int
    var maximumOutputMessages: Int
    var maximumOutputBytes: Int
    var minimumCyclesBetweenInterventions: Int

    func accepts(outputBytes: Int, outputMessages: Int) -> Bool {
        outputBytes <= maximumOutputBytes && outputMessages <= maximumOutputMessages
    }

    static let interactive = ScriptExecutionLimitPolicy(
        maximumExecutionMilliseconds: 250,
        maximumOutputMessages: 400,
        maximumOutputBytes: 64_000,
        minimumCyclesBetweenInterventions: 8
    )
}

enum ScriptTestExpectation: String, Codable, CaseIterable, Identifiable {
    case validates
    case logsExecution
    case presentsFailure

    var id: String { rawValue }
}

struct ScriptValidationTestCase: Identifiable, Equatable {
    var id: String
    var sourceText: String
    var expectedStatus: ScriptParseStatus
    var expectations: Set<ScriptTestExpectation>

    static let defaults = [
        ScriptValidationTestCase(
            id: "empty-safe-demo",
            sourceText: "# safe demo\n",
            expectedStatus: .valid,
            expectations: [.validates]
        ),
        ScriptValidationTestCase(
            id: "logging-script",
            sourceText: "# logging\nprint state\n",
            expectedStatus: .warnings,
            expectations: [.validates, .logsExecution]
        ),
        ScriptValidationTestCase(
            id: "unsafe-failure",
            sourceText: "# unsafe mutation\nmutate engine\n",
            expectedStatus: .errors,
            expectations: [.presentsFailure]
        )
    ]
}

struct ScriptDocumentationTopic: Identifiable, Equatable {
    var id: String
    var title: String
    var summary: String
    var emphasizesExperimentFraming: Bool

    static let defaults = [
        ScriptDocumentationTopic(
            id: "scripts-as-experiments",
            title: "Scripts As Experiments",
            summary: "Scripts are presented as reproducible experiments and observations rather than hidden controls.",
            emphasizesExperimentFraming: true
        ),
        ScriptDocumentationTopic(
            id: "determinism",
            title: "Determinism",
            summary: "Script hash, selected being, cycle, touched variables, and result are logged for replay.",
            emphasizesExperimentFraming: true
        )
    ]
}

enum BrainVisualizationDependency: String, Codable, CaseIterable, Identifiable {
    case requiresApeScript
    case usesBrainCapability
    case unavailable

    var id: String { rawValue }
}

struct BrainVisualizationCapabilityReview: Equatable {
    var dependency: BrainVisualizationDependency
    var requiredCapabilities: Set<BridgeCapability>
    var recommendation: String

    static let separateCapability = BrainVisualizationCapabilityReview(
        dependency: .usesBrainCapability,
        requiredCapabilities: [.brain, .braincode],
        recommendation: "Brain visualization should depend on explicit engine brain capabilities, not on script support alone."
    )
}

struct ScriptPanelBridgeAPIRequirement: Identifiable, Equatable {
    var id: String
    var functionName: String
    var purpose: String
    var requiredBeforePublicMilestone: Bool

    static let futureRequirements = [
        ScriptPanelBridgeAPIRequirement(
            id: "validate-script",
            functionName: "sua_script_validate",
            purpose: "Validate source text and return structured diagnostics.",
            requiredBeforePublicMilestone: true
        ),
        ScriptPanelBridgeAPIRequirement(
            id: "execute-script",
            functionName: "sua_script_execute",
            purpose: "Execute script under runtime command serialization.",
            requiredBeforePublicMilestone: true
        ),
        ScriptPanelBridgeAPIRequirement(
            id: "script-output",
            functionName: "sua_script_output_read",
            purpose: "Read non-blocking script output for run log and script panel routing.",
            requiredBeforePublicMilestone: false
        ),
        ScriptPanelBridgeAPIRequirement(
            id: "script-dry-run",
            functionName: "sua_script_dry_run",
            purpose: "Optionally validate runtime effects without mutating the engine.",
            requiredBeforePublicMilestone: false
        )
    ]
}

struct ExtensibilityTruthReview: Equatable {
    var apeSDKRemainsSourceOfTruth: Bool
    var liveEngineMutationOutsideBridgeAllowed: Bool
    var exportedAnalysisCanBeExternal: Bool
    var notes: [String]

    static let initial = ExtensibilityTruthReview(
        apeSDKRemainsSourceOfTruth: true,
        liveEngineMutationOutsideBridgeAllowed: false,
        exportedAnalysisCanBeExternal: true,
        notes: [
            "SwiftUI receives snapshots and submits commands.",
            "External analysis works from exported data.",
            "ApeSDK remains authoritative for simulation state."
        ]
    )
}
