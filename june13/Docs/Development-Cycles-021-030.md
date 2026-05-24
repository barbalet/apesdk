# SimulatedUniverse Development Cycles 021-030

These cycles begin the ApeSDK Bridge and State Contracts section. The
implementation artifact is `SimulatedUniverse/Models/UniverseSnapshots.swift`,
with a small update to `BeingIdentifier` in
`SimulatedUniverse/Foundation/UniverseFoundation.swift`.

## Cycle 021: Existing C API Inventory

Initial bridge inventory:

- Simulation lifecycle: `sim_init`, `sim_cycle`, `sim_update_output`,
  `sim_close`, `sim_group`, `sim_timing`, and `sim_memory_allocated` from
  `universe/universe.h`.
- Transfer and persistence: `tranfer_out`, `tranfer_out_json`, `tranfer_in`,
  `transfer_debug_csv`, plus `shared_openFileName` and `shared_saveFileName`
  from `shared.h`.
- Selection and movement: `sim_set_select`, `sim_change_selected`,
  `command_change_selected`, `sim_view_options`, `sim_rotate`, `sim_move`,
  and `sim_view_regular`.
- Drawing and legacy viewport: `shared_draw`, `shared_cycle`,
  `shared_legacy_draw`, `draw_cycle`, `draw_pointer`, `draw_selected_location`,
  and the legacy overlay toggles in `gui/gui.h`.
- Environment: `land_date`, `land_time`, `land_tide_level`, `land_topography`,
  `land_topography_highdef`, `land_highres_tide`, `land_weather`,
  `weather_pressure`, `weather_wind_vector`, and `weather_seven_values`.
- Being identity and body: `being_name_simple`, `being_gender_name`,
  `being_family_name`, `being_location_x`, `being_location_y`,
  `being_high_res`, `being_facing`, `being_energy`, `being_state`,
  `being_state_description`, `being_drive`, `being_drives`, `being_carried`,
  `being_dob`, and `being_index`.
- Memory and social graph: `being_episodic`, `being_social`,
  `episode_description`, `episodic_first_person_memories_percent`,
  `social_graph_link_name`, `social_respect_mean`, and relationship description
  helpers.
- Commands and text watch surfaces: `command_stats`, `command_episodic`,
  `command_social_graph`, `command_braincode`, `command_genome`,
  `command_appearance`, `command_probes`, and `watch_ape`.

## Cycle 022: Temporary Direct Structure Reads

Safe-for-prototype direct reads are allowed only inside the bridge layer and
should be replaced by facade functions when the snapshot contract settles.

Temporary reads:

- `sim_group()->num`, `sim_group()->max`, and `sim_group()->select`.
- `sim_group()->beings[index]` while creating population and being snapshots.
- `simulated_being.events.episodic` and `simulated_being.events.social` when
  building memory and social snapshots.
- `simulated_being.changes.drives` and `simulated_being.changes.inventory` only
  when public accessors are insufficient.

Not allowed outside the bridge:

- Holding live `simulated_group *` or `simulated_being *` in SwiftUI or renderer
  state.
- Comparing selected beings by pointer alone across reloads or transfer files.
- Letting UI code directly traverse `events`, `changes`, or `braindata`.

## Cycle 023: Universe Snapshot

`UniverseSnapshot` now captures the whole-run state needed by the app shell:

- Contract version.
- Cycle.
- Land date and time.
- Weather and tide summaries.
- Selected being id.
- Population count and max.
- Camera-neutral map metadata.

This keeps world-level UI separate from selected-being details.

## Cycle 024: Being Snapshot

`BeingSnapshot` now captures:

- Stable/transient identity.
- Display name.
- Location and optional high-resolution location.
- Facing.
- Age.
- Sex.
- Energy.
- State flags and description.
- Drives.
- Inventory.
- Social summary.

The bridge should build this from public accessors first and direct reads only
where the cycle-022 temporary rule allows it.

## Cycle 025: Terrain Sample

`TerrainSampleSnapshot` now represents a clicked or inspected map point:

- Coordinate.
- Height.
- Water level and underwater flag.
- Local weather.
- Resource classification.
- Nearby being ids.

This lets the environment inspector read a sampled model rather than query the
engine from SwiftUI.

## Cycle 026: Event Snapshot

`EventSnapshot` now represents five event sources:

- Run log.
- Episodic memory.
- User intervention.
- Script execution.
- Replay marker.

Each event has a cycle-scoped id, kind, summary, optional coordinate, involved
beings, and optional raw argument.

## Cycle 027: Social Snapshot

`SocialSnapshot` now captures:

- Source being.
- Optional target being.
- Target display name.
- Raw and described relationship.
- Friend/foe value.
- Attraction.
- Familiarity.
- Last known location.
- Evidence event ids.

The evidence ids are important: the social graph panel should connect summary
values back to memory or run-log evidence.

## Cycle 028: Memory Snapshot

`MemorySnapshot` now captures:

- Source being.
- Source memory index.
- Raw and described event.
- Raw affect.
- Optional coordinate.
- Raw argument.
- Optional involved being.
- Optional evidence event id.

This preserves exact source indexes while letting the UI display safer strings
and links.

## Cycle 029: Drive Snapshot

`DriveSnapshot` now exposes the core planned channels:

- Hunger.
- Social.
- Fatigue.
- Sex.
- Raw values for additional engine drive channels.

The raw dictionary keeps the model expandable if the engine exposes more drives
or if names change during bridge work.

## Cycle 030: Selection Identity

Decision: selected beings should use a two-layer identity.

- `BeingIdentifier.value` is the transient in-run index or bridge-local id.
- `BeingIdentifier.stableKey` is optional but preferred when available.
- `BeingStableKey` combines gender name, family name, and date of birth.

Why this shape:

- Pointers are invalid across reloads and transfer files.
- Array indexes can shift when beings die or are compacted.
- Names alone can collide.
- Date of birth plus the packed ApeSDK names gives the interface a better
  recovery key while still allowing an index fallback during early prototypes.

The bridge should resolve selection by stable key first, then fall back to the
transient index only within the same live run.
