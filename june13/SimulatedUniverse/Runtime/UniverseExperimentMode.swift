import Foundation

enum InterventionTaxonomy: String, Codable, CaseIterable, Identifiable {
    case cameraOnly
    case observationalAnnotation
    case scenarioSetup
    case simulationChangingIntervention

    var id: String { rawValue }

    var mutatesSimulation: Bool {
        self == .scenarioSetup || self == .simulationChangingIntervention
    }
}

enum ExperimentActor: String, Codable, CaseIterable, Identifiable {
    case user
    case script
    case template
    case system

    var id: String { rawValue }
}

enum InterventionResult: String, Codable, CaseIterable, Identifiable {
    case accepted
    case rejected
    case deferred
    case failed

    var id: String { rawValue }
}

struct InterventionLogRecord: Codable, Equatable, Identifiable {
    var id: UUID
    var cycle: Int
    var actor: ExperimentActor
    var taxonomy: InterventionTaxonomy
    var commandName: String
    var parameters: [String: String]
    var preStateReference: String?
    var result: InterventionResult

    static func make(
        cycle: Int,
        actor: ExperimentActor,
        taxonomy: InterventionTaxonomy,
        commandName: String,
        parameters: [String: String],
        preStateReference: String?,
        result: InterventionResult
    ) -> InterventionLogRecord {
        InterventionLogRecord(
            id: UUID(),
            cycle: cycle,
            actor: actor,
            taxonomy: taxonomy,
            commandName: commandName,
            parameters: parameters,
            preStateReference: preStateReference,
            result: result
        )
    }
}

struct ExperimentScenarioSetup: Equatable {
    var fixedSeed: UInt64?
    var startDate: Int?
    var populationCount: Int?
    var initialSelectedBeingID: BeingIdentifier?
    var startsPaused: Bool

    var loadRequest: ScenarioLoadRequest {
        var profile = DeterministicScenarioProfile.defaultObservation
        if let fixedSeed {
            profile.seed = fixedSeed
        }
        if let populationCount {
            profile.populationCount = populationCount
        }
        if let startDate {
            profile.startTimeMinutes = startDate
        }

        return ScenarioLoadRequest(source: fixedSeed.map(ScenarioLoadSource.fixedSeed) ?? .defaultWorld, profile: profile, startsPausedOverride: startsPaused)
    }

    static let defaultTeaching = ExperimentScenarioSetup(
        fixedSeed: DeterministicScenarioProfile.defaultObservation.seed,
        startDate: 0,
        populationCount: 64,
        initialSelectedBeingID: nil,
        startsPaused: false
    )
}

struct WeatherInterventionControl: Equatable {
    var weatherValue: Int
    var marksRunAsInfluenced: Bool
    var confirmationRequired: Bool

    var command: RuntimeInterventionCommand {
        .setWeather(weatherValue)
    }

    static let disabledNeutral = WeatherInterventionControl(weatherValue: 0, marksRunAsInfluenced: true, confirmationRequired: true)
}

struct ResourceInterventionControl: Equatable {
    var injection: ResourceInjection
    var requiredCapability: BridgeCapability
    var safeCommandPathAvailable: Bool

    var command: RuntimeInterventionCommand? {
        safeCommandPathAvailable ? .injectResource(injection) : nil
    }
}

struct ControlledEventInjectionPlan: Equatable {
    var title: String
    var directMutationAllowed: Bool
    var futureBridgeAPINeeded: String
    var safetyNotes: [String]

    static let futureOnly = ControlledEventInjectionPlan(
        title: "Controlled Event Injection",
        directMutationAllowed: false,
        futureBridgeAPINeeded: "A facade command that validates event type, target beings, coordinate, and deterministic replay metadata.",
        safetyNotes: [
            "No direct engine memory mutation.",
            "Rejected until the bridge exposes a safe command path.",
            "Must log pre-state and result."
        ]
    )
}

struct ObservationAnnotation: Equatable, Identifiable {
    var id: UUID
    var cycle: Int
    var coordinate: MapCoordinate?
    var selectedBeingID: BeingIdentifier?
    var text: String
    var changesSimulation: Bool

