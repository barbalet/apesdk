# SimulatedUniverse 200 Cycle Technical Plan

This plan expands the local `README.md` proposal into technical planning cycles
for building a full SimulatedUniverse interface around ApeSDK. Each cycle should
produce a reviewable decision, prototype, test, or implementation slice. The
cycles are ordered so the project stays anchored in the existing simulation
engine before adding richer interaction, visualization, experiment, and replay
systems.

## Cycles 001-020: Product Shape And Architecture

- Cycle 001: Define the non-negotiable product principle that ApeSDK remains the simulation authority and the interface does not fork behavioral rules.
- Cycle 002: Write the first technical glossary for world state, being state, event state, user intervention, overlay, snapshot, and replay.
- Cycle 003: Define app modes for Explore, Inspect, Experiment, and Replay, including which controls are visible in each mode.
- Cycle 004: Decide the initial Mac Catalyst deployment targets, Swift version, Xcode baseline, and signing expectations.
- Cycle 005: Establish the source layout inside `june13` for app code, engine bridge code, UI models, rendering, tests, and documentation.
- Cycle 006: Draft the app shell navigation model with world viewport, left mode rail, right inspector, bottom timeline, and transient command surfaces.
- Cycle 007: Define which state belongs to the UI only, including camera, zoom, selected overlay, pinned beings, panel expansion, and local preferences.
- Cycle 008: Define which state changes must go through explicit simulation commands, including weather edits, resource injection, script execution, and scenario loading.
- Cycle 009: Create a compatibility map between current ApeSDK modules and planned app subsystems.
- Cycle 010: Define minimal source boundaries between SwiftUI views, rendering models, simulation bridge, and persistence.
- Cycle 011: Choose a first renderer strategy for the living world: high-quality 2D map first, with 3D deferred until the model API is stable.
- Cycle 012: Define a frame budget for simulation stepping, state snapshot generation, world rendering, panel updates, and logging.
- Cycle 013: Identify thread ownership rules for the simulation loop, UI state, render cache, and file operations.
- Cycle 014: Define pause, run, step, fast-forward, and fixed-cycle controls so they can be tested without UI automation.
- Cycle 015: Define failure surfaces for engine errors, script errors, transfer errors, corrupted replay files, and missing assets.
- Cycle 016: Draft a deterministic scenario profile format with seed, start time, population count, terrain settings, and enabled interventions.
- Cycle 017: Define naming rules for internal app types so Swift names do not collide with ApeSDK C types.
- Cycle 018: Decide whether the bridge should start as direct C interop, a C wrapper layer, or a small stable C facade.
- Cycle 019: Record the first cut of open technical risks, including bridge lifetime, large state snapshots, rendering cost, and reproducibility.
- Cycle 020: Create a milestone map for world view, selection, overlays, memory/social panels, scripts, logging, replay, and release packaging.

## Cycles 021-040: ApeSDK Bridge And State Contracts

- Cycle 021: Inventory the existing C APIs needed for initialization, simulation cycles, selected being access, transfer, commands, and drawing.
- Cycle 022: Identify direct structure reads that are safe for a first prototype and mark them as temporary bridge dependencies.
- Cycle 023: Define a stable `UniverseSnapshot` model containing cycle, time, weather, tide, selected being id, population count, and camera-neutral map metadata.
- Cycle 024: Define a `BeingSnapshot` model containing identity, location, facing, age, sex, energy, state flags, drives, inventory, and social summary.
- Cycle 025: Define a `TerrainSample` model for clicked map points, including map coordinate, height, water state, resource classification, and nearby beings.
- Cycle 026: Define an `EventSnapshot` model for run log entries, episodic memory entries, user interventions, script execution, and replay markers.
- Cycle 027: Define a `SocialSnapshot` model for relationship type, friend/foe value, attraction, familiarity, family names, last contact, and evidence links.
- Cycle 028: Define a `MemorySnapshot` model for time, event type, affect, location, argument, involved being, and source memory index.
- Cycle 029: Define a `DriveSnapshot` model for hunger, social, fatigue, sex, and any other exposed drive channels.
- Cycle 030: Decide how selected beings are identified across cycles when array positions, death, or reloads change.
- Cycle 031: Plan a C facade for reading snapshots without letting Swift duplicate ApeSDK logic.
- Cycle 032: Plan lifetime management for `sim_init`, `sim_close`, transfer buffers, string buffers, and any callback pointers.
- Cycle 033: Define bridge error codes and Swift error mapping for failed initialization, invalid selection, invalid terrain sample, and script failure.
- Cycle 034: Plan bridge tests that initialize a fixed simulation, step a known number of cycles, and compare stable snapshot fields.
- Cycle 035: Define a version handshake so the app can display ApeSDK version, compile options, and snapshot contract version.
- Cycle 036: Decide how `BRAIN_ON`, ApeScript inclusion, and other compile-time flags should be represented in app capability checks.
- Cycle 037: Plan a bridge fixture mode that can load deterministic mini-worlds for UI and renderer testing.
- Cycle 038: Define a strategy for converting C strings and byte arrays into Swift values without leaking memory.
- Cycle 039: Plan an engine command queue so UI requests are serialized against the simulation loop.
- Cycle 040: Review the bridge contract against every planned panel and overlay before implementing broad UI.

