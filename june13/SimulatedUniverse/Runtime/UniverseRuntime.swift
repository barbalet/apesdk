import Combine
import Foundation

enum RuntimePhase: String, Codable, CaseIterable, Identifiable {
    case uninitialized
    case initializing
    case running
    case paused
    case stepping
    case suspended
    case shuttingDown
    case stopped
    case failed

    var id: String { rawValue }

    var title: String {
        switch self {
        case .uninitialized:
            return "Uninitialized"
        case .initializing:
            return "Initializing"
        case .running:
            return "Running"
        case .paused:
            return "Paused"
        case .stepping:
            return "Stepping"
        case .suspended:
            return "Suspended"
        case .shuttingDown:
            return "Shutting Down"
        case .stopped:
            return "Stopped"
        case .failed:
            return "Failed"
        }
    }
}

enum RuntimeClockMode: Equatable {
    case paused
    case realtime(targetCyclesPerSecond: Int)
    case singleStep(cycles: Int)
    case accelerated(multiplier: Int)
    case fixedBatch(cycles: Int)

    var runMode: RunMode {
        switch self {
        case .paused:
            return .paused
        case .realtime:
            return .realtime
        case .singleStep:
            return .singleStep
        case .accelerated:
            return .accelerated
        case .fixedBatch:
            return .fixedBatch
        }
    }

    var command: SimulationCommand {
        switch self {
        case .paused:
            return .pause
        case .realtime:
            return .resume
        case .singleStep(let cycles), .fixedBatch(let cycles):
            return .step(cycles: max(1, cycles))
        case .accelerated(let multiplier):
            return .step(cycles: max(1, multiplier))
        }
    }
}

enum RunMode: String, Codable, CaseIterable, Identifiable {
    case paused
    case realtime
    case singleStep
    case accelerated
    case fixedBatch
    case replay

    var id: String { rawValue }
}

enum RuntimeSeedSource: String, Codable, CaseIterable, Identifiable {
    case defaultProfile
    case fixedSeed
    case transferMetadata
    case replayMetadata

    var id: String { rawValue }
}

struct RuntimeSeedConfiguration: Codable, Equatable, Identifiable {
    var source: RuntimeSeedSource
    var resolvedSeed: UInt64
    var reproducibilityNote: String

    var id: String {
        "\(source.rawValue)-\(resolvedSeed)"
    }

    static let defaultObservation = RuntimeSeedConfiguration(
        source: .defaultProfile,
        resolvedSeed: DeterministicScenarioProfile.defaultObservation.seed,
        reproducibilityNote: "Uses the default observation profile until the user selects a fixed seed, replay, or transfer file."
    )
}

enum ScenarioLoadSource: Equatable {
    case defaultWorld
    case fixedSeed(UInt64)
    case transferFile(URL)

    var seedSource: RuntimeSeedSource {
        switch self {
        case .defaultWorld:
            return .defaultProfile
        case .fixedSeed:
            return .fixedSeed
        case .transferFile:
            return .transferMetadata
        }
    }
}

struct ScenarioLoadRequest: Equatable {
    var source: ScenarioLoadSource
    var profile: DeterministicScenarioProfile
    var startsPausedOverride: Bool?

    var resolvedSeed: UInt64 {
        switch source {
        case .defaultWorld:
            return profile.seed
        case .fixedSeed(let seed):
            return seed
        case .transferFile:
            return profile.seed
        }
    }

    var startsPaused: Bool {
        startsPausedOverride ?? profile.startsPaused
    }

    var descriptor: ScenarioDescriptor {
        ScenarioDescriptor(
            seed: resolvedSeed,
            populationCount: profile.populationCount,
            startsPaused: startsPaused
        )
    }

    var seedConfiguration: RuntimeSeedConfiguration {
        RuntimeSeedConfiguration(
            source: source.seedSource,
            resolvedSeed: resolvedSeed,
            reproducibilityNote: "Scenario \(profile.name) resolves to seed \(resolvedSeed)."
        )
    }

    static let defaultObservation = ScenarioLoadRequest(
        source: .defaultWorld,
        profile: .defaultObservation,
        startsPausedOverride: nil
    )
}

enum ScenarioLoaderOption: String, CaseIterable, Identifiable {
    case defaultWorld
    case fixedSeedWorld
    case transferFile

    var id: String { rawValue }

