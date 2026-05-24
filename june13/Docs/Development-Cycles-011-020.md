# SimulatedUniverse Development Cycles 011-020

These cycles finish the Product Shape and Architecture section of the 200-cycle
plan. The implementation artifact is
`SimulatedUniverse/Foundation/UniverseArchitecture.swift`, which captures the
renderer decision, frame budget, ownership rules, clock controls, failure
surfaces, deterministic scenario profile, naming rules, bridge strategy,
technical risks, and delivery milestones.

## Cycle 011: Renderer Strategy

Decision: the first renderer is a layered 2D cartographic world.

Reasoning:

- ApeSDK already has a rich terrain, weather, being, memory, and social model.
- The first renderer must expose this state clearly before pursuing 3D scene
  complexity.
- 2D map rendering can support zoom, selection, overlays, terrain sampling,
  event glyphs, paths, and replay evidence with less architectural risk.
- 3D is deferred until the snapshot model, camera semantics, and evidence model
  are stable.

Implementation: `RendererStrategy.cartographic2D` is the selected near-term
direction and `RendererStrategy.deferred3D` is tracked as a later option.

## Cycle 012: Frame Budget

Initial interactive target: 60 frames per second.

Starting allocation:

- Simulation step: 4 ms
- Snapshot generation: 2 ms
- World rendering: 7 ms
- Panel updates: 2 ms
- Logging: 1 ms

This creates a 16 ms initial budget, leaving the project with a simple target
that can be tested and revised once bridge snapshots and renderer caches exist.

Implementation: `FrameBudget.initialInteractive`.

## Cycle 013: Thread Ownership

Ownership rules:

- Runtime controller owns simulation stepping and command serialization.
- SwiftUI owns view state such as camera, selected overlays, selected being,
  pinned beings, and expanded panels.
- Rendering owns draw-ready caches derived from snapshots.
- Persistence owns file operations for scenarios, runs, replays, preferences,
  and exports.

Rule: SwiftUI must not hold live ApeSDK pointers, and rendering must not read
engine memory directly.

Implementation: `RuntimeOwnership`.

## Cycle 014: Time Controls

The time-control vocabulary is:

- Paused
- Run realtime
- Step N cycles
- Fast-forward by a multiplier
- Run a fixed number of cycles

These controls map to `SimulationCommand` so they can be tested without UI
automation and later routed through a command queue.

Implementation: `SimulationClockControl`.

## Cycle 015: Failure Surfaces

Failure surfaces:

- Engine error: pause runtime, preserve the latest snapshot, show diagnostics.
- Script error: show parser/execution diagnostics and log script identity.
- Transfer error: reject load and keep current run intact.
- Corrupted replay: open no replay state and report validation failure.
- Missing asset: use fallback visuals and report diagnostics.

Implementation: `FailureSurface`.

## Cycle 016: Deterministic Scenario Profile

The initial scenario profile format contains:

- Name
- Seed
- Start cycle
- Start time in simulation minutes
- Population count
- Terrain profile key
- Enabled intervention capabilities
- Starts-paused flag

The profile is deliberately plain and codable so it can later become a saved
scenario format.

Implementation: `DeterministicScenarioProfile.defaultObservation`.

## Cycle 017: Naming Rules

Rules:

- App-wide Swift types use `Universe` or `SimulatedUniverse` naming.
- Future C facade functions use an `sua_` prefix to avoid colliding with ApeSDK.
- Read-only engine-derived data types end in `Snapshot`.
- Observable UI adapter types end in `Model`.
- Commands describe engine intent, not button labels.

Implementation: `NamingRule`.

## Cycle 018: Bridge Strategy

Decision: start toward a small stable C facade.

Direct C interop is useful for exploratory prototypes, but it lets UI code
couple too tightly to internal structures. A wrapper layer is a transition path,
but the intended long-term architecture is a stable facade that exposes explicit
commands and copied snapshots while ApeSDK remains authoritative.

Implementation: `BridgeStrategy.selected == .stableFacade`.

## Cycle 019: Initial Technical Risks

Initial risks:

- Bridge lifetime and memory ownership.
- Large state snapshots.
- Rendering cost at high zoom or large population.
- Run reproducibility.

Each risk now has an id, severity, and mitigation so future cycles can turn
them into tests and instrumentation.

Implementation: `TechnicalRisk.initialRisks`.

## Cycle 020: Delivery Milestones

Milestone order:

- World view
- Selection
- Overlays
- Memory and social panels
- Scripts and run log
- Replay
- Release packaging

These milestones keep the project focused on the README goals: living world
first, active observation, overview plus intimacy, explicit interventions, and
scientific credibility.

Implementation: `DeliveryMilestone`.
