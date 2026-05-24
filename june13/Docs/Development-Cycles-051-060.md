# SimulatedUniverse Development Cycles 051-060

These cycles complete the first simulation-runtime planning block. The main
implementation artifact is
`SimulatedUniverse/Runtime/UniverseRuntimeInteractions.swift`, with controller
hooks added to `SimulatedUniverse/Runtime/UniverseRuntime.swift`.

## Cycle 051: Selection Persistence

Decision: selection must survive ordinary time advancement but fail explicitly
when the selected being is no longer valid.

Implementation:

- `BeingSelectionState` stores selected, followed, and pinned being IDs.
- `SelectionLossReason` distinguishes user clearing, time advancement, selected
  being death, population reload, and scenario reload.
- `SelectionResolution` records whether selection was kept, cleared, or reduced
  to pinned-only state.
- `UniverseRuntimeController.submit(_:)` now accepts selection commands.

The key rule is that time advancement alone should not clear selection. A
population reload or death may clear selection, but that must be recorded as a
reason rather than silently dropping UI state.

## Cycle 052: Pause On Error

Decision: script failures, invalid transfer loads, and snapshot contract
mismatches must stop simulation time before the user can continue.

Implementation:

- `PauseOnErrorPolicy.deterministic`
- `RuntimePauseReason`
- `UniverseRuntimeController.pauseReason`

The controller already records bridge errors and enters `failed`; it now also
records why the clock was forced into a paused state. This keeps future UI copy
and diagnostics grounded in the actual failure class.

## Cycle 053: Time Scale UI

Decision: the interface must distinguish simulation cycles from human
wall-clock time.

Implementation: `TimeScalePresentation`.

The model carries:

- Current simulation cycle.
- Cycles per simulated ape minute.
- Target cycles per human second.
- Human wall-clock elapsed time.

This lets future timeline controls say exactly whether they are describing
engine cycles, simulated biological time, or elapsed app time.

## Cycle 054: Panel Invalidation

Decision: inspector panels need invalidation rules separate from rendering.
Selection changes should invalidate being-specific panels, while stale snapshots
should invalidate panels after a known cycle threshold.

Implementation:

- `PanelInvalidationCause`
- `PanelInvalidationRule`
- `PanelInvalidationRule.defaults`

Initial thresholds are strict for drives and profile data, looser for memory,
social graph, script, and run log panels. Environment invalidation depends on
terrain sampling rather than selection.

## Cycle 055: Preferences Versus Runs

Decision: saved preferences and saved simulation runs are different artifacts.

Implementation:

- `PersistenceArtifactScope`
- `UserPreferenceRecord`
- `SavedRunManifest`

Preferences store UI choices: mode, overlays, panels, and selection-following
behavior. Run manifests store reproducibility metadata: scenario, seed, build
version, current cycle, interventions, transfer reference, and snapshot contract
version. Transfer files remain the authoritative simulation-state artifact.

## Cycle 056: Selection And Sampling Commands

Decision: selection, follow, clear selection, and terrain sampling should be
typed commands, not view-local button behavior.

Implementation: `RuntimeSelectionCommand`.

Command cases:

- Select being.
- Follow being.
- Clear selection.
- Sample terrain.

The controller maps these to selection state or deterministic fixture terrain
sampling. The terrain path is now bridge-shaped, ready for a future
`sua_snapshot_terrain_json` implementation.

## Cycle 057: Runtime Control Commands

Decision: pause, resume, step, reset, and scenario load should share a single
control-command API.

Implementation: `RuntimeControlCommand`.

The controller now accepts typed control commands and routes them to existing
runtime methods. Reset and load scenario both resolve to scenario replacement
for now; future work can split reset semantics once transfer restoration and
replay startup exist.

## Cycle 058: Intervention Commands

Decision: every state-changing intervention must emit run-log evidence.

Implementation:

- `RuntimeInterventionCommand`
- `RunLogRecord`
- `RunLogRecordKind`
- `UniverseRuntimeController.runLog`

Initial interventions cover weather override, resource injection, script load,
and annotation. The fixture runtime records the intervention and increments run
metadata even before the real bridge mutates ApeSDK state.

## Cycle 059: Debug Metrics

Decision: runtime metrics should exist in debug builds and inspection modes but
stay out of normal Explore mode.

Implementation:

- `RuntimeDebugMetrics`
- `RuntimeMetricDescriptor`
- `RuntimeMetricVisibility`

Metrics now cover phase, cycle, submitted commands, unattended cycles, and
bridge errors. Visibility is mode-aware, so future UI can show these in Inspect
or debug surfaces without crowding the normal exploration interface.

## Cycle 060: Unattended Deterministic Sessions

Decision: the runtime needs a deterministic long-run plan that does not depend
on UI interaction.

Implementation:

- `UnattendedRuntimePlan.deterministicSmoke`
- `DeterministicSessionCheckpoint`
- `UniverseRuntimeController.runUnattended(_:)`

The smoke plan targets 10,000 deterministic cycles with 1,000-cycle checkpoints
and no accepted bridge errors. The controller can now drive the fixture bridge
in bounded batches using the runtime execution model's maximum batch size.