    var acceptanceRule: String {
        switch self {
        case .defaultWorld:
            return "Start from the built-in deterministic observation profile."
        case .fixedSeedWorld:
            return "Start from the same profile but replace the seed before engine initialization."
        case .transferFile:
            return "Validate transfer metadata before replacing the current run."
        }
    }
}

struct ScenarioLoaderPlan: Equatable {
    var acceptedOptions: [ScenarioLoaderOption]
    var validatesContractVersion: Bool
    var pausesBeforeReplacement: Bool
    var preservesFailedRunSnapshot: Bool

    static let minimal = ScenarioLoaderPlan(
        acceptedOptions: [.defaultWorld, .fixedSeedWorld, .transferFile],
        validatesContractVersion: true,
        pausesBeforeReplacement: true,
        preservesFailedRunSnapshot: true
    )
}

enum SnapshotPublicationKind: String, CaseIterable, Identifiable {
    case universe
    case selectedBeing
    case terrain
    case memory
    case social
    case events

    var id: String { rawValue }
}

struct SnapshotThrottlePolicy: Equatable {
    var universeCycleInterval: Int
    var selectedBeingCycleInterval: Int
    var terrainCycleInterval: Int
    var panelMinimumInterval: TimeInterval

    func shouldPublish(kind: SnapshotPublicationKind, lastCycle: Int, currentCycle: Int, elapsedSinceLastPanelUpdate: TimeInterval) -> Bool {
        switch kind {
        case .universe:
            return currentCycle - lastCycle >= max(1, universeCycleInterval)
        case .selectedBeing:
            return currentCycle - lastCycle >= max(1, selectedBeingCycleInterval)
        case .terrain:
            return currentCycle - lastCycle >= max(1, terrainCycleInterval)
        case .memory, .social, .events:
            return elapsedSinceLastPanelUpdate >= panelMinimumInterval
        }
    }

    static let interactive = SnapshotThrottlePolicy(
        universeCycleInterval: 1,
        selectedBeingCycleInterval: 4,
        terrainCycleInterval: 8,
        panelMinimumInterval: 0.15
    )
}

struct RuntimeExecutionModel: Equatable {
    var simulationQueueLabel: String
    var snapshotQueueLabel: String
    var publishesOnMainActor: Bool
    var maximumCyclesPerBatch: Int
    var cancellationGraceMilliseconds: Int

    static let catalystDefault = RuntimeExecutionModel(
        simulationQueueLabel: "com.nobleape.simulated-universe.simulation",
        snapshotQueueLabel: "com.nobleape.simulated-universe.snapshots",
        publishesOnMainActor: true,
        maximumCyclesPerBatch: 120,
        cancellationGraceMilliseconds: 250
    )
}

enum RuntimeLifecycleEvent: String, CaseIterable, Identifiable {
    case appLaunch
    case sceneBecameActive
    case sceneResignedActive
    case sceneEnteredBackground
    case windowClosed
    case windowReopened
    case userRequestedShutdown

    var id: String { rawValue }
}

struct RuntimeLifecycleRule: Identifiable, Equatable {
    var id: RuntimeLifecycleEvent { event }
    var event: RuntimeLifecycleEvent
    var resultingPhase: RuntimePhase
    var rule: String

    static let catalystRules = [
        RuntimeLifecycleRule(
            event: .appLaunch,
            resultingPhase: .initializing,
            rule: "Create a runtime controller and load the default scenario once the first scene appears."
        ),
        RuntimeLifecycleRule(
            event: .sceneResignedActive,
            resultingPhase: .paused,
            rule: "Pause stepping but preserve the current snapshot and run metadata."
        ),
        RuntimeLifecycleRule(
            event: .sceneEnteredBackground,
            resultingPhase: .suspended,
            rule: "Cancel scheduled stepping and keep only copied Swift state."
        ),
        RuntimeLifecycleRule(
            event: .windowClosed,
            resultingPhase: .shuttingDown,
            rule: "Stop command submission, destroy the facade context, and leave no borrowed engine memory."
        ),
        RuntimeLifecycleRule(
            event: .windowReopened,
            resultingPhase: .initializing,
            rule: "Restore UI preferences and reload a scenario from metadata or a transfer file."
        )
    ]
}

struct RunMetadata: Codable, Equatable, Identifiable {
    var id: UUID
    var scenarioName: String
    var seed: UInt64
    var buildVersion: String
    var startedAt: Date
    var currentCycle: Int
    var mode: RunMode
    var interventionCount: Int