## Cycles 041-060: Simulation Runtime

- Cycle 041: Build a simulation controller that owns engine initialization, cycle stepping, pause state, and snapshot publication.
- Cycle 042: Add a deterministic seed setup path and verify repeated launches produce identical early snapshots.
- Cycle 043: Add a cycle clock that can run real-time, paused, single-step, and accelerated modes.
- Cycle 044: Add a snapshot throttle so inspector panels do not request expensive state every rendered frame.
- Cycle 045: Define a background execution model that keeps simulation stepping off the main thread while UI updates remain main-thread safe.
- Cycle 046: Add cancellation and shutdown rules so closing a window cannot leave the engine in a partial state.
- Cycle 047: Define how the app handles app suspension, window close, reopen, and Catalyst scene restoration.
- Cycle 048: Add a run metadata model containing seed, build version, start date, current cycle, mode, and intervention count.
- Cycle 049: Add a simulation health monitor for frame latency, cycle latency, dropped snapshots, and bridge error count.
- Cycle 050: Plan a minimal scenario loader that can start from default world, fixed seed world, or transfer file.
- Cycle 051: Define selection persistence when time advances, the selected being dies, or the population reloads.
- Cycle 052: Add explicit pause-on-error behavior for script failures, invalid transfer loads, and snapshot contract mismatches.
- Cycle 053: Plan a time scale UI that distinguishes simulation cycles from human wall-clock time.
- Cycle 054: Add state invalidation rules for panels when the selected being changes or snapshots become stale.
- Cycle 055: Define how saved preferences differ from saved simulation runs.
- Cycle 056: Add a first command API for select being, follow being, clear selection, and sample terrain.
- Cycle 057: Add a second command API for pause, resume, step, reset, and load scenario.
- Cycle 058: Add a third command API for state-changing interventions, all of which must emit run log records.
- Cycle 059: Define runtime metrics visible in debug builds but hidden from normal Explore mode.
- Cycle 060: Confirm the runtime can run unattended for long deterministic sessions without UI interaction.

## Cycles 061-080: World Rendering

- Cycle 061: Define the coordinate systems for ApeSDK map space, viewport space, screen space, and panel glyph space.
- Cycle 062: Plan a camera model with pan, zoom, follow, fit population, and return-to-selection behaviors.
- Cycle 063: Build a terrain tile cache that can draw height, water, shoreline, and base ecological texture layers.
- Cycle 064: Define a color system for terrain that avoids hiding state behind decorative color choices.
- Cycle 065: Add water rendering with tide level, shore transition, and inspectable water-state values.
- Cycle 066: Add time-of-day lighting that communicates dawn, day, dusk, and night without obscuring overlays.
- Cycle 067: Add weather rendering for cloud, rain, wind direction, lightning, and weather-value transitions.
- Cycle 068: Define resource visualization in normal view, including subtle vegetation and resource hints.
- Cycle 069: Define resource visualization in analysis view, including heatmap and clear numeric legend.
- Cycle 070: Add ape markers with position, facing, movement state, selection highlight, and scale-aware labels.
- Cycle 071: Add event glyphs for eating, grooming, conflict, swimming, birth, speech, object exchange, and death.
- Cycle 072: Add path trail rendering for selected and pinned beings with cycle aging and clutter control.
- Cycle 073: Add territory or familiarity overlays if the engine exposes enough state to make them meaningful.
- Cycle 074: Add social link overlay rendering for selected being relationships and compare-mode relationships.
- Cycle 075: Add memory location overlay rendering for selected memories and recent events.
- Cycle 076: Add weather overlay instruments for tide, cloud, wind, and time of day.
- Cycle 077: Add a terrain cursor and sample marker for click inspection.
- Cycle 078: Define overlay priority rules so social, memory, resource, and weather overlays can combine predictably.
- Cycle 079: Add renderer performance tests for large populations and high zoom.
- Cycle 080: Verify rendered world state can be compared to source snapshot values during tests.

