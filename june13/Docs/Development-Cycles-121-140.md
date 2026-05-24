# SimulatedUniverse Development Cycles 121-140

These cycles define user influence and Experiment mode. The implementation
artifact is `SimulatedUniverse/Runtime/UniverseExperimentMode.swift`.

## Cycle 121: Intervention Taxonomy

Decision: user influence must distinguish camera-only actions, observational
annotations, scenario setup, and simulation-changing interventions.

Implementation: `InterventionTaxonomy`.

Only scenario setup and simulation-changing intervention mutate simulation
state.

## Cycle 122: Intervention Log Schema

Decision: every intervention needs cycle, actor, command, parameters, pre-state,
and result.

Implementation:

- `ExperimentActor`
- `InterventionResult`
- `InterventionLogRecord`

## Cycle 123: Scenario Setup

Decision: scenario setup should cover fixed seed, start date, population count,
and initial selected being.

Implementation: `ExperimentScenarioSetup`.

The setup resolves to a `ScenarioLoadRequest` for the runtime layer.

## Cycle 124: Weather Controls

Decision: weather controls must clearly mark that the simulation was
user-influenced.

Implementation: `WeatherInterventionControl`.

Weather controls expose the runtime intervention command and require
confirmation by default.

## Cycle 125: Resource Controls

Decision: resource intervention controls are available only when a safe bridge
command path exists.

Implementation: `ResourceInterventionControl`.

Without a safe command path, the control returns no runtime command.

## Cycle 126: Controlled Event Injection

Decision: controlled event injection is future planning only until the bridge
supports safe validation.

Implementation: `ControlledEventInjectionPlan.futureOnly`.

Direct engine memory mutation is explicitly disallowed.

## Cycle 127: Annotations

Decision: annotations let users mark moments without changing simulation state.

Implementation: `ObservationAnnotation`.

Annotations store cycle, optional coordinate, optional selected being, and text.

## Cycle 128: Bookmarks

Decision: bookmarks should capture cycle, selected being, camera, overlays, and
panel context.

Implementation: `ExperimentBookmark`.

Bookmarks are UI context, not engine mutation.

## Cycle 129: Experiment Summary

Decision: experiments need a summary containing seed, interventions, scripts,
bookmarks, and outcome notes.

Implementation: `ExperimentSummaryModel`.

The model reports whether the run was influenced by state-changing actions.

## Cycle 130: Experiment Templates

Decision: templates should cover teaching, debugging, behavior comparison, and
long-run observation.

Implementation:

- `ExperimentTemplateKind`
- `ExperimentTemplate.defaults`

Templates control which intervention taxonomies are allowed.

## Cycle 131: Confirmations

Decision: state-changing interventions should require confirmation unless
expert mode is enabled.

Implementation: `InterventionConfirmationPolicy.standard`.

## Cycle 132: Undo Expectations

Decision: undo rules must explain which interventions can be undone locally and
which require replay.

Implementation:

- `InterventionUndoExpectation`
- `InterventionUndoRule.defaults`

## Cycle 133: Command Validation

Decision: invalid interventions should fail before reaching the engine.

Implementation:

- `InterventionValidationResult`
- `InterventionCommandValidator`

Validation checks experiment taxonomy and required bridge capability.

## Cycle 134: Intervention Tests

Decision: logging and serialization need deterministic test cases.

Implementation: `InterventionTestCase.defaults`.

The first cases cover weather, annotations, and resource rejection without
capability.

## Cycle 135: Deterministic Comparison

Decision: no-intervention and intervention runs need a comparison model when
appropriate.

Implementation: `DeterministicInterventionComparison`.

## Cycle 136: Saving And Export

Decision: experiments need rules for naming, saving, duplication, and export.

Implementation: `ExperimentPersistencePlan.initial`.

## Cycle 137: Privacy And Provenance

Decision: exports must declare provenance and user annotation behavior.

Implementation: `ExportProvenanceNote.publicSharing`.

## Cycle 138: Experiment Mode Review

Decision: Experiment mode should support science, teaching, and debugging
without hidden gameplay control.

Implementation: `ExperimentModeReview.initial`.

## Cycle 139: Read-Only Teaching Preset

Decision: teaching presets should demonstrate behavior without state mutation.

Implementation: `ExperimentPreset.readOnlyTeaching`.

## Cycle 140: Developer Preset

Decision: developer presets can expose raw IDs, compile flags, bridge
diagnostics, and performance metrics.

Implementation: `ExperimentPreset.developer`.