    mutating func advance(to cycle: Int, mode: RunMode) {
        currentCycle = cycle
        self.mode = mode
    }

    mutating func recordIntervention() {
        interventionCount += 1
    }
}

struct SimulationHealthSnapshot: Codable, Equatable {
    var frameLatencyMilliseconds: Double
    var cycleLatencyMilliseconds: Double
    var droppedSnapshotCount: Int
    var bridgeErrorCount: Int
    var lastBridgeErrorCode: BridgeErrorCode?

    var isHealthy: Bool {
        frameLatencyMilliseconds <= 20.0 &&
        cycleLatencyMilliseconds <= 8.0 &&
        bridgeErrorCount == 0
    }

    mutating func recordStep(cycleLatencyMilliseconds: Double) {
        self.cycleLatencyMilliseconds = cycleLatencyMilliseconds
    }

    mutating func recordDroppedSnapshot() {
        droppedSnapshotCount += 1
    }

    mutating func recordBridgeError(_ code: BridgeErrorCode) {
        bridgeErrorCount += 1
        lastBridgeErrorCode = code
    }

    static let initial = SimulationHealthSnapshot(
        frameLatencyMilliseconds: 0.0,
        cycleLatencyMilliseconds: 0.0,
        droppedSnapshotCount: 0,
        bridgeErrorCount: 0,
        lastBridgeErrorCode: nil
    )
}

protocol UniverseBridgeClient: AnyObject {
    func initialize(with request: ScenarioLoadRequest) throws -> UniverseSnapshot
    func step(cycles: Int) throws -> UniverseSnapshot
    func sampleTerrain(_ coordinate: MapCoordinate) throws -> TerrainSampleSnapshot
    func shutdown()
}

final class FixtureUniverseBridgeClient: UniverseBridgeClient {
    private var request = ScenarioLoadRequest.defaultObservation
    private var cycle = 0

    func initialize(with request: ScenarioLoadRequest) throws -> UniverseSnapshot {
        self.request = request
        cycle = request.profile.startCycle
        return makeSnapshot()
    }

    func step(cycles: Int) throws -> UniverseSnapshot {
        guard cycles >= 0 else {
            throw UniverseBridgeError.commandRejected(.step(cycles: cycles))
        }

        cycle += cycles
        return makeSnapshot()
    }

    func sampleTerrain(_ coordinate: MapCoordinate) throws -> TerrainSampleSnapshot {
        guard coordinate.x >= 0 && coordinate.y >= 0 else {
            throw UniverseBridgeError.invalidTerrainSample(coordinate)
        }

        let seed = request.resolvedSeed
        let value = Int((UInt64(coordinate.x * 31 + coordinate.y * 17) &+ seed &+ UInt64(max(0, cycle))) % 255)
        return TerrainSampleSnapshot(
            coordinate: coordinate,
            height: value,
            waterLevel: 96,
            isUnderWater: value < 96,
            weather: makeSnapshot().weather,
            resource: ResourceSnapshot(classification: value.isMultiple(of: 5) ? "food" : "terrain", density: value % 16, rawValue: value),
            nearbyBeingIDs: []
        )
    }

    func shutdown() {
        cycle = 0
    }

    private func makeSnapshot() -> UniverseSnapshot {
        let seed = request.resolvedSeed
        let cycleValue = UInt64(max(0, cycle))
        let windX = Int((seed &+ cycleValue) % 9) - 4
        let windY = Int(((seed >> 8) &+ cycleValue) % 9) - 4
        let weather = WeatherSnapshot(
            condition: Int((seed ^ cycleValue) % 12),
            pressure: 980 + Int((seed &+ cycleValue) % 50),
            wind: MapVector(x: windX, y: windY),
            lightningActive: (seed &+ cycleValue).isMultiple(of: 97)
        )

        return UniverseSnapshot(
            contractVersion: SnapshotContract.currentVersion,
            cycle: cycle,
            landDate: cycle / 1440,
            landTime: cycle % 1440,
            weather: weather,
            tide: TideSnapshot(level: Int((seed &+ cycleValue) % 32)),
            selectedBeingID: nil,
            population: PopulationSnapshot(
                count: request.profile.populationCount,
                max: max(256, request.profile.populationCount),
                selectedIndex: nil
            ),
            map: .defaultApeSDK
        )
    }
}