## Cycles 081-100: Selection, Following, And Comparison

- Cycle 081: Implement hit testing for beings using viewport coordinates and current camera transform.
- Cycle 082: Define selection behavior for overlapping beings, dense groups, and empty map clicks.
- Cycle 083: Add selected being highlight states for normal, followed, pinned, dead, and out-of-view.
- Cycle 084: Build the selected ape profile summary with name, age, sex, energy, state, drive summary, inventory, and location.
- Cycle 085: Add follow mode that keeps the camera near one being while preserving user-controlled zoom.
- Cycle 086: Add follow mode settings for smooth tracking, hard lock, and temporary user pan override.
- Cycle 087: Build a life-thread feed that records notable events for the followed being.
- Cycle 088: Define what counts as notable enough for the life-thread feed to avoid noisy logging.
- Cycle 089: Add pinned beings so a user can keep multiple individuals visible in panels and overlays.
- Cycle 090: Build compare mode for two beings with side-by-side identity, energy, drives, state, and recent events.
- Cycle 091: Extend compare mode with relationship evidence, shared memories, family relation, and genetic summary.
- Cycle 092: Add compare-mode map overlay showing recent paths and relative distance.
- Cycle 093: Define behavior when a compared or followed being dies, disappears, or becomes invalid after reload.
- Cycle 094: Add selection history so users can return to recently inspected beings.
- Cycle 095: Add keyboard shortcuts for select next, select previous, follow, pin, compare, and clear selection.
- Cycle 096: Add accessibility labels for selected beings, overlays, and key inspector values.
- Cycle 097: Add test fixtures for hit testing, selection persistence, and follow camera behavior.
- Cycle 098: Add a selection debug panel showing raw ids, array indexes, and snapshot cycle.
- Cycle 099: Validate selection state does not mutate simulation state unless an explicit command does so.
- Cycle 100: Review selection, follow, and compare workflows with the README goal of active observation rather than puppeteering.

## Cycles 101-120: Inspector Panels

- Cycle 101: Build the compact ape profile panel and wire it to `BeingSnapshot`.
- Cycle 102: Add expandable profile sections for body state, inventory, genetics, family, and current location.
- Cycle 103: Build a drive timeline panel with fixed sample cadence and per-drive scaling.
- Cycle 104: Define drive timeline retention, memory cost, and what happens after long runs.
- Cycle 105: Build the memory panel with event type, time, affect, location, argument, and involved being.
- Cycle 106: Add memory filters for self, social, food, conflict, mating, object, water, and movement events.
- Cycle 107: Add memory-to-map linking so selecting a memory can reveal its location overlay.
- Cycle 108: Add memory-to-social linking so selecting a memory can reveal involved relationship entries.
- Cycle 109: Build the social graph panel with known beings, relationship type, attraction, familiarity, and friend/foe values.
- Cycle 110: Add social evidence drill-down from a relationship to relevant episodic memories.
- Cycle 111: Add graph layout rules for a selected being's local social network.
- Cycle 112: Add social comparison between two beings, including asymmetry in relationship values.
- Cycle 113: Build the environment inspector for terrain click values and nearby beings.
- Cycle 114: Add weather panel values for current weather, tide, time of day, and recent weather changes.
- Cycle 115: Add resource panel values for sampled terrain, local resource density, and nearby activity.
- Cycle 116: Build the run log panel with event, intervention, script, error, and bookmark channels.
- Cycle 117: Add log filters, search, cycle jump, and export selection.
- Cycle 118: Define panel update policies for live, paused, and replay modes.
- Cycle 119: Add empty states for no selection, invalid selection, unavailable compile option, and unsupported snapshot field.
- Cycle 120: Add snapshot-driven panel tests that require no rendered UI.

