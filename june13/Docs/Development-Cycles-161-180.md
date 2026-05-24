# SimulatedUniverse Development Cycles 161-180

These cycles define replay, persistence, and verification. The implementation
artifact is `SimulatedUniverse/Persistence/UniverseReplayPersistence.swift`.

## Cycle 161: Saved Run Metadata

Decision: saved runs need engine version, app version, seed, compile flags,
scenario, snapshot contract, and intervention log.

Implementation: `SavedRunMetadataV1`.

## Cycle 162: Replay Storage Strategy

Decision: replay should use a hybrid format.

Implementation:

- `ReplayStorageStrategy`
- `ReplayStorageDecision.hybridDefault`

## Cycle 163: Transfer Checkpoints

Decision: checkpoints should use existing transfer support where possible.

Implementation: `TransferCheckpointPlan.sparse`.

## Cycle 164: Replay Timeline Controls

Decision: replay controls need scrub, step, play, speed, bookmark jump, and
event jump commands.

Implementation:

- `ReplayTimelineCommand`
- `ReplayTimelineControlState`

## Cycle 165: Replay Fidelity Levels

Decision: replay must distinguish exact deterministic rerun, event-level
replay, and visual-only review.

Implementation: `ReplayFidelityLevel`.

## Cycle 166: Replay Invalidation

Decision: engine version, compile flags, and snapshot contract changes can
invalidate exact replay.

Implementation: `ReplayInvalidationRule.defaults`.

## Cycle 167: Export Selection

Decision: exports should include run log, selection history, social graph
slices, and memory evidence.

Implementation: `ReplayExportSelection.researchDefault`.

## Cycle 168: Import Validation

Decision: incompatible saved runs need clear user-facing errors.

Implementation: `RunImportValidationError`.

## Cycle 169: Local Preferences

Decision: local preferences are separate from simulation state.

Implementation: `LocalPreferenceRecord`.

## Cycle 170: Snapshot Archive Pruning

Decision: long sessions need bounded storage.

Implementation: `SnapshotArchivePruningPolicy.longSession`.

## Cycle 171: Checksums

Decision: metadata and script inputs need checksums.

Implementation: `RunChecksum.fnv1a(_:)`.

## Cycle 172: Deterministic Replay Tests

Decision: fixed seed and no-intervention replay needs an expected snapshot
digest.

Implementation: `DeterministicReplayTestCase.noInterventionSmoke`.

## Cycle 173: Intervention Replay Tests

Decision: intervention replay should assert log order and changed fields.

Implementation: `InterventionReplayTestCase.weatherIntervention`.

## Cycle 174: UI Replay Tests

Decision: event selection should restore camera and panel context.

Implementation: `UIReplayRestorationTestCase.eventSelection`.

## Cycle 175: Failure Tests

Decision: corrupted runs, missing script files, and incompatible engine versions
need failure tests.

Implementation: `ReplayFailureTestCase.defaults`.

## Cycle 176: Export Bundle

Decision: shared demonstrations and research runs need a bundle structure.

Implementation: `ExportBundleStructure.demonstrationBundle`.

## Cycle 177: Replay Notes

Decision: users should annotate cycles and events during replay review.

Implementation: `ReplayNote`.

## Cycle 178: Replay Proof Documentation

Decision: documentation must state what replay can and cannot prove.

Implementation: `ReplayProofDocumentation.initial`.

## Cycle 179: Persistence Review

Decision: persistence should be reviewed for size, determinism, readability, and
migration.

Implementation: `PersistenceReview.initial`.

## Cycle 180: Schema Freeze

Decision: run metadata schema version 1 is frozen before broad external
testing.

Implementation: `RunMetadataSchemaFreeze.version1`.
