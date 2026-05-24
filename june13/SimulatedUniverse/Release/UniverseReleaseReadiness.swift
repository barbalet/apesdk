import Foundation

enum DeveloperDocumentationSurface: String, Codable, CaseIterable, Identifiable {
    case bridgeContracts
    case snapshotModels
    case rendererLayers
    case commandQueues

    var id: String { rawValue }
}

struct DeveloperDocumentationPlan: Identifiable, Equatable {
    var id: DeveloperDocumentationSurface { surface }
    var surface: DeveloperDocumentationSurface
    var title: String
    var mustExplain: [String]
    var sourceFiles: [String]

    static let defaults = [
        DeveloperDocumentationPlan(
            surface: .bridgeContracts,
            title: "Bridge Contracts",
            mustExplain: ["Swift owns value snapshots.", "ApeSDK owns live simulation state.", "Commands cross the bridge through serialized envelopes."],
            sourceFiles: ["Bridge/UniverseBridgeContracts.swift", "Bridge/SimulatedUniverseBridge.h"]
        ),
        DeveloperDocumentationPlan(
            surface: .snapshotModels,
            title: "Snapshot Models",
            mustExplain: ["Snapshot contract versioning is explicit.", "UI models cite source snapshot fields.", "Unknown engine values remain visible as unknown values."],
            sourceFiles: ["Models/UniverseSnapshots.swift"]
        ),
        DeveloperDocumentationPlan(
            surface: .rendererLayers,
            title: "Renderer Layers",
            mustExplain: ["Base terrain, beings, events, overlays, selection, and cursor layers are ordered.", "Overlay visuals answer inspection questions.", "Rendering models are not simulation authority."],
            sourceFiles: ["Rendering/UniverseRendering.swift", "Rendering/UniverseRenderingOverlays.swift"]
        ),
        DeveloperDocumentationPlan(
            surface: .commandQueues,
            title: "Command Queues",
            mustExplain: ["Mutating commands are serialized.", "Observation commands do not change simulation state.", "Interventions enter the run log."],
            sourceFiles: ["Runtime/UniverseRuntimeInteractions.swift", "Runtime/UniverseExperimentMode.swift"]
        )
    ]
}

enum OnboardingStepKind: String, Codable, CaseIterable, Identifiable {
    case observeWorld
    case selectBeing
    case followLifeThread
    case inspectEvidence
    case makeExplicitIntervention
    case replayRun

    var id: String { rawValue }
}

struct OnboardingInteractionStep: Identifiable, Equatable {
    var id: OnboardingStepKind { kind }
    var kind: OnboardingStepKind
    var startsFromLivingWorld: Bool
    var requiresModalGate: Bool
    var successSignal: String

    static let defaults = [
        OnboardingInteractionStep(kind: .observeWorld, startsFromLivingWorld: true, requiresModalGate: false, successSignal: "User sees terrain, weather, and moving beings immediately."),
        OnboardingInteractionStep(kind: .selectBeing, startsFromLivingWorld: true, requiresModalGate: false, successSignal: "User selects a being and sees the compact profile update."),
        OnboardingInteractionStep(kind: .followLifeThread, startsFromLivingWorld: true, requiresModalGate: false, successSignal: "Camera follows the selected being without mutating the simulation."),
        OnboardingInteractionStep(kind: .inspectEvidence, startsFromLivingWorld: true, requiresModalGate: false, successSignal: "Memory and social panels show values tied to snapshots."),
        OnboardingInteractionStep(kind: .makeExplicitIntervention, startsFromLivingWorld: true, requiresModalGate: false, successSignal: "Run log records the intervention before and after state."),
        OnboardingInteractionStep(kind: .replayRun, startsFromLivingWorld: true, requiresModalGate: false, successSignal: "Replay mode states its fidelity and restores review context.")
    ]
}

enum SampleScenarioKind: String, Codable, CaseIterable, Identifiable {
    case observation
    case socialRelationships
    case memoryInspection
    case weatherChange
    case scriptExperiment

    var id: String { rawValue }
}

struct SampleScenarioPlan: Identifiable, Equatable {
    var id: SampleScenarioKind { kind }
    var kind: SampleScenarioKind
    var title: String
    var seed: UInt64
    var enabledMode: UniverseMode
    var expectedEvidence: [String]

