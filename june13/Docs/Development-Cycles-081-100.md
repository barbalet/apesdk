# SimulatedUniverse Development Cycles 081-100

These cycles establish selection, following, pinning, comparison, accessibility,
fixtures, and workflow guardrails. The implementation artifact is
`SimulatedUniverse/Runtime/UniverseSelectionWorkflows.swift`.

## Cycle 081: Hit Testing

Decision: hit testing should use viewport-projected marker positions rather
than raw map coordinates.

Implementation:

- `BeingHitTestCandidate`
- `BeingHitTestResult`
- `BeingHitTestPolicy`
- `BeingHitTester`

The hit tester uses current marker radius, screen position, padding, selected
priority, pinned priority, and distance to choose candidates.

## Cycle 082: Dense Selection Behavior

Decision: overlapping beings, dense groups, and empty map clicks need explicit
selection outcomes.

Implementation:

- `DenseSelectionBehavior`
- `SelectionClickOutcome`
- `SelectionClickResolution`

Empty clicks can clear or preserve selection depending on policy. Dense clicks
can prefer the existing selected being, select nearest, cycle overlapping
candidates, or open a choice list.

## Cycle 083: Selected Highlight States

Decision: selected beings need distinct visual states for normal, followed,
pinned, dead, and out-of-view cases.

Implementation:

- `SelectedBeingHighlightState`
- `SelectedBeingHighlightModel`

The model includes ring width, marker color, and label suffix so views do not
infer selection semantics from raw IDs.

## Cycle 084: Selected Ape Profile Summary

Decision: the compact profile summary should be derived from `BeingSnapshot`.

Implementation: `SelectedApeProfileSummary`.

The summary includes name, age, sex, energy, state, drive summary, inventory
summary, and location. A deterministic fixture initializer supports UI work
before real selected-being snapshots are connected.

## Cycle 085: Follow Mode Camera Behavior

Decision: follow mode should keep the camera near one being while preserving
user-controlled zoom.

Implementation:

- `FollowCameraUpdate`
- `FollowModeSettings`

The update model moves the camera toward the target marker without rewriting
zoom. This gives renderer tests a stable follow-camera contract.

## Cycle 086: Follow Mode Settings

Decision: follow should support smooth tracking, hard lock, and temporary user
pan override.

Implementation: `FollowTrackingMode`.

Smooth mode interpolates. Hard lock centers immediately. Temporary pan override
allows the user to inspect nearby space without permanently leaving follow mode.

## Cycle 087: Life-Thread Feed

Decision: the followed being needs a readable life-thread feed for notable
events.

Implementation:

- `LifeThreadEvent`
- `LifeThreadFeed`

The fixture feed records recent notable events for the followed ID and is ready
to be replaced by real event or memory snapshots.

## Cycle 088: Notability Rules

Decision: the life-thread feed should not log every minor state change.

Implementation:

- `LifeThreadNotability`
- `LifeThreadRule.defaults`

Birth and death are always notable. Conflict can require high affect. Grooming
and object exchange are notable when they involve the selected being. Eating is
suppressed unless it clears a higher threshold.

## Cycle 089: Pinned Beings

Decision: users should be able to keep multiple beings visible without making
them all selected.

Implementation: `PinnedBeingCollection`.

The first collection supports a maximum pinned count, idempotent pinning, and
explicit unpinning.

## Cycle 090: Compare Mode

Decision: compare mode needs side-by-side identity, energy, drives, state, and
recent-event summaries.

Implementation: `BeingComparisonSummary`.

The first summary compares profile summaries and records energy delta and
recent-event placeholders for future memory data.

## Cycle 091: Relationship Evidence

Decision: compare mode should extend to relationship evidence, shared memories,
family relation, and genetic summary.

Implementation: `RelationshipEvidenceSummary`.

The unavailable state is explicit so panels can distinguish missing bridge data
from an empty social relationship.

## Cycle 092: Compare Map Overlay

Decision: compared beings should have a map overlay showing recent paths and
relative distance.

Implementation: `CompareMapOverlayModel`.

The model accepts left/right markers plus path trails and returns left trail,
right trail, relative distance, and midpoint.

## Cycle 093: Invalid Compared Or Followed Beings

Decision: death, disappearance, and reload invalidation need explicit policies.

Implementation:

- `InvalidComparedBeingPolicy`
- `InvalidSelectionResolution`

The first policy keeps historical snapshots, clears invalid compare sides,
pauses follow mode, and shows an invalid badge.

## Cycle 094: Selection History

Decision: users should be able to return to recently inspected beings.

Implementation:

- `SelectionHistoryEntry`
- `SelectionHistory`

The history de-duplicates by being ID, keeps newest entries first, and caps the
list to a configured maximum.

## Cycle 095: Keyboard Shortcuts

Decision: common selection operations need stable shortcut identifiers.

Implementation: `SelectionKeyboardShortcut`.

The initial shortcuts cover select next, select previous, follow, pin, compare,
and clear selection.

## Cycle 096: Accessibility Labels

Decision: selected beings, overlays, and inspector values need explicit labels
instead of relying on visual state.

Implementation: `AccessibilityDescriptor`.

The first descriptor builds a selected-being accessibility label from profile
summary identity, sex, energy, and location.

## Cycle 097: Selection Fixtures

Decision: hit testing, selection persistence, and follow camera behavior need
deterministic fixtures.

Implementation: `SelectionWorkflowFixture`.

The first fixture records markers, click point, expected selected ID, and
expected camera center.

## Cycle 098: Selection Debug Panel Data

Decision: developer mode should expose raw IDs, array indexes, and snapshot
cycle without mixing them into normal UI.

Implementation: `SelectionDebugSnapshot`.

The debug snapshot records selected, followed, pinned, transient index, cycle,
and hit-test candidate count.

## Cycle 099: Simulation Mutation Audit

Decision: selection state must not mutate simulation state unless a command is
explicitly simulation-changing.

Implementation: `SelectionMutationAudit`.

Selection, follow, clear-selection, and sample-terrain commands are marked as
observational UI commands.

## Cycle 100: Active Observation Review

Decision: selection, follow, and compare should support active observation
rather than hidden puppeteering.

Implementation: `ActiveObservationReview.selectionFollowCompare`.

The review records that selection changes UI focus, follow moves the camera,
compare presents evidence, and invalid selection policies preserve context.
