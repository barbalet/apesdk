import Foundation

enum QualityTestArea: String, Codable, CaseIterable, Identifiable {
    case bridge
    case runtime
    case commands
    case snapshots
    case runMetadata
    case uiModel
    case renderer
    case performance
    case memory
    case accessibility
    case catalyst
    case crashHandling
    case logging
    case documentation

    var id: String { rawValue }
}

struct DeterministicTestTargetPlan: Identifiable, Equatable {
    var id: String
    var areas: Set<QualityTestArea>
    var fixtureName: String
    var mustRunWithoutUI: Bool
    var fixedSeed: UInt64
    var requiredPasses: Int

    var isStrict: Bool {
        mustRunWithoutUI && requiredPasses > 1
    }

    static let bridgeRuntimeSmoke = DeterministicTestTargetPlan(
        id: "bridge-runtime-smoke",
        areas: [.bridge, .runtime, .commands, .snapshots],
        fixtureName: BridgeTestPlan.initialDeterministic.fixture.id,
        mustRunWithoutUI: true,
        fixedSeed: DeterministicScenarioProfile.defaultObservation.seed,
        requiredPasses: 5
    )

    static let persistenceReplaySmoke = DeterministicTestTargetPlan(
        id: "persistence-replay-smoke",
        areas: [.runMetadata, .snapshots],
        fixtureName: DeterministicReplayTestCase.noInterventionSmoke.id,
        mustRunWithoutUI: true,
        fixedSeed: DeterministicReplayTestCase.noInterventionSmoke.fixedSeed,
        requiredPasses: 5
    )
}

enum UIModelExpectation: String, Codable, CaseIterable, Identifiable {
    case selectionPersistsAcrossModes
    case followUpdatesCamera
    case compareKeepsPinnedBeings
    case panelFilteringIsStable
    case commandControlsMatchRuntimePhase

    var id: String { rawValue }
}

struct UIModelTestCase: Identifiable, Equatable {
    var id: String
    var startingMode: UniverseMode
    var endingMode: UniverseMode
    var enabledOverlays: Set<UniverseOverlay>
    var expectations: Set<UIModelExpectation>

    static let defaults = [
        UIModelTestCase(
            id: "inspect-selection-to-replay",
            startingMode: .inspect,
            endingMode: .replay,
            enabledOverlays: [.memory, .social],
            expectations: [.selectionPersistsAcrossModes, .compareKeepsPinnedBeings, .panelFilteringIsStable]
        ),
        UIModelTestCase(
            id: "follow-selection-while-running",
            startingMode: .explore,
            endingMode: .explore,
            enabledOverlays: [.weather, .resources],
            expectations: [.followUpdatesCamera, .commandControlsMatchRuntimePhase]
        )
    ]
}

enum RendererVerificationExpectation: String, Codable, CaseIterable, Identifiable {
    case mapToScreenRoundTrip
    case overlayOrdering
    case terrainSampling
    case beingHitTesting
    case snapshotSceneAgreement

    var id: String { rawValue }
}

struct RendererVerificationTestCase: Identifiable, Equatable {
    var id: String
    var fixture: RendererPerformanceFixture
    var overlayOrder: [RenderOverlayKind]
    var expectations: Set<RendererVerificationExpectation>
    var maximumMismatchedProbes: Int

    static let largePopulation = RendererVerificationTestCase(
        id: "large-population-render-verification",
        fixture: .largePopulationHighZoom,
        overlayOrder: OverlayCompositionPlan.make(enabledOverlays: [.resources, .weather, .social, .memory]).orderedKinds,
        expectations: [.mapToScreenRoundTrip, .overlayOrdering, .terrainSampling, .beingHitTesting, .snapshotSceneAgreement],
        maximumMismatchedProbes: 0
    )
}

enum PerformanceBudgetKind: String, Codable, CaseIterable, Identifiable {
    case frameTime
    case snapshotExtraction
    case commandLatency
    case replayScrub
    case scriptValidation

    var id: String { rawValue }
}

struct PerformanceBudget: Identifiable, Equatable {
    var id: PerformanceBudgetKind { kind }
    var kind: PerformanceBudgetKind
    var maximumMilliseconds: Double
    var percentile: Int
}

struct PerformanceTestScenario: Identifiable, Equatable {
    var id: String
    var cycleCount: Int
    var populationCount: Int
    var overlayCount: Int
    var budgets: [PerformanceBudget]
    var repeats: Int

    static let interactiveLargeWorld = PerformanceTestScenario(
        id: "interactive-large-world",
        cycleCount: 3_600,
        populationCount: 512,
        overlayCount: 5,
        budgets: [
            PerformanceBudget(kind: .frameTime, maximumMilliseconds: 16.67, percentile: 95),
            PerformanceBudget(kind: .snapshotExtraction, maximumMilliseconds: 10.0, percentile: 95),
            PerformanceBudget(kind: .commandLatency, maximumMilliseconds: 30.0, percentile: 99),
            PerformanceBudget(kind: .replayScrub, maximumMilliseconds: 50.0, percentile: 95),
            PerformanceBudget(kind: .scriptValidation, maximumMilliseconds: 250.0, percentile: 99)
        ],
        repeats: 3
    )
}

