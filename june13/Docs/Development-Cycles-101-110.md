# SimulatedUniverse Development Cycles 101-110

These cycles start the inspector-panel block. The implementation artifact is
`SimulatedUniverse/Models/UniverseInspectorPanels.swift`.

## Cycle 101: Compact Ape Profile Panel

Decision: the compact ape profile panel should be directly wired to
`BeingSnapshot`.

Implementation:

- `CompactApeProfilePanel`
- `InspectorMetric`

The panel maps identity, sex, state, age, energy, drives, inventory, location,
and source cycle into stable display rows.

## Cycle 102: Expandable Profile Sections

Decision: detailed profile information should be grouped into body state,
inventory, genetics, family, and current location sections.

Implementation:

- `ProfileSectionKind`
- `ExpandableProfileSection`

Sections are derived from `BeingSnapshot` and mark which groups should start
expanded by default.

## Cycle 103: Drive Timeline Panel

Decision: drive timelines need fixed sample cadence and per-drive scaling.

Implementation:

- `DriveKind`
- `DriveTimelineSample`
- `DriveTimelineScale`
- `DriveTimelinePanelModel`

Drive samples record hunger, social, fatigue, and sex drive values at a known
cycle. Scaling converts raw values to normalized chart values.

## Cycle 104: Drive Timeline Retention

Decision: long runs need explicit retention and memory limits.

Implementation: `DriveTimelineRetentionPolicy.interactive`.

The initial policy samples every 8 cycles, retains up to 1,800 samples, and
sets a 256 KB planning budget for the panel's timeline state.

## Cycle 105: Memory Panel

Decision: memories need rows that expose event type, time, affect, location,
argument, and involved being.

Implementation:

- `MemoryPanelRow`
- `MemoryPanelModel`

Rows are initialized from `MemorySnapshot` and preserve links to location and
involved being when the snapshot provides them.

## Cycle 106: Memory Filters

Decision: memory views need filters for self, social, food, conflict, mating,
object, water, and movement events.

Implementation:

- `MemoryFilterKind`
- `MemoryFilterState`
- `MemoryPanelRow.classify(_:)`

The classifier uses event descriptions and involved-being references until the
bridge exposes richer event taxonomy.

## Cycle 107: Memory-To-Map Linking

Decision: selecting a memory should be able to reveal its map location overlay.

Implementation: `MemoryMapLink`.

Rows with coordinates produce a map link containing memory ID, coordinate,
overlay ID, and reveal zoom.

## Cycle 108: Memory-To-Social Linking

Decision: selecting a social memory should reveal the involved relationship
entry when available.

Implementation: `MemorySocialLink`.

Rows with involved beings produce social links keyed by memory and target being
ID.

## Cycle 109: Social Graph Panel

Decision: social graph rows should expose known beings, relationship type,
attraction, familiarity, and friend/foe values.

Implementation:

- `SocialGraphRow`
- `SocialGraphPanelModel`

The panel can report known-being count and strongest relationship using raw
attraction and familiarity values.

## Cycle 110: Social Evidence Drill-Down

Decision: relationship rows should drill down into relevant episodic memories.

Implementation: `SocialEvidenceDrillDown`.

The first drill-down links a social row to memories involving the target being
and preserves evidence event IDs for future event-row navigation.
