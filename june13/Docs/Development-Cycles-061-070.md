# SimulatedUniverse Development Cycles 061-070

These cycles start the world-rendering block. The implementation artifact is
`SimulatedUniverse/Rendering/UniverseRendering.swift`, with the existing SwiftUI
canvas now drawing from render-scene models rather than hard-coded placeholder
geometry.

## Cycle 061: Coordinate Systems

Decision: rendering must name every coordinate space it crosses so future
ApeSDK bridge data does not get mixed with SwiftUI canvas points.

Implementation:

- `UniverseCoordinateSystem`
- `CoordinateSystemDescriptor`
- `MapPoint`
- `ViewportProjection`

The defined spaces are ApeSDK map space, viewport space, screen space, and
panel glyph space. ApeSDK map coordinates remain simulation-owned integers.
Viewport coordinates are camera-relative. Screen coordinates are SwiftUI points.
Panel glyph coordinates are normalized values for compact legends and
inspector graphics.

## Cycle 062: Camera Model

Decision: the renderer needs a camera model that can support pan, zoom, follow,
fit population, and return-to-selection behaviors before input gestures are
added.

Implementation: `RenderCameraState`.

Camera operations:

- `pannedBy(x:y:)`
- `zoomed(by:around:)`
- `following(_:at:)`
- `returningToSelection(_:)`
- `fitPopulation(points:map:viewport:)`

This keeps camera math in the rendering layer rather than scattering projection
logic through SwiftUI views.

## Cycle 063: Terrain Tile Cache

Decision: terrain should render from tile summaries so height, water, shoreline,
and ecology can be cached independently of panel refresh cadence.

Implementation:

- `TerrainTileKey`
- `TerrainLayerKind`
- `TerrainTileSample`
- `TerrainTile`
- `TerrainTileCache`

The current tile cache uses deterministic fixture samples derived from snapshot
cycle, land time, tide, and viewport size. The real bridge can later replace the
fixture samples without changing the canvas drawing path.

## Cycle 064: Terrain Color System

Decision: color choices should communicate state directly instead of being only
decorative.

Implementation:

- `RenderRGB`
- `TerrainColorBand`
- `TerrainColorSystem.accessible`

The palette separates deep water, shallow water, shore, lowland, upland, ridge,
analysis-low, analysis-mid, analysis-high, selected ape markers, and default ape
markers. The analysis colors are intentionally distinct from the normal terrain
colors so heatmaps can be read without guessing.

## Cycle 065: Water Rendering

Decision: water rendering must expose tide, terrain height, and shore
transition values to inspectors.

Implementation:

- `WaterRenderState`
- `WaterRenderSample`

Each water sample records tide level, terrain height, dry/shore/shallow/deep
state, shore blend, and inspectable numeric values. The canvas uses these
samples to draw water cells and shoreline strokes.

## Cycle 066: Time-Of-Day Lighting

Decision: lighting should communicate dawn, day, dusk, and night without hiding
overlays or analysis state.

Implementation:

- `DayPhase`
- `TimeOfDayLighting`

Lighting adds tint and shadow strength as separate render data. The current
canvas uses a translucent overlay rather than changing underlying terrain
colors, so future social, memory, weather, and resource overlays remain
legible.

## Cycle 067: Weather Rendering

Decision: weather should become explicit render state before visual effects are
made more elaborate.

Implementation: `WeatherRenderState`.

Weather state now exposes:

- Cloud coverage.
- Rain intensity.
- Wind vector.
- Lightning visibility.
- A transition key for smoothing future effect changes.

The canvas draws simple cloud and wind indications from these values.

## Cycle 068: Resource Visualization In Normal View

Decision: normal Explore view should show resources subtly and avoid turning the
world into an analysis chart.

Implementation:

- `ResourceRenderMode.normal`
- `ResourceVisualizationHint`
- `ResourceVisualizationPlan`

Normal resource hints use small, low-opacity vegetation/resource marks only for
higher-density samples.

## Cycle 069: Resource Visualization In Analysis View

Decision: analysis view should have a clear numeric legend and stronger
heatmap-style signals.

Implementation:

- `ResourceRenderMode.analysis`
- `ResourceLegendEntry`
- `TerrainColorSystem.analysisColor(density:)`

The rendering model now distinguishes normal and analysis resource plans. The
analysis plan includes low, medium, and high legend entries and optional numeric
labels for resource density.

## Cycle 070: Ape Markers

Decision: beings should render as scale-aware markers with position, facing,
movement state, selection highlight, pin state, and labels that appear only when
they are readable.

Implementation:

- `ApeMovementState`
- `ApeMarkerRenderModel`
- `ApeMarkerRenderModel.fixtureMarkers(...)`

Fixture markers are deterministic and derived from snapshot cycle, population,
camera, and selection state. This gives the world viewport a stable marker
contract while the bridge work continues toward real `BeingSnapshot` data.
