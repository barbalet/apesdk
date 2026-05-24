# SimulatedUniverse Development Cycles 111-120

These cycles complete the first inspector-panel block. The implementation
artifact is `SimulatedUniverse/Models/UniverseInspectorAdvanced.swift`.

## Cycle 111: Social Graph Layout

Decision: local social networks need layout rules before the panel draws them.

Implementation:

- `SocialGraphLayoutStyle`
- `SocialGraphNode`
- `SocialGraphEdge`
- `SocialGraphLayoutModel`

The first layout is radial around the selected being, with target radius derived
from familiarity.

## Cycle 112: Social Comparison

Decision: relationship comparison must preserve asymmetry between two beings.

Implementation: `AsymmetricRelationshipComparison`.

The model stores left-to-right and right-to-left rows, attraction delta,
familiarity delta, and a human-readable asymmetry summary.

## Cycle 113: Environment Inspector

Decision: terrain click inspection needs a panel model for terrain values and
nearby beings.

Implementation: `EnvironmentInspectorPanelModel`.

The panel maps a `TerrainSampleSnapshot` into height, water, underwater state,
resource classification, density, and nearby being IDs.

## Cycle 114: Weather Panel

Decision: weather panels should show current weather, tide, time of day, and
recent changes.

Implementation:

- `WeatherPanelModel`
- `WeatherPanelChange`

The first change detector compares current and previous weather condition.

## Cycle 115: Resource Panel

Decision: sampled resources need local density and nearby activity values.

Implementation: `ResourcePanelModel`.

The panel includes classification, density, raw value, and nearby activity
count from the sampled terrain.

## Cycle 116: Run Log Panel

Decision: the run log needs separate channels for event, intervention, script,
error, and bookmark entries.

Implementation:

- `RunLogPanelChannel`
- `RunLogPanelEntry`
- `RunLogPanelModel`

The model is independent of rendered UI and can be fed by runtime, experiment,
script, and replay layers.

## Cycle 117: Log Filters And Export Selection

Decision: logs need filtering, search, cycle jump, and export selection.

Implementation: `RunLogPanelFilter`.

Filters combine channel, search text, and optional cycle range. The panel model
can return selected visible entries and jump to the cycle for a selected entry.

## Cycle 118: Panel Update Policies

Decision: live, paused, and replay modes need explicit panel refresh behavior.

Implementation:

- `PanelUpdateMode`
- `PanelUpdatePolicy.liveDefaults`

Profile and drive panels update quickly. Memory, social, environment, run log,
and script panels tolerate slower or stale data.

## Cycle 119: Empty States

Decision: missing panel data should explain why it is missing.

Implementation:

- `InspectorEmptyStateReason`
- `InspectorEmptyState`

The initial states cover no selection, invalid selection, unavailable compile
option, and unsupported snapshot field.

## Cycle 120: Snapshot-Driven Panel Tests

Decision: panel tests should run from snapshots without rendered UI.

Implementation: `SnapshotDrivenPanelTestCase.defaults`.

The first cases cover profile, memory filtering, social graph, environment
sampling, and run-log filtering.