    static let defaults = [
        SampleScenarioPlan(kind: .observation, title: "Default Living World", seed: DeterministicScenarioProfile.defaultObservation.seed, enabledMode: .explore, expectedEvidence: ["Population count", "Weather", "Terrain sampling"]),
        SampleScenarioPlan(kind: .socialRelationships, title: "Social Relationship Review", seed: 71_002, enabledMode: .inspect, expectedEvidence: ["Known beings", "Attraction values", "Recent social events"]),
        SampleScenarioPlan(kind: .memoryInspection, title: "Memory Evidence Walkthrough", seed: 71_003, enabledMode: .inspect, expectedEvidence: ["Event memories", "Location references", "Involved beings"]),
        SampleScenarioPlan(kind: .weatherChange, title: "Weather Intervention", seed: 71_004, enabledMode: .experiment, expectedEvidence: ["Initial weather", "Intervention log", "Post-command snapshot"]),
        SampleScenarioPlan(kind: .scriptExperiment, title: "Script Validation Run", seed: 71_005, enabledMode: .experiment, expectedEvidence: ["Script hash", "Diagnostics", "Replay metadata"])
    ]
}

enum SimulatedUniverseBuildConfigurationKind: String, Codable, CaseIterable, Identifiable {
    case debug
    case release
    case profile

    var id: String { rawValue }
}

struct SimulatedUniverseBuildSettingPlan: Identifiable, Equatable {
    var id: SimulatedUniverseBuildConfigurationKind { kind }
    var kind: SimulatedUniverseBuildConfigurationKind
    var optimization: String
    var compilationConditions: Set<String>
    var intendedUse: String

    static let defaults = [
        SimulatedUniverseBuildSettingPlan(kind: .debug, optimization: "-Onone", compilationConditions: ["DEBUG"], intendedUse: "Daily implementation and UI preview work."),
        SimulatedUniverseBuildSettingPlan(kind: .release, optimization: "-O", compilationConditions: [], intendedUse: "Signed public artifacts and archive validation."),
        SimulatedUniverseBuildSettingPlan(kind: .profile, optimization: "-O", compilationConditions: ["PROFILE"], intendedUse: "Internal performance investigation with symbols and profiling affordances.")
    ]
}

enum ReleaseChecklistArea: String, Codable, CaseIterable, Identifiable {
    case signing
    case archive
    case notarization
    case artifacts
    case versionMetadata
    case reproducibility

    var id: String { rawValue }
}

struct ReleaseChecklistItem: Identifiable, Equatable {
    var id: String
    var area: ReleaseChecklistArea
    var title: String
    var requiredForPublicMilestone: Bool

    static let defaults = [
        ReleaseChecklistItem(id: "bundle-version", area: .versionMetadata, title: "Confirm MARKETING_VERSION and CURRENT_PROJECT_VERSION.", requiredForPublicMilestone: true),
        ReleaseChecklistItem(id: "archive-maccatalyst", area: .archive, title: "Archive Mac Catalyst build with deterministic source revision.", requiredForPublicMilestone: true),
        ReleaseChecklistItem(id: "signing-mode", area: .signing, title: "Use Developer ID signing when credentials are available, otherwise mark artifact as unsigned internal preview.", requiredForPublicMilestone: true),
        ReleaseChecklistItem(id: "notary-expectation", area: .notarization, title: "Submit and staple notarized release artifacts before broad public distribution.", requiredForPublicMilestone: false),
        ReleaseChecklistItem(id: "artifact-names", area: .artifacts, title: "Name artifacts with app, platform, architecture, version, and git revision.", requiredForPublicMilestone: true),
        ReleaseChecklistItem(id: "deterministic-evidence", area: .reproducibility, title: "Attach deterministic C test and Catalyst build results to release notes.", requiredForPublicMilestone: true)
    ]
}

enum ApeSDKRegressionTarget: String, Codable, CaseIterable, Identifiable {
    case rootTestScript
    case simulationHash
    case scriptParser
    case toolkit
    case entityImmune
    case render

    var id: String { rawValue }
}

struct ApeSDKRegressionCheck: Identifiable, Equatable {
    var id: ApeSDKRegressionTarget { target }
    var target: ApeSDKRegressionTarget
    var command: String
    var blocksRelease: Bool
    var protectsInterfaceAssumption: String

