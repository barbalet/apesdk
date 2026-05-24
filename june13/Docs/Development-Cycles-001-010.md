# SimulatedUniverse Development Cycles 001-010

These cycles convert the opening section of `SimulatedUniverse-200-Cycle-Plan.md`
from proposal into initial project decisions. The implementation artifact is
`SimulatedUniverse/Foundation/UniverseFoundation.swift`, which gives the Catalyst
app stable names for modes, overlays, UI-only state, simulation commands, module
roles, and source boundaries.

## Cycle 001: Simulation Authority

Decision: ApeSDK remains the simulation authority. SimulatedUniverse observes,
renders, explains, and commands ApeSDK state; it does not create a parallel
behavior model.

Technical implications:

- SwiftUI views must not invent being state that is not backed by ApeSDK.
- Interpretation layers may summarize state, but must keep a traceable source.
- Any state-changing UI action must become an explicit simulation command.
- Rendering can be rich, but visual claims must be explainable through snapshots,
  events, terrain samples, weather values, social entries, memories, or scripts.

## Cycle 002: Technical Glossary

- World state: the current ApeSDK simulation state owned by the engine.
- Being state: the selected or listed state for one simulated being, including
  identity, location, body, drives, memory, social graph, and current flags.
- Event state: a discrete occurrence from engine behavior, memory, script output,
  user annotation, or intervention log.
- User intervention: any user action that mutates simulation state, such as
  changing weather, injecting resources, loading scripts, or resetting scenarios.
- Overlay: an optional visual layer over the world view, such as social links,
  memory locations, weather instruments, or resources.
- Snapshot: a structured, read-only Swift model extracted from ApeSDK for UI and
  rendering use.
- Replay: a saved or regenerated run view that can restore cycle context, event
  context, and visible evidence.

## Cycle 003: App Modes

- Explore: default living-world mode with world view, selection, follow, overlays,
  and compact inspector access.
- Inspect: analytical mode with pause/step affordances, expanded panels, terrain
  sampling, memory inspection, social graph drill-down, and exact values.
- Experiment: controlled intervention mode with scenario setup, scripts, explicit
  command confirmation, and intervention logging.
- Replay: timeline mode for saved runs, bookmarks, event jumps, and comparison
  against original run metadata.

The app now has `UniverseMode` as a typed foundation enum.

## Cycle 004: Catalyst Target Baseline

Decision:

- Project: `june13/SimulatedUniverse.xcodeproj`
- Target: `SimulatedUniverse`
- Platform: iOS target with Mac Catalyst support
- Deployment: iOS 17.0 / Mac Catalyst destination
- Swift: Swift 5
- Signing: automatic in project, disabled for local verification with
  `CODE_SIGNING_ALLOWED=NO`
- Bundle id: `com.nobleape.SimulatedUniverse`

Verified command:

```sh
xcodebuild -project june13/SimulatedUniverse.xcodeproj -scheme SimulatedUniverse -configuration Debug -destination 'platform=macOS,variant=Mac Catalyst' CODE_SIGNING_ALLOWED=NO build
```

## Cycle 005: Source Layout

Initial layout inside `june13/SimulatedUniverse`:

- `Foundation`: app-wide modes, state vocabulary, source boundaries, and command
  types that are independent of UI implementation.
- `Bridge`: future C interop, C facade, memory ownership, and snapshot extraction.
- `Models`: Swift snapshot and panel models derived from ApeSDK state.
- `Rendering`: terrain, entity, overlay, glyph, camera, and hit-test rendering.
- `Runtime`: simulation loop ownership, pause/step timing, command queue, and
  snapshot cadence.
- `Persistence`: preferences, scenario files, run metadata, replay metadata, and
  exports.
- `Tests`: future deterministic bridge, model, runtime, and renderer tests.

## Cycle 006: App Shell Navigation

The shell remains world-first:

- Left rail: mode selection and overlay toggles.
- Center: living world viewport.
- Right side: selected being, memory, social, environment, and run log inspector.
- Toolbar: pause, step, run log, and later scenario/script/replay controls.
- Bottom timeline: reserved for later cycles, because replay and event history
  need explicit snapshot/log contracts first.
- Transient command surfaces: menus, sheets, and popovers for interventions,
  scripts, scenario loading, and export.

## Cycle 007: UI-Only State

UI-only state is represented by `UniverseUIState` and includes:

- Current app mode.
- Camera center, zoom, and followed being id.
- Enabled overlays.
- Selected being id.
- Pinned being ids.
- Expanded inspector panels.

This state may be persisted as preference or layout state, but it must not alter
engine behavior.

## Cycle 008: Simulation Commands

State-changing or engine-querying actions are represented by `SimulationCommand`.
Current command categories:

- Runtime control: pause, resume, step.
- Scenario control: reset scenario.
- Selection/query: select being, sample terrain.
- Intervention: set weather, inject resource.
- Script/replay: load script, load replay.

The command type is intentionally separate from SwiftUI controls so the runtime
can test command handling without UI automation.

## Cycle 009: ApeSDK Compatibility Map

- `toolkit`: low-level memory, strings, math, files, objects, and shared types.
- `script`: ApeScript parsing, diagnostics, and experiment execution.
- `sim`: terrain, weather, time, resources, territory, and environment constants.
- `entity`: beings, braincode, body, drives, immune state, episodic memory, and
  social graph.
- `universe`: simulation lifecycle, stepping, selection, transfer, commands, and
  run authority.
- `render`: graph primitives and future reusable drawing support.
- `gui`: legacy draw/shared behavior used as a behavioral and visual reference,
  not as the long-term UI architecture.

This map is represented by `ApeSDKModule`.

## Cycle 010: Source Boundaries

Initial boundaries are represented by `SourceBoundary`:

- SwiftUI views display state and collect user intent.
- Rendering models convert snapshots into draw-ready terrain, beings, overlays,
  and glyphs.
- Simulation bridge owns C interop, memory ownership, command serialization, and
  snapshot extraction.
- Runtime controller owns cycle timing, pause state, snapshot cadence, and
  mode-safe command routing.
- Persistence owns preferences, scenarios, run metadata, replay references, and
  exports.

Rule: dependencies should point inward toward stable models and command types,
not outward toward views.