    static func make(cycle: Int, coordinate: MapCoordinate?, selectedBeingID: BeingIdentifier?, text: String) -> ObservationAnnotation {
        ObservationAnnotation(id: UUID(), cycle: cycle, coordinate: coordinate, selectedBeingID: selectedBeingID, text: text, changesSimulation: false)
    }
}

struct ExperimentBookmark: Equatable, Identifiable {
    var id: UUID
    var title: String
    var cycle: Int
    var selectedBeingID: BeingIdentifier?
    var camera: RenderCameraState
    var overlays: Set<UniverseOverlay>
    var expandedPanels: Set<InspectorPanelIdentifier>
}

struct ExperimentSummaryModel: Equatable {
    var title: String
    var seed: UInt64
    var interventions: [InterventionLogRecord]
    var scriptNames: [String]
    var bookmarks: [ExperimentBookmark]
    var outcomeNotes: [String]

    var influencedRun: Bool {
        interventions.contains { $0.taxonomy.mutatesSimulation }
    }
}

enum ExperimentTemplateKind: String, Codable, CaseIterable, Identifiable {
    case teaching
    case debugging
    case behaviorComparison
    case longRunObservation

    var id: String { rawValue }
}

struct ExperimentTemplate: Identifiable, Equatable {
    var id: ExperimentTemplateKind
    var title: String
    var scenarioSetup: ExperimentScenarioSetup
    var allowedTaxonomies: Set<InterventionTaxonomy>
    var notes: [String]

    static let defaults = [
        ExperimentTemplate(
            id: .teaching,
            title: "Teaching Observation",
            scenarioSetup: .defaultTeaching,
            allowedTaxonomies: [.cameraOnly, .observationalAnnotation],
            notes: ["Read-only demonstration preset."]
        ),
        ExperimentTemplate(
            id: .debugging,
            title: "Debugging Session",
            scenarioSetup: .defaultTeaching,
            allowedTaxonomies: Set(InterventionTaxonomy.allCases),
            notes: ["Developer preset may expose raw identifiers and bridge diagnostics."]
        ),
        ExperimentTemplate(
            id: .behaviorComparison,
            title: "Behavior Comparison",
            scenarioSetup: .defaultTeaching,
            allowedTaxonomies: [.cameraOnly, .observationalAnnotation, .scenarioSetup],
            notes: ["Compare fixed seeds and selected beings without hidden mutation."]
        ),
        ExperimentTemplate(
            id: .longRunObservation,
            title: "Long-Run Observation",
            scenarioSetup: .defaultTeaching,
            allowedTaxonomies: [.cameraOnly, .observationalAnnotation],
            notes: ["Designed for unattended deterministic observation."]
        )
    ]
}

struct InterventionConfirmationPolicy: Equatable {
    var expertModeEnabled: Bool
    var confirmsStateChangingInterventions: Bool
    var confirmsScriptExecution: Bool

    func requiresConfirmation(for taxonomy: InterventionTaxonomy) -> Bool {
        guard expertModeEnabled == false else {
            return false
        }

        return confirmsStateChangingInterventions && taxonomy.mutatesSimulation
    }

    static let standard = InterventionConfirmationPolicy(expertModeEnabled: false, confirmsStateChangingInterventions: true, confirmsScriptExecution: true)
}

enum InterventionUndoExpectation: String, Codable, CaseIterable, Identifiable {
    case reversibleInUI
    case requiresReplay
    case irreversible
    case notApplicable

    var id: String { rawValue }
}

struct InterventionUndoRule: Identifiable, Equatable {
    var id: InterventionTaxonomy { taxonomy }
    var taxonomy: InterventionTaxonomy
    var expectation: InterventionUndoExpectation
    var explanation: String

    static let defaults = [
        InterventionUndoRule(taxonomy: .cameraOnly, expectation: .reversibleInUI, explanation: "Camera moves can be undone locally."),
        InterventionUndoRule(taxonomy: .observationalAnnotation, expectation: .reversibleInUI, explanation: "Annotations can be edited or removed from the experiment notes."),
        InterventionUndoRule(taxonomy: .scenarioSetup, expectation: .requiresReplay, explanation: "Scenario setup changes the initial conditions."),
        InterventionUndoRule(taxonomy: .simulationChangingIntervention, expectation: .requiresReplay, explanation: "Engine state changes require replay or a transfer checkpoint.")
    ]
}

struct InterventionValidationResult: Equatable {
    var accepted: Bool
    var reason: String