@MainActor
final class UniverseRuntimeController: ObservableObject {
    @Published private(set) var phase: RuntimePhase = .uninitialized
    @Published private(set) var snapshot: UniverseSnapshot = .empty
    @Published private(set) var metadata: RunMetadata?
    @Published private(set) var health: SimulationHealthSnapshot = .initial
    @Published private(set) var lastError: UniverseBridgeError?
    @Published private(set) var pauseReason: RuntimePauseReason?
    @Published private(set) var clockMode: RuntimeClockMode = .paused
    @Published private(set) var selection: BeingSelectionState = .empty
    @Published private(set) var terrainSample: TerrainSampleSnapshot?
    @Published private(set) var runLog: [RunLogRecord] = []
    @Published private(set) var debugMetrics: RuntimeDebugMetrics = .empty

    let executionModel: RuntimeExecutionModel
    let throttlePolicy: SnapshotThrottlePolicy
    let loaderPlan: ScenarioLoaderPlan
    let pauseOnErrorPolicy: PauseOnErrorPolicy

    private let bridge: UniverseBridgeClient
    private let now: () -> Date
    private var submittedCommandCount = 0
    private var unattendedCyclesCompleted = 0

    init(
        bridge: UniverseBridgeClient = FixtureUniverseBridgeClient(),
        executionModel: RuntimeExecutionModel = .catalystDefault,
        throttlePolicy: SnapshotThrottlePolicy = .interactive,
        loaderPlan: ScenarioLoaderPlan = .minimal,
        pauseOnErrorPolicy: PauseOnErrorPolicy = .deterministic,
        now: @escaping () -> Date = Date.init
    ) {
        self.bridge = bridge
        self.executionModel = executionModel
        self.throttlePolicy = throttlePolicy
        self.loaderPlan = loaderPlan
        self.pauseOnErrorPolicy = pauseOnErrorPolicy
        self.now = now
        refreshDebugMetrics()
    }

    func initialize(with request: ScenarioLoadRequest = .defaultObservation) {
        guard phase == .uninitialized || phase == .stopped || phase == .failed else {
            return
        }

        phase = .initializing
        do {
            let initializedSnapshot = try bridge.initialize(with: request)
            snapshot = initializedSnapshot
            selection = .empty(at: initializedSnapshot.cycle)
            terrainSample = nil
            runLog = []
            health = .initial
            clockMode = request.startsPaused ? .paused : .realtime(targetCyclesPerSecond: 60)
            metadata = RunMetadata(
                id: UUID(),
                scenarioName: request.profile.name,
                seed: request.resolvedSeed,
                buildVersion: BridgeVersionHandshake.developmentDefault.apeSDKShortVersionName,
                startedAt: now(),
                currentCycle: initializedSnapshot.cycle,
                mode: clockMode.runMode,
                interventionCount: 0
            )
            lastError = nil
            pauseReason = nil
            phase = request.startsPaused ? .paused : .running
            refreshDebugMetrics()
        } catch let error as UniverseBridgeError {
            recordFailure(error)
        } catch {
            recordFailure(.initializationFailed(String(describing: error)))
        }
    }

    func loadScenario(_ request: ScenarioLoadRequest) {
        if loaderPlan.pausesBeforeReplacement {
            pause()
        }

        phase = .uninitialized
        initialize(with: request)
    }

    func pause() {
        guard phase == .running || phase == .stepping || phase == .suspended else {
            return
        }

        clockMode = .paused
        metadata?.advance(to: snapshot.cycle, mode: .paused)
        phase = .paused
        refreshDebugMetrics()
    }

    func resume() {
        if phase == .uninitialized {
            initialize()
            return
        }

        guard phase == .paused || phase == .suspended else {
            return
        }

        clockMode = .realtime(targetCyclesPerSecond: 60)
        metadata?.advance(to: snapshot.cycle, mode: .realtime)
        phase = .running
        refreshDebugMetrics()
    }

    func step(cycles: Int = 1) {
        if phase == .uninitialized {
            initialize(with: ScenarioLoadRequest.defaultObservation)
        }

        guard phase == .paused || phase == .running else {
            return
        }

        clockMode = .singleStep(cycles: max(1, cycles))
        phase = .stepping
        let start = now()

        do {
            let steppedSnapshot = try bridge.step(cycles: max(1, cycles))
            snapshot = steppedSnapshot
            selection.advance(to: steppedSnapshot.cycle)
            health.recordStep(cycleLatencyMilliseconds: now().timeIntervalSince(start) * 1000.0)
            metadata?.advance(to: steppedSnapshot.cycle, mode: .singleStep)
            lastError = nil
            pauseReason = nil
            phase = .paused
            refreshDebugMetrics()
        } catch let error as UniverseBridgeError {
            recordFailure(error)
        } catch {
            recordFailure(.simulationUnavailable)
        }
    }

