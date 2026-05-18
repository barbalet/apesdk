# Chapter 4 - Weather, Topography, And Resource Geography

The environmental core of ApeSDK lives primarily in [`sim/land.c`](../sim/land.c),
[`sim/tile.c`](../sim/tile.c), [`sim/sim.h`](../sim/sim.h), and shared constants
in [`universe/universe.h`](../universe/universe.h).

This layer answers the question: what kind of world do the beings inhabit before
any being makes a decision?

## Terrain As A Signal Field

Terrain is not background art. It is a signal field used by movement, food,
weather, rendering, and interpretation. The map has multiple resolutions:

- base map dimensions from `MAP_BITS`;
- high-resolution derived terrain from `HI_RES_MAP_BITS`;
- tile-level state for weather and topography;
- control and terrain buffers for drawing.

`land_init()`, `land_init_high_def()`, and tile creation functions establish the
initial world. Interpolation helpers such as `math_bilinear_8_times` expand
lower-resolution data into richer fields. That is a scientific tradeoff: smooth
terrain is easier to render and sample, but it can soften sharp resource
boundaries that would otherwise drive behavior.

## Operators And Resource Meaning

`universe/universe.h` contains a compact operator table:

```c
static const n_byte operators[17][7] = { ... };
```

The table uses terms for area, height, water, moving sun, total sun, and salt.
The later rows combine those terms into resource-style meanings such as bush,
grass, tree, seaweed, rockpool, beach, insect, mouse, parrot, lizard, and eagle.

This is one of the most readable places in the model. A resource is not magic.
It is a compact combination of environmental features. Change the combination,
and you change the ecology.

## Water, Tide, And Shore Pressure

Water is defined around `WATER_MAP`, `TIDE_AMPLITUDE_LUNAR`,
`TIDE_AMPLITUDE_SOLAR`, and `TIDE_MAX`. Tide and water checks influence where
beings can be placed, how movement feels, and where resources concentrate.

Shorelines are especially important because they can create crowding without a
single social rule changing. If beings are compressed by water or tide, conflict
and social contact may increase as a second-order effect.

When testing a tide-related change, record:

- failed or redirected movement;
- shoreline occupancy;
- food events near water;
- conflict near narrow passages;
- social graph density near coasts.

## Weather And Light

Weather values include sunny day, cloudy day, rainy day, clear night, cloudy
night, rainy night, and dawn/dusk. The code also models wind, lightning, and
light/time relationships.

Daylight is not just visual. It can be a behavioral gate. If activity appears
seasonal or cyclical, distinguish three possible causes:

1. the behavior code explicitly responds to time;
2. terrain or resource values change with light;
3. rendering makes a pattern visible that numeric logs do not support.

That distinction prevents "it looked different" from becoming an unsupported
claim.

## Tiles And Large Worlds

`sim/tile.c` separates tile behavior from the simpler single-map view. This
keeps the code flexible for larger terrain experiments and planet-style builds.
Functions such as `tile_cycle`, `tile_weather_init`, `tile_land_init`, and
`tiles_topography` show the project moving beyond a single visual map.

The current macOS CI builds include the `planet` Xcode project, so this layer is
not just legacy scenery. It is part of the active platform boundary.

## Environmental Change Protocol

For any terrain, weather, or resource change:

```text
choose fixed seeds
run a baseline
change one environmental assumption
log position, food, water, weather, and social contacts
compare behavior before interpreting visuals
```

Good environmental tests do not ask "does the map look better?" first. They ask
"which downstream behaviors changed, and can we explain the path from field to
being?"

## Questions For The Reader

- Which resources are direct environmental classifications?
- Which behaviors react to water directly?
- Which behaviors react to water indirectly through crowding?
- How much does interpolation alter movement or food discovery?
- Which weather values affect behavior, and which only affect display?
- Does a proposed visual improvement preserve scientific meaning?