## Cycles 121-140: User Influence And Experiment Mode

- Cycle 121: Define the intervention taxonomy: camera-only, observational annotation, scenario setup, and simulation-changing intervention.
- Cycle 122: Add an intervention log schema with cycle, actor, command, parameters, pre-state reference, and result.
- Cycle 123: Build scenario setup for fixed seed, start date, population count, and initial selected being.
- Cycle 124: Add weather intervention controls with clear marking that the simulation has been user-influenced.
- Cycle 125: Add resource intervention controls only after the engine exposes a safe command path.
- Cycle 126: Add controlled event injection planning for future work without implementing unsafe direct mutation.
- Cycle 127: Add annotations that let users mark moments without changing simulation state.
- Cycle 128: Add bookmarks that capture cycle, selected being, camera, overlays, and panel context.
- Cycle 129: Build an experiment summary view with seed, interventions, scripts, bookmarks, and outcome notes.
- Cycle 130: Define experiment templates for teaching, debugging, behavior comparison, and long-run observation.
- Cycle 131: Add confirmation UI for state-changing interventions and a setting for expert mode confirmations.
- Cycle 132: Define undo expectations for interventions, including which actions cannot be undone without replay.
- Cycle 133: Add command validation so invalid interventions fail before reaching the engine.
- Cycle 134: Add test coverage for intervention logging and command serialization.
- Cycle 135: Add deterministic comparison between no-intervention and intervention runs where appropriate.
- Cycle 136: Define how experiments are named, saved, duplicated, and exported.
- Cycle 137: Add privacy and provenance notes for exported runs and user annotations.
- Cycle 138: Review experiment mode to ensure it supports science, teaching, and debugging rather than hidden gameplay control.
- Cycle 139: Add a read-only teaching preset that demonstrates behavior without allowing state mutation.
- Cycle 140: Add a developer preset that exposes raw ids, compile flags, bridge diagnostics, and performance metrics.

## Cycles 141-160: ApeScript And Extensibility

- Cycle 141: Inventory current ApeScript examples and classify them by safe demo, engine manipulation, brain experiment, and debug utility.
- Cycle 142: Define a script loading model for local files, bundled examples, recent scripts, and experiment-attached scripts.
- Cycle 143: Build a script panel with source text, parse status, errors, warnings, and execution controls.
- Cycle 144: Expose parser diagnostics with line, column, message, and help text.
- Cycle 145: Add a script dry-run or validation mode if the engine can support it safely.
- Cycle 146: Add script execution logging with script name, hash, cycle, selected being, touched variables, and result.
- Cycle 147: Define how script actions interact with deterministic replay and experiment metadata.
- Cycle 148: Add script capability checks for builds without ApeScript support.
- Cycle 149: Add bundled script examples that demonstrate brain activity, weather experiments, and social observation.
- Cycle 150: Add script output routing into the run log and script panel without blocking the simulation loop.
- Cycle 151: Add script error handling that pauses only when configured or when the error is unsafe to ignore.
- Cycle 152: Define a future plugin boundary for external analysis tools without committing to a plugin API too early.
- Cycle 153: Add export hooks for snapshot streams in JSON or another structured format.
- Cycle 154: Add import planning for externally prepared scenarios while protecting determinism and validation.
- Cycle 155: Define safe limits for script execution time, output volume, and intervention frequency.
- Cycle 156: Add tests for script validation, script logging, and failure presentation.
- Cycle 157: Add documentation that explains scripts as experiments rather than hidden controls.
- Cycle 158: Review whether brain visualization should depend on script support or expose a separate engine capability.
- Cycle 159: Add a structured list of future bridge APIs needed by the script panel.
- Cycle 160: Review extensibility against the principle that ApeSDK remains the source of simulation truth.