    func handleLifecycleEvent(_ event: RuntimeLifecycleEvent) {
        switch event {
        case .appLaunch, .sceneBecameActive, .windowReopened:
            if phase == .uninitialized || phase == .stopped {
                initialize()
            }
        case .sceneResignedActive:
            pause()
        case .sceneEnteredBackground:
            pause()
            phase = .suspended
        case .windowClosed, .userRequestedShutdown:
            shutdown()
        }
    }

    func shutdown() {
        guard phase != .stopped && phase != .uninitialized else {
            return
        }

        phase = .shuttingDown
        bridge.shutdown()
        clockMode = .paused
        metadata?.advance(to: snapshot.cycle, mode: .paused)
        phase = .stopped
        refreshDebugMetrics()
    }

    func submit(_ command: RuntimeSelectionCommand) {
        submittedCommandCount += 1

        switch command {
        case .selectBeing(let id):
            selection.select(id, follow: false, cycle: snapshot.cycle)
        case .followBeing(let id):
            selection.follow(id, cycle: snapshot.cycle)
        case .clearSelection:
            selection.clear(reason: .userCleared, cycle: snapshot.cycle)
        case .sampleTerrain(let coordinate):
            do {
                terrainSample = try bridge.sampleTerrain(coordinate)
                lastError = nil
            } catch let error as UniverseBridgeError {
                recordFailure(error)
            } catch {
                recordFailure(.invalidTerrainSample(coordinate))
            }
        }

        refreshDebugMetrics()
    }

    func submit(_ command: RuntimeControlCommand) {
        submittedCommandCount += 1

        switch command {
        case .pause:
            pause()
        case .resume:
            resume()
        case .step(let cycles):
            step(cycles: cycles)
        case .reset(let request), .loadScenario(let request):
            loadScenario(request)
        }

        refreshDebugMetrics()
    }

    func submit(_ command: RuntimeInterventionCommand) {
        submittedCommandCount += 1
        metadata?.recordIntervention()
        runLog.append(.intervention(command, cycle: snapshot.cycle, createdAt: now()))
        refreshDebugMetrics()
    }

    func runUnattended(_ plan: UnattendedRuntimePlan = .deterministicSmoke) {
        if phase == .uninitialized || phase == .stopped {
            initialize(with: plan.scenario)
        }

        guard phase == .paused || phase == .running else {
            return
        }

        let targetCycle = snapshot.cycle + max(0, plan.targetCycles)
        clockMode = .fixedBatch(cycles: plan.targetCycles)
        phase = .running

        do {
            while snapshot.cycle < targetCycle {
                let remaining = targetCycle - snapshot.cycle
                let batch = min(max(1, plan.maximumBatchSize), remaining)
                snapshot = try bridge.step(cycles: batch)
            }

            unattendedCyclesCompleted += plan.targetCycles
            metadata?.advance(to: snapshot.cycle, mode: .fixedBatch)
            runLog.append(.runtime(
                commandName: "runUnattended",
                summary: "Completed \(plan.targetCycles) deterministic cycles without UI interaction.",
                cycle: snapshot.cycle,
                createdAt: now()
            ))
            phase = .paused
            clockMode = .paused
            refreshDebugMetrics()
        } catch let error as UniverseBridgeError {
            recordFailure(error)
        } catch {
            recordFailure(.simulationUnavailable)
        }
    }

    private func recordFailure(_ error: UniverseBridgeError) {
        lastError = error
        pauseReason = pauseOnErrorPolicy.pauseReason(for: error.code) ?? .bridgeError
        health.recordBridgeError(error.code)
        clockMode = .paused
        phase = .failed
        refreshDebugMetrics()
    }

    private func refreshDebugMetrics() {
        debugMetrics = RuntimeDebugMetrics(
            phase: phase,
            cycle: snapshot.cycle,
            submittedCommandCount: submittedCommandCount,
            unattendedCyclesCompleted: unattendedCyclesCompleted,
            health: health
        )
    }
}
