# SimulatedUniverse Development Cycles 071-080

These cycles complete the first world-rendering block. The implementation
artifact is `SimulatedUniverse/Rendering/UniverseRenderingOverlays.swift`.

## Cycle 071: Event Glyphs

Decision: notable simulation events should have small, typed glyph models rather
than ad hoc drawing instructions.

Implementation:

- `EventGlyphKind`
- `EventGlyphRenderModel`
- deterministic fixture glyphs for eating, grooming, conflict, swimming, birth,
  speech, object exchange, and death

Glyphs carry event identity, map position, screen position, cycle age, display
radius, color, and priority. This gives later run-log and memory panels a
shared visual vocabulary.

## Cycle 072: Path Trails

Decision: selected and pinned beings need trail rendering with aging and clutter
control.

Implementation:

- `PathTrailPoint`
- `PathTrailRenderModel`
- `PathTrailPolicy.interactive`

The fixture trail model draws only selected or pinned paths, fades older points,
and records a maximum visible trail count for future renderer tests.

## Cycle 073: Territory And Familiarity Overlays

Decision: territory and familiarity overlays should exist only when they can be
meaningful and inspectable.

Implementation:

- `TerritoryOverlayKind`
- `TerritoryOverlayCell`
- `TerritoryOverlayModel`

The current fixture marks an overlay meaningful only when enough tile cells are
available. The real engine bridge can later replace fixture values with
territory or familiarity state.

## Cycle 074: Social Link Overlays

Decision: relationship overlays should connect selected/compared beings without
pretending the UI controls the social graph.

Implementation: `SocialLinkRenderModel`.

Links include source and target IDs, screen endpoints, relationship strength,
line width, color, and compare-mode status. Compare mode increases the number of
visible links but still treats them as evidence, not commands.

## Cycle 075: Memory Location Overlays

Decision: selected memories and recent events need map locations that can be
shared by the world view and inspector panels.

Implementation: `MemoryLocationRenderModel`.

Memory overlays include memory ID, optional event ID, map position, screen
position, cycle age, and color. The fixture keeps these deterministic until real
memory snapshots are available.

## Cycle 076: Weather Overlay Instruments

Decision: tide, cloud, wind, and time of day should have compact instrument
models rather than being buried in decorative weather effects.

Implementation:

- `WeatherInstrumentKind`
- `WeatherInstrumentModel`

Each instrument exposes a title, value description, normalized value, and
optional vector. This will support both world overlays and environment panels.

## Cycle 077: Terrain Cursor

Decision: click inspection needs a cursor model that can point at either a raw
map coordinate or a sampled terrain result.

Implementation: `TerrainCursorModel`.

The cursor stores screen position, map position, optional terrain sample,
visibility, and a concise label containing coordinate and height when available.

## Cycle 078: Overlay Priority

Decision: social, memory, resource, weather, event, selection, and cursor
overlays need deterministic ordering.

Implementation:

- `RenderOverlayKind`
- `OverlayPriorityRule.defaults`
- `OverlayCompositionPlan`

Selection and cursor overlays sit above blended overlays. Resources, territory,
memory, social, weather, and event glyphs have explicit priority values so
combined overlays are predictable.

## Cycle 079: Renderer Performance Tests

Decision: renderer performance targets need explicit fixtures before test code
exists.

Implementation: `RendererPerformanceFixture.largePopulationHighZoom`.

The first fixture describes a 512-being, high-zoom, multi-overlay render with a
16.67 ms frame budget and explicit marker/glyph count ceilings.

## Cycle 080: Source Snapshot Comparison

Decision: rendered world state must be comparable to source snapshot values in
tests.

Implementation:

- `RenderSourceProbe`
- `RenderSnapshotComparison`
- `RenderOverlayScene.sourceComparison`

The comparison model records snapshot cycle and probes that connect source
values to rendered values. This is the start of a deterministic render
verification path for the fixture world.