## Cycles 161-180: Replay, Persistence, And Verification

- Cycle 161: Define saved run metadata containing engine version, app version, seed, compile flags, scenario, and intervention log.
- Cycle 162: Decide whether replay stores snapshots, event logs, transfer checkpoints, or a hybrid format.
- Cycle 163: Add a checkpoint format plan using existing transfer support where possible.
- Cycle 164: Add replay timeline controls for scrub, step, play, speed, jump to bookmark, and jump to event.
- Cycle 165: Define replay fidelity levels: exact deterministic rerun, event-level replay, and visual-only review.
- Cycle 166: Add replay invalidation rules when engine version or compile flags differ from the saved run.
- Cycle 167: Add export of run log, selected being history, social graph slices, and memory evidence.
- Cycle 168: Add import validation for saved runs and clear user-facing errors for incompatible files.
- Cycle 169: Add local persistence for window layout, last project, recent scripts, and non-simulation preferences.
- Cycle 170: Add snapshot archive pruning so long sessions do not consume unbounded storage.
- Cycle 171: Add checksums for saved run metadata and script inputs.
- Cycle 172: Add deterministic replay tests with fixed seed, no interventions, and expected snapshot digest.
- Cycle 173: Add intervention replay tests with expected log order and expected changed fields.
- Cycle 174: Add UI replay tests for selecting an event and restoring camera/panel context.
- Cycle 175: Add failure tests for corrupted run files, missing script files, and incompatible engine versions.
- Cycle 176: Define export bundle structure for sharing demonstrations or research runs.
- Cycle 177: Add a replay notes model so users can annotate cycles and events during review.
- Cycle 178: Add documentation for what replay can prove and what it cannot prove.
- Cycle 179: Review persistence for file size, determinism, human readability, and future migration.
- Cycle 180: Freeze version 1 of the run metadata schema before broad external testing.

## Cycles 181-200: Quality, Performance, And Release Readiness

- Cycle 181: Build a deterministic test target for the bridge, runtime, commands, snapshots, and run metadata.
- Cycle 182: Build UI model tests for selection, follow, compare, panel filtering, and mode switching.
- Cycle 183: Build renderer tests for coordinate transforms, overlay ordering, terrain sampling, and entity hit testing.
- Cycle 184: Add performance tests for 60-second runs, large populations, high zoom, many overlays, and long logs.
- Cycle 185: Add memory tests for snapshot retention, replay buffers, image caches, and script output.
- Cycle 186: Add accessibility tests for labels, keyboard navigation, contrast, reduced motion, and focus order.
- Cycle 187: Add Catalyst-specific checks for window resizing, menu commands, toolbar behavior, and file dialogs.
- Cycle 188: Add crash handling and diagnostic capture for bridge errors, renderer failures, and save/load problems.
- Cycle 189: Define logging levels for normal user logs, experiment logs, debug diagnostics, and developer traces.
- Cycle 190: Add app documentation for Explore, Inspect, Experiment, Replay, scripts, saved runs, and reproducibility.
- Cycle 191: Add developer documentation for bridge contracts, snapshot models, renderer layers, and command queues.
- Cycle 192: Add onboarding that starts in the living world and teaches by interaction rather than a landing page.
- Cycle 193: Add sample scenarios for observation, social relationships, memory inspection, weather, and script experiments.
- Cycle 194: Add build settings for Debug, Release, and an internal profiling configuration.
- Cycle 195: Add release checklist for signing, archive, notarization expectations, artifact names, and version metadata.
- Cycle 196: Add regression checks against ApeSDK deterministic C tests so the interface does not hide engine breakage.
- Cycle 197: Add issue templates for engine bridge bugs, visual interpretation bugs, replay bugs, and documentation gaps.
- Cycle 198: Run an end-to-end dry run from new scenario to selected being follow, memory inspection, intervention, save, replay, and export.
- Cycle 199: Review the product against the README goals of living world first, overview plus intimacy, explicit interventions, and scientific credibility.
- Cycle 200: Decide the first public milestone scope and cut everything not needed for a credible initial SimulatedUniverse release.
