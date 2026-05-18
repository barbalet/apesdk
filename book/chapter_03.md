# Chapter 3 - Time, Coordinates, And The Clockwork Of A World

ApeSDK treats time and space as first-class modeling choices. The core anchors
are [`sim/sim.h`](../sim/sim.h), [`sim/spacetime.c`](../sim/spacetime.c),
[`sim/land.c`](../sim/land.c), and coordinate-related helpers in
[`entity/being.c`](../entity/being.c).

The world is not a free-floating scene. It is a grid with conversion rules, a
clock with fixed periods, and topology that affects what beings can see, reach,
remember, and revisit.

## Simulation Time

The current time constants in `sim/sim.h` define a compact calendar:

- `TIME_HOUR_MINUTES` is 60.
- `TIME_DAY_MINUTES` is 24 hours.
- `TIME_MONTH_MINUTES` is 28 days.
- `TIME_YEAR_DAYS` is 364 days, equivalent to 13 months of 28 days.

That is intentionally simpler than a Gregorian calendar. Regular periods make
long experiments easier to interpret. If a behavior repeats every 28 days, that
pattern can be compared directly against the simulation calendar.

The spacetime layer provides helpers such as:

- `spacetime_set`;
- `spacetime_copy`;
- `spacetime_after`;
- `spacetime_before_now`;
- `spacetime_convert_to_map`.

These functions are small, but they protect causal order. A memory event, social
intention, or save/load state needs a reliable answer to "when did this happen?"

## Land Time Before Behavior

`land_cycle()` advances the environment. It is one of the functions that gives
the world a pulse: date, time, tide, weather, and tile state all become
downstream inputs for beings.

Conceptually:

```text
advance land time
update tide and environmental fields
update weather or tile conditions
then let beings respond
```

The order matters. If beings act against stale light or tide values, the model
introduces a one-cycle artifact. In a long run, that artifact can look like a
preference or seasonal behavior.

## Coordinate Scales

ApeSDK uses several coordinate scales:

- ape-space for fine being locations;
- map-space for terrain sampling;
- high-resolution map-space for interpolated terrain;
- territory-space for coarser familiarity and place memory.

`sim/sim.h` defines conversions such as:

```c
#define APESPACE_TO_MAPSPACE(num) ((num)>>APE_TO_MAP_BIT_RATIO)
#define MAPSPACE_TO_APESPACE(num) ((num)<<APE_TO_MAP_BIT_RATIO)
#define MAPSPACE_TO_TERRITORY(num) ((num)>>MAP_TO_TERRITORY_RATIO)
```

These macros are a major part of the model. A behavior at one scale may vanish
or become exaggerated at another. Territory memory should not be interpreted as
the same kind of precision as a being's immediate location.

## Wrapped Worlds

The map uses power-of-two boundaries and positive coordinate macros. That makes
wrapping efficient and avoids fixed edge habitats. A wrapped world prevents
beings from being trapped by artificial borders, but it also creates a modeling
assumption: there is no true edge of the world.

This is useful when testing social clustering, foraging, and movement because
edge effects can otherwise dominate results. It is less useful if the experiment
needs cliffs, coast-endpoints, fences, or one-way migration barriers.

## Line Of Sight And Facing

Spatial logic is not only distance. Direction and visibility matter. Being-level
code uses direction-aware and wrapped checks so that an agent's facing can alter
what it detects. That is where topology becomes perception.

When changing visibility, test more than one metric:

- contact counts;
- failed movement;
- social event rates;
- collision or conflict frequency;
- path length;
- memory formation.

A visibility change that looks local can alter the entire social graph.

## Practical Protocol

For time or coordinate changes, use a boundary perturbation suite:

1. Run a fixed seed for a short baseline.
2. Inspect dawn, dusk, midnight, and date rollover.
3. Inspect map wrapping at coordinate boundaries.
4. Save and reload state near a boundary.
5. Compare event order, position, and selected being state.

Time and coordinate code rarely looks dramatic. That is exactly why it deserves
tests. If the clock or map is wrong, every higher-level behavior can still look
plausible while being causally broken.