enum MemoryRetentionTarget: String, Codable, CaseIterable, Identifiable {
    case snapshotArchive
    case replayBuffer
    case renderModels
    case scriptOutput
    case bridgeCopies

    var id: String { rawValue }
}

struct MemoryTestScenario: Identifiable, Equatable {
    var id: String
    var target: MemoryRetentionTarget
    var maximumBytes: Int
    var prunePolicy: SnapshotArchivePruningPolicy?
    var expectedRecovery: String

    static let defaults = [
        MemoryTestScenario(
            id: "snapshot-archive-pruning",
            target: .snapshotArchive,
            maximumBytes: SnapshotArchivePruningPolicy.longSession.maximumArchiveBytes,
            prunePolicy: .longSession,
            expectedRecovery: "Prune old unbookmarked snapshots while preserving checkpoints."
        ),
        MemoryTestScenario(
            id: "script-output-cap",
            target: .scriptOutput,
            maximumBytes: ScriptExecutionLimitPolicy.interactive.maximumOutputBytes,
            prunePolicy: nil,
            expectedRecovery: "Stop appending script output and surface a capped-output diagnostic."
        ),
        MemoryTestScenario(
            id: "bridge-copy-lifetime",
            target: .bridgeCopies,
            maximumBytes: 32_000_000,
            prunePolicy: nil,
            expectedRecovery: "Release copied bridge data after snapshot models are built."
        )
    ]
}

enum AccessibilityCoverageTarget: String, Codable, CaseIterable, Identifiable {
    case selectedBeing
    case modeSwitcher
    case timeline
    case inspectorPanels
    case overlayControls
    case scriptEditor

    var id: String { rawValue }
}

struct AccessibilityTestCase: Identifiable, Equatable {
    var id: String
    var target: AccessibilityCoverageTarget
    var requiresLabel: Bool
    var requiresKeyboardPath: Bool
    var respectsReducedMotion: Bool
    var minimumContrastRatio: Double

    static let defaults = [
        AccessibilityTestCase(
            id: "selected-being-summary",
            target: .selectedBeing,
            requiresLabel: true,
            requiresKeyboardPath: true,
            respectsReducedMotion: true,
            minimumContrastRatio: 4.5
        ),
        AccessibilityTestCase(
            id: "replay-timeline",
            target: .timeline,
            requiresLabel: true,
            requiresKeyboardPath: true,
            respectsReducedMotion: true,
            minimumContrastRatio: 4.5
        ),
        AccessibilityTestCase(
            id: "script-editor",
            target: .scriptEditor,
            requiresLabel: true,
            requiresKeyboardPath: true,
            respectsReducedMotion: true,
            minimumContrastRatio: 4.5
        )
    ]
}

enum CatalystReadinessArea: String, Codable, CaseIterable, Identifiable {
    case resizableWindows
    case toolbarCommands
    case menus
    case fileImportExport
    case keyboardShortcuts
    case dragAndDrop

    var id: String { rawValue }
}

struct CatalystReadinessCheck: Identifiable, Equatable {
    var id: String
    var area: CatalystReadinessArea
    var expectedBehavior: String
    var blocksPrototypeMilestone: Bool

    static let defaults = [
        CatalystReadinessCheck(
            id: "resizable-world-window",
            area: .resizableWindows,
            expectedBehavior: "World, inspector, and replay controls remain usable at compact and wide desktop sizes.",
            blocksPrototypeMilestone: true
        ),
        CatalystReadinessCheck(
            id: "save-open-run-files",
            area: .fileImportExport,
            expectedBehavior: "Saved runs, exported bundles, and imported replay files use native document affordances.",
            blocksPrototypeMilestone: true
        ),
        CatalystReadinessCheck(
            id: "mode-menu-commands",
            area: .menus,
            expectedBehavior: "Explore, Inspect, Experiment, and Replay commands mirror visible toolbar state.",
            blocksPrototypeMilestone: false
        )
    ]
}

enum DiagnosticCaptureSource: String, Codable, CaseIterable, Identifiable {
    case bridge
    case runtime
    case renderer
    case persistence
    case script
    case userAction

    var id: String { rawValue }
}

struct DiagnosticCapturePlan: Identifiable, Equatable {
    var id: String
    var source: DiagnosticCaptureSource
    var redactsLocalPaths: Bool
    var includesRunMetadata: Bool
    var includesRecentCommands: Bool
    var retentionLimit: Int

