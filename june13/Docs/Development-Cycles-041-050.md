# SimulatedUniverse Development Cycles 041-050

These cycles establish the first runtime-control layer for SimulatedUniverse.
The implementation artifact is `SimulatedUniverse/Runtime/UniverseRuntime.swift`,
with the existing app shell lightly wired to the deterministic fixture runtime.

## Cycle 041: Runtime Controller

Decision: the app needs one controller that owns runtime phase, scenario
initialization, stepping, pause state, snapshot publication, health, and
shutdown. SwiftUI views should observe copied runtime state rather than talk to
the bridge directly.

Implementation:

- `UniverseRuntimeController` owns the active `UniverseBridgeClient`.
- `RuntimePhase` describes initialization, running, pausing, stepping,
  suspension, shutdown, stopped, and failed states.
- `UniverseRuntimeController.snapshot` publishes the latest copied
  `UniverseSnapshot`.
- `UniverseRuntimeController.lastError` captures bridge failure context without
  exposing engine internals to the UI.

This keeps the UI on a single public path for runtime intent: initialize, load
scenario, pause, resume, step, handle lifecycle event, or shut down.

## Cycle 042: Deterministic Seed Path

Decision: every run must resolve its seed before engine initialization so that
the bridge, runtime metadata, fixture tests, and future replay system agree on
the same source of truth.

Implementation:

- `RuntimeSeedSource` records whether the seed came from the default profile,
  a fixed seed, transfer metadata, or replay metadata.
- `RuntimeSeedConfiguration` records the resolved seed and reproducibility note.
- `ScenarioLoadRequest.resolvedSeed` resolves the seed before the bridge is
  called.
- `FixtureUniverseBridgeClient` uses the resolved seed to produce repeatable
  weather, tide, population, and cycle snapshots.

The fixture is intentionally deterministic. It allows UI, renderer, and
controller work to progress before the C facade is fully connected to ApeSDK.

## Cycle 043: Cycle Clock

Decision: runtime clock modes should be explicit data rather than hidden button
state.

Implementation:

- `RuntimeClockMode.paused`
- `RuntimeClockMode.realtime(targetCyclesPerSecond:)`
- `RuntimeClockMode.singleStep(cycles:)`
- `RuntimeClockMode.accelerated(multiplier:)`
- `RuntimeClockMode.fixedBatch(cycles:)`

Each clock mode maps back to a `SimulationCommand`, and each maps to a durable
`RunMode` value for metadata. The controller currently supports pause, resume,
and single-step execution against the deterministic fixture. Accelerated and
fixed-batch modes are represented now so the scheduler can adopt them without a
public state rename later.

## Cycle 044: Snapshot Throttle

Decision: not every panel should refresh on every rendered frame. The world
viewport needs high-cadence universe snapshots, while memory, social, and event
panels can update on slower intervals.

Implementation: `SnapshotThrottlePolicy`.

Initial policy:

- Universe snapshots can publish every cycle.
- Selected-being snapshots publish every 4 cycles.
- Terrain samples publish every 8 cycles.
- Memory, social, and event panels wait at least 0.15 seconds between updates.

This model is a planning contract for the future scheduler and renderer cache;
it also gives tests concrete values to assert.

## Cycle 045: Background Execution Model

Decision: the simulation should step away from SwiftUI view code, and copied
snapshots should return to the main actor for publication.

Implementation: `RuntimeExecutionModel.catalystDefault`.

The model defines:

- A simulation queue label.
- A snapshot queue label.
- Main-actor publication.
- Maximum cycles per batch.
- Cancellation grace time.

The current controller remains synchronous because the bridge facade does not
exist yet, but the ownership rule is now explicit: runtime code is the only
writer, and UI state receives copied snapshots.

## Cycle 046: Cancellation And Shutdown

Decision: closing a window or replacing a scenario must stop command submission
before destroying the bridge context.

Implementation:

- `UniverseRuntimeController.shutdown()` moves through `shuttingDown` before
  `stopped`.
- `RuntimeExecutionModel.cancellationGraceMilliseconds` sets the initial target
  for future task cancellation.
- `ScenarioLoaderPlan.pausesBeforeReplacement` requires a pause before scenario
  replacement.
- `UniverseBridgeClient.shutdown()` is the single shutdown hook the future C
  bridge implementation must satisfy.

The fixture bridge clears its cycle state during shutdown. The real bridge will
map the same call to `sua_context_destroy`.

## Cycle 047: App And Scene Lifecycle

Decision: Mac Catalyst scene events need predictable runtime transitions.

Implementation:

- `RuntimeLifecycleEvent` enumerates app launch, scene active, scene inactive,
  background, window close, window reopen, and user shutdown.
- `RuntimeLifecycleRule.catalystRules` documents the desired transition for
  each important Catalyst lifecycle event.
- `UniverseRuntimeController.handleLifecycleEvent(_:)` implements the first
  controller behavior for launch, activation, suspension, close, reopen, and
  shutdown.

The important behavioral point is that suspension pauses stepping while keeping
the copied snapshot and metadata available for restoration.

## Cycle 048: Run Metadata

Decision: each run needs durable metadata that can support reproducibility,
replay, diagnostics, and future export.

Implementation: `RunMetadata`.

Metadata now records:

- Run identifier.
- Scenario name.
- Resolved seed.
- Build/version label.
- Start date.
- Current cycle.
- Runtime mode.
- Intervention count.

The controller updates metadata as snapshots advance. Future intervention
commands will call `recordIntervention()` when weather, resources, scripts, or
annotations change the simulation.

## Cycle 049: Simulation Health Monitor

Decision: health should be visible as data before there is a diagnostics panel.

Implementation: `SimulationHealthSnapshot`.

Initial health fields:

- Frame latency.
- Cycle latency.
- Dropped snapshot count.
- Bridge error count.
- Last bridge error code.

The current controller records cycle latency and bridge failures. Renderer work
will later record frame latency and dropped snapshots when draw-ready cache
updates lag behind runtime publication.

## Cycle 050: Minimal Scenario Loader

Decision: the first scenario loader should do only three things well:

- Start the default ApeSDK observation world.
- Start a fixed-seed variant of the same world.
- Prepare for loading a transfer file once the facade can validate one.

Implementation:

- `ScenarioLoadSource`
- `ScenarioLoadRequest`
- `ScenarioLoaderOption`
- `ScenarioLoaderPlan.minimal`
- `UniverseRuntimeController.loadScenario(_:)`

The minimal loader pauses before replacement, validates contract versions as a
future bridge responsibility, and preserves failed-run snapshots so a broken
load does not destroy diagnostic context.