    static let defaults = [
        ApeSDKRegressionCheck(target: .rootTestScript, command: "./test.sh", blocksRelease: true, protectsInterfaceAssumption: "The interface does not hide deterministic engine breakage."),
        ApeSDKRegressionCheck(target: .simulationHash, command: "./test.sh", blocksRelease: true, protectsInterfaceAssumption: "Saved runs and replay checks are grounded in stable simulation output."),
        ApeSDKRegressionCheck(target: .scriptParser, command: "./test.sh", blocksRelease: true, protectsInterfaceAssumption: "Script validation panels match ApeScript behavior."),
        ApeSDKRegressionCheck(target: .toolkit, command: "./test.sh", blocksRelease: true, protectsInterfaceAssumption: "Bridge copies and structured data depend on stable toolkit behavior."),
        ApeSDKRegressionCheck(target: .entityImmune, command: "./test.sh", blocksRelease: true, protectsInterfaceAssumption: "Being inspection does not mask entity subsystem regressions."),
        ApeSDKRegressionCheck(target: .render, command: "./test.sh", blocksRelease: false, protectsInterfaceAssumption: "Renderer reference behavior remains explainable.")
    ]
}

enum SimulatedUniverseIssueTemplateKind: String, Codable, CaseIterable, Identifiable {
    case engineBridgeBug
    case visualInterpretationBug
    case replayBug
    case documentationGap

    var id: String { rawValue }
}

struct IssueTemplateDefinition: Identifiable, Equatable {
    var id: SimulatedUniverseIssueTemplateKind { kind }
    var kind: SimulatedUniverseIssueTemplateKind
    var fileName: String
    var labels: [String]
    var requiredEvidence: [String]

    static let defaults = [
        IssueTemplateDefinition(kind: .engineBridgeBug, fileName: "simulated_universe_engine_bridge_bug.md", labels: ["simulated-universe", "bridge"], requiredEvidence: ["ApeSDK revision", "Bridge capability list", "Command or snapshot involved"]),
        IssueTemplateDefinition(kind: .visualInterpretationBug, fileName: "simulated_universe_visual_interpretation_bug.md", labels: ["simulated-universe", "rendering"], requiredEvidence: ["Screenshot or frame description", "Snapshot fields expected", "Snapshot fields rendered"]),
        IssueTemplateDefinition(kind: .replayBug, fileName: "simulated_universe_replay_bug.md", labels: ["simulated-universe", "replay"], requiredEvidence: ["Run metadata", "Replay fidelity level", "Intervention log"]),
        IssueTemplateDefinition(kind: .documentationGap, fileName: "simulated_universe_documentation_gap.md", labels: ["simulated-universe", "documentation"], requiredEvidence: ["Missing topic", "Reader goal", "Relevant source file or reference"])
    ]
}

enum EndToEndDryRunStepKind: String, Codable, CaseIterable, Identifiable {
    case createScenario
    case selectBeing
    case followBeing
    case inspectMemory
    case applyIntervention
    case saveRun
    case replayRun
    case exportBundle

    var id: String { rawValue }
}

struct EndToEndDryRunStep: Identifiable, Equatable {
    var id: EndToEndDryRunStepKind { kind }
    var kind: EndToEndDryRunStepKind
    var expectedEvidence: String
    var blocksMilestone: Bool

    static let defaults = [
        EndToEndDryRunStep(kind: .createScenario, expectedEvidence: "Scenario metadata includes fixed seed and population.", blocksMilestone: true),
        EndToEndDryRunStep(kind: .selectBeing, expectedEvidence: "Selected being profile cites snapshot fields.", blocksMilestone: true),
        EndToEndDryRunStep(kind: .followBeing, expectedEvidence: "Camera follows without simulation mutation.", blocksMilestone: true),
        EndToEndDryRunStep(kind: .inspectMemory, expectedEvidence: "Memory panel separates evidence from interpretation.", blocksMilestone: true),
        EndToEndDryRunStep(kind: .applyIntervention, expectedEvidence: "Run log records command, cycle, and changed fields.", blocksMilestone: true),
        EndToEndDryRunStep(kind: .saveRun, expectedEvidence: "Saved run includes metadata, checksum, and intervention log.", blocksMilestone: true),
        EndToEndDryRunStep(kind: .replayRun, expectedEvidence: "Replay states exact, event-level, or visual-only fidelity.", blocksMilestone: true),
        EndToEndDryRunStep(kind: .exportBundle, expectedEvidence: "Export contains manifest, run log, snapshots, and notes.", blocksMilestone: false)
    ]
}