    static let defaults = [
        DiagnosticCapturePlan(id: "bridge-failures", source: .bridge, redactsLocalPaths: true, includesRunMetadata: true, includesRecentCommands: true, retentionLimit: 200),
        DiagnosticCapturePlan(id: "renderer-failures", source: .renderer, redactsLocalPaths: true, includesRunMetadata: true, includesRecentCommands: false, retentionLimit: 100),
        DiagnosticCapturePlan(id: "save-load-failures", source: .persistence, redactsLocalPaths: true, includesRunMetadata: true, includesRecentCommands: true, retentionLimit: 100),
        DiagnosticCapturePlan(id: "script-failures", source: .script, redactsLocalPaths: true, includesRunMetadata: true, includesRecentCommands: true, retentionLimit: 200)
    ]
}

enum UniverseLoggingLevel: Int, Codable, CaseIterable, Identifiable {
    case user = 0
    case experiment = 1
    case diagnostic = 2
    case developerTrace = 3

    var id: Int { rawValue }
}

struct LoggingChannelPolicy: Identifiable, Equatable {
    var id: String
    var level: UniverseLoggingLevel
    var visibleInRunLog: Bool
    var exportedByDefault: Bool
    var maximumEntries: Int

    static let defaults = [
        LoggingChannelPolicy(id: "user-actions", level: .user, visibleInRunLog: true, exportedByDefault: true, maximumEntries: 1_000),
        LoggingChannelPolicy(id: "experiment-events", level: .experiment, visibleInRunLog: true, exportedByDefault: true, maximumEntries: 5_000),
        LoggingChannelPolicy(id: "diagnostics", level: .diagnostic, visibleInRunLog: false, exportedByDefault: false, maximumEntries: 2_000),
        LoggingChannelPolicy(id: "developer-trace", level: .developerTrace, visibleInRunLog: false, exportedByDefault: false, maximumEntries: 500)
    ]
}

enum AppDocumentationAudience: String, Codable, CaseIterable, Identifiable {
    case learner
    case researcher
    case developer

    var id: String { rawValue }
}

struct AppDocumentationTopic: Identifiable, Equatable {
    var id: String
    var title: String
    var mode: UniverseMode?
    var audience: AppDocumentationAudience
    var proves: [String]
    var linksToReferenceDocs: Bool

    static let defaults = [
        AppDocumentationTopic(
            id: "explore-mode",
            title: "Explore Mode",
            mode: .explore,
            audience: .learner,
            proves: ["The visible world comes from ApeSDK snapshots.", "Camera movement does not mutate the simulation."],
            linksToReferenceDocs: true
        ),
        AppDocumentationTopic(
            id: "inspect-mode",
            title: "Inspect Mode",
            mode: .inspect,
            audience: .researcher,
            proves: ["Selected being panels cite snapshot fields.", "Memory and social evidence remain separated from interpretation."],
            linksToReferenceDocs: true
        ),
        AppDocumentationTopic(
            id: "experiment-mode",
            title: "Experiment Mode",
            mode: .experiment,
            audience: .researcher,
            proves: ["Interventions are logged.", "Scripts declare deterministic limits and diagnostics."],
            linksToReferenceDocs: true
        ),
        AppDocumentationTopic(
            id: "replay-mode",
            title: "Replay Mode",
            mode: .replay,
            audience: .developer,
            proves: ["Replay fidelity is explicit.", "Saved runs record engine version, seed, compile flags, and command order."],
            linksToReferenceDocs: true
        )
    ]
}

struct QualityReadinessReview: Equatable {
    var deterministicPlans: [DeterministicTestTargetPlan]
    var uiCases: [UIModelTestCase]
    var rendererCases: [RendererVerificationTestCase]
    var performanceScenarios: [PerformanceTestScenario]
    var memoryScenarios: [MemoryTestScenario]
    var accessibilityCases: [AccessibilityTestCase]
    var catalystChecks: [CatalystReadinessCheck]
    var diagnostics: [DiagnosticCapturePlan]
    var logging: [LoggingChannelPolicy]
    var documentationTopics: [AppDocumentationTopic]

    var blocksPrototype: [String] {
        catalystChecks.filter(\.blocksPrototypeMilestone).map(\.id)
    }

    static let initial = QualityReadinessReview(
        deterministicPlans: [.bridgeRuntimeSmoke, .persistenceReplaySmoke],
        uiCases: UIModelTestCase.defaults,
        rendererCases: [.largePopulation],
        performanceScenarios: [.interactiveLargeWorld],
        memoryScenarios: MemoryTestScenario.defaults,
        accessibilityCases: AccessibilityTestCase.defaults,
        catalystChecks: CatalystReadinessCheck.defaults,
        diagnostics: DiagnosticCapturePlan.defaults,
        logging: LoggingChannelPolicy.defaults,
        documentationTopics: AppDocumentationTopic.defaults
    )
}