    static let accepted = InterventionValidationResult(accepted: true, reason: "Accepted.")
}

struct InterventionCommandValidator: Equatable {
    var enabledCapabilities: Set<BridgeCapability>
    var allowedTaxonomies: Set<InterventionTaxonomy>

    func validate(taxonomy: InterventionTaxonomy, requiredCapability: BridgeCapability?) -> InterventionValidationResult {
        guard allowedTaxonomies.contains(taxonomy) else {
            return InterventionValidationResult(accepted: false, reason: "Intervention taxonomy is not enabled for this experiment.")
        }

        if let requiredCapability, enabledCapabilities.contains(requiredCapability) == false {
            return InterventionValidationResult(accepted: false, reason: "Required bridge capability is not enabled.")
        }

        return .accepted
    }
}

struct InterventionTestCase: Identifiable, Equatable {
    var id: String
    var commandName: String
    var expectedSerializedKeys: Set<String>
    var expectedResult: InterventionResult

    static let defaults = [
        InterventionTestCase(id: "weather-log", commandName: "setWeather", expectedSerializedKeys: ["weather"], expectedResult: .accepted),
        InterventionTestCase(id: "annotation-log", commandName: "addAnnotation", expectedSerializedKeys: ["text", "cycle"], expectedResult: .accepted),
        InterventionTestCase(id: "resource-rejected-without-capability", commandName: "injectResource", expectedSerializedKeys: ["kind", "amount"], expectedResult: .rejected)
    ]
}

struct DeterministicInterventionComparison: Equatable {
    var baselineRunID: UUID
    var interventionRunID: UUID
    var comparedFields: [String]
    var expectedDifferences: [String]
    var deterministicPrecondition: String
}

struct ExperimentPersistencePlan: Equatable {
    var namingRule: String
    var saveExtension: String
    var duplicateBehavior: String
    var exportContents: [String]

    static let initial = ExperimentPersistencePlan(
        namingRule: "Use scenario name, seed, and first intervention cycle.",
        saveExtension: "simulateduniverse-experiment",
        duplicateBehavior: "Duplicate metadata, annotations, bookmarks, and script references without copying transfer files unless requested.",
        exportContents: ["summary", "intervention-log", "bookmarks", "annotations", "run-manifest"]
    )
}

struct ExportProvenanceNote: Equatable {
    var includesUserAnnotations: Bool
    var includesLocalPaths: Bool
    var includesRawIdentifiers: Bool
    var note: String

    static let publicSharing = ExportProvenanceNote(
        includesUserAnnotations: true,
        includesLocalPaths: false,
        includesRawIdentifiers: true,
        note: "Exports should identify engine version, app version, seed, compile flags, and whether interventions changed the run."
    )
}

struct ExperimentModeReview: Equatable {
    var supportsScience: Bool
    var supportsTeaching: Bool
    var supportsDebugging: Bool
    var hiddenGameplayControlRisk: Bool
    var notes: [String]

    static let initial = ExperimentModeReview(
        supportsScience: true,
        supportsTeaching: true,
        supportsDebugging: true,
        hiddenGameplayControlRisk: false,
        notes: [
            "State-changing actions are explicit interventions.",
            "Camera and annotations remain observational.",
            "Templates separate teaching from developer diagnostics."
        ]
    )
}

struct ExperimentPreset: Identifiable, Equatable {
    var id: String
    var title: String
    var template: ExperimentTemplateKind
    var readOnly: Bool
    var exposesRawIDs: Bool
    var exposesCompileFlags: Bool
    var exposesBridgeDiagnostics: Bool
    var exposesPerformanceMetrics: Bool

    static let readOnlyTeaching = ExperimentPreset(
        id: "read-only-teaching",
        title: "Read-Only Teaching",
        template: .teaching,
        readOnly: true,
        exposesRawIDs: false,
        exposesCompileFlags: false,
        exposesBridgeDiagnostics: false,
        exposesPerformanceMetrics: false
    )

    static let developer = ExperimentPreset(
        id: "developer-diagnostics",
        title: "Developer Diagnostics",
        template: .debugging,
        readOnly: false,
        exposesRawIDs: true,
        exposesCompileFlags: true,
        exposesBridgeDiagnostics: true,
        exposesPerformanceMetrics: true
    )
}