enum ReadmeGoal: String, Codable, CaseIterable, Identifiable {
    case livingWorldFirst
    case overviewPlusIntimacy
    case explicitInterventions
    case scientificCredibility

    var id: String { rawValue }
}

struct ProductGoalReviewItem: Identifiable, Equatable {
    var id: ReadmeGoal { goal }
    var goal: ReadmeGoal
    var passCondition: String
    var firstMilestoneStatus: String

    static let defaults = [
        ProductGoalReviewItem(goal: .livingWorldFirst, passCondition: "The first screen is the running world, not a landing page.", firstMilestoneStatus: "Keep."),
        ProductGoalReviewItem(goal: .overviewPlusIntimacy, passCondition: "Users can move from population view to selected being evidence.", firstMilestoneStatus: "Keep compact profile, follow, terrain, memory summary, and social summary."),
        ProductGoalReviewItem(goal: .explicitInterventions, passCondition: "Every state-changing action enters the run log.", firstMilestoneStatus: "Keep weather/resource/script commands only when logged."),
        ProductGoalReviewItem(goal: .scientificCredibility, passCondition: "Panels and overlays cite ApeSDK state rather than inventing behavior.", firstMilestoneStatus: "Keep source-field provenance visible.")
    ]
}

enum MilestoneScopeDecision: String, Codable, CaseIterable, Identifiable {
    case keep
    case deferToLater
    case cut

    var id: String { rawValue }
}

struct PublicMilestoneScopeItem: Identifiable, Equatable {
    var id: String
    var title: String
    var decision: MilestoneScopeDecision
    var rationale: String

    static let defaults = [
        PublicMilestoneScopeItem(id: "living-world", title: "Living world view", decision: .keep, rationale: "This is the central promise of the interface."),
        PublicMilestoneScopeItem(id: "selection-follow", title: "Selection and follow", decision: .keep, rationale: "This creates the personal thread through the simulation."),
        PublicMilestoneScopeItem(id: "profile-memory-social", title: "Profile, memory, and social panels", decision: .keep, rationale: "These prove overview plus intimacy."),
        PublicMilestoneScopeItem(id: "logged-interventions", title: "Small logged interventions", decision: .keep, rationale: "Weather, resources, and script loading establish explicit experiment framing."),
        PublicMilestoneScopeItem(id: "exact-replay", title: "Exact replay", decision: .deferToLater, rationale: "Keep metadata and timeline planning, but do not block the first credible preview on full replay fidelity."),
        PublicMilestoneScopeItem(id: "external-plugins", title: "External plugin API", decision: .cut, rationale: "The data contracts need to settle before public extension points."),
        PublicMilestoneScopeItem(id: "advanced-brain-visualization", title: "Advanced brain visualization", decision: .deferToLater, rationale: "Requires confirmed bridge support and careful evidence design.")
    ]
}

struct FinalDevelopmentCycleReview: Equatable {
    var developerDocs: [DeveloperDocumentationPlan]
    var onboarding: [OnboardingInteractionStep]
    var sampleScenarios: [SampleScenarioPlan]
    var buildSettings: [SimulatedUniverseBuildSettingPlan]
    var releaseChecklist: [ReleaseChecklistItem]
    var regressionChecks: [ApeSDKRegressionCheck]
    var issueTemplates: [IssueTemplateDefinition]
    var dryRun: [EndToEndDryRunStep]
    var productGoals: [ProductGoalReviewItem]
    var milestoneScope: [PublicMilestoneScopeItem]

    var firstPublicMilestoneKeeps: [String] {
        milestoneScope.filter { $0.decision == .keep }.map(\.title)
    }

    static let initial = FinalDevelopmentCycleReview(
        developerDocs: DeveloperDocumentationPlan.defaults,
        onboarding: OnboardingInteractionStep.defaults,
        sampleScenarios: SampleScenarioPlan.defaults,
        buildSettings: SimulatedUniverseBuildSettingPlan.defaults,
        releaseChecklist: ReleaseChecklistItem.defaults,
        regressionChecks: ApeSDKRegressionCheck.defaults,
        issueTemplates: IssueTemplateDefinition.defaults,
        dryRun: EndToEndDryRunStep.defaults,
        productGoals: ProductGoalReviewItem.defaults,
        milestoneScope: PublicMilestoneScopeItem.defaults
    )
}
