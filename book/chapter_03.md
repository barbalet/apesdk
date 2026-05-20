# Chapter 3 - Time, Coordinates, And The Clockwork Of A World

ApeSDK treats time and space as first-class modeling choices. The core anchors
are [`sim/sim.h`](../sim/sim.h), [`sim/spacetime.c`](../sim/spacetime.c),
[`sim/land.c`](../sim/land.c), and coordinate-related helpers in
[`entity/being.c`](../entity/being.c).

The world is not a free-floating scene. It is a grid with conversion rules, a
clock with fixed periods, and topology that affects what beings can see, reach,
remember, and revisit.

## Noble Ape Initial-Condition Context

The [Noble Ape talk](https://nobleape.com/) repeatedly returns to initial
conditions: seeded worlds, runs that become more interesting after simulated
time passes, and populations whose adult, juvenile, memory, and language state
all depend on how a run began. That is a useful reminder for the current
ApeSDK. Time and coordinates are not neutral bookkeeping. They define the
starting line from which every later behavior becomes interpretable.

The transcript also treats the command-line simulation as a way to discuss time
and state more precisely than a visual demo alone. That distinction remains
important. A rendered world may show motion, but the time model and coordinate
model explain what that motion means.

## Archive Evidence

- The [ApeSDK Philosophic manual page](https://www.apesdk.com/doc/man/philosophic.html)
  distinguishes the simulation's "internal and external" quantities. This
  directly supports this chapter's separation of external simulation time and
  coordinates from the internal time and space a being may store or infer.
- The same manual describes time externally as fixed and progressive, which
  matches the current code's explicit date/time counters and regular calendar
  assumptions.
- The [Original Manuals](https://www.apesdk.com/doc/Barbalet_OriginalManuals.pdf)
  devote an early section to land, existence, and time. That placement supports
  this chapter's claim that time and place are foundational, not decorative.
- In the auto-captioned
  [thunderstorms video](https://www.youtube.com/watch?v=dw8FRHvx5qI&t=320s),
  the land is discussed through wind, dissipation, topography, and date
  information. That video evidence supports this chapter's treatment of
  coordinates and time as environmental mechanics rather than display labels.

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

## Time As A Causal Spine

In a living system, time is not merely an index. It is the spine that gives
events order. Eating before hunger and eating after hunger may use the same
functions, but they mean different things. Grooming before a conflict and
grooming after a conflict produce different social interpretations. A birth
event before a relationship update and a birth event after that update can alter
which being is remembered as present.

ApeSDK's time model is compact enough to be inspectable. That compactness is a
virtue. The fixed day, month, and year lengths make it easier to reason about
periodic effects. A 28-day month and 13-month year are not trying to mimic the
real calendar. They are creating a stable calendar for experiments.

When you see a date in the simulation, avoid translating it too quickly into
human calendar expectations. Ask what the date means inside the model:

- how many cycles have passed;
- which daily phase the world is in;
- whether a tide or light transition is nearby;
- whether a gestation, weaning, forgetting, or goal timeout window has elapsed;
- whether an event is old enough to decay or expire.

That internal meaning matters more than real-world date familiarity.

## Boundaries Are Where Bugs Hide

Temporal bugs often hide at boundaries: minute rollover, day rollover, month
rollover, year rollover, dawn, dusk, timeout expiration, save/load moments, and
script start or stop. A behavior may look correct for thousands of middle
minutes and fail exactly when one counter resets.

Boundary tests are therefore more valuable than random long runs for time code.
A long run may pass through a boundary, but if it does not assert the right
thing at that moment, it only proves that the program did not crash. A good
boundary test says:

```text
given time 1439
when one cycle passes
then time is 0 and date increases by one
and no event appears to move backward
```

The same principle applies to memory expiration. If an episodic event should
fade after a certain number of days, test the last valid moment, the first
expired moment, and the save/load path around both.

## Coordinates As A Negotiation Between Precision And Meaning

The coordinate system has several scales because no single scale is right for
every question. A being's immediate movement needs finer precision than a
territory memory. A rendered terrain buffer needs a different view than a social
range check. A tile-based world needs yet another level of grouping.

This is a negotiation between precision and meaning. Fine coordinates are good
for movement and collision. Coarse coordinates are good for familiarity, place
memory, and summary. High-resolution terrain is good for visual and sampling
detail. Map-space is good for stable environmental fields.

The danger is accidentally comparing values across scales. A social range in
ape-space cannot be read as a map-space range. A map cell cannot be treated as a
single precise body location. Every conversion loses or gains detail, and that
detail can change behavior.

When debugging position-dependent behavior, write down the scale of every value
you inspect. Many mysteries disappear when one variable is discovered to be in a
different space from the others.

## Wrapped Topology And Ecological Assumptions

A wrapped world is elegant because it removes edges. Beings do not pile up
against an artificial wall. Long-term movement can continue without boundary
cases dominating the simulation. This is especially useful when studying
patterns that should emerge from internal dynamics rather than map edges.

But wrapped topology is not neutral. It means a being near one side of the map
may be close to a being on the opposite side. It means migration has no terminal
frontier. It means a territory can be locally continuous across a coordinate
boundary. It means there is no natural "outside."

For some experiments, that is exactly right. For others, it is wrong. If you
want to study island isolation, one-way dispersal, enclosure effects, or
frontier expansion, wraparound may hide the phenomenon. The important thing is
to name the assumption.

## Visibility, Facing, And Social Consequence

Line of sight is where geometry becomes social opportunity. A being cannot
respond to another being it does not detect. Detection influences approach,
avoidance, grooming, conflict, mate-seeking, and memory formation.

Facing adds asymmetry. Two beings at the same distance may have different
perceptual access depending on direction. That gives the model a simple but
important kind of embodiment: being in a place is not enough; orientation
matters.

When changing visibility, do not only count detections. Count downstream events:

- Did social graph entries change?
- Did episodic memories change?
- Did crowding change?
- Did movement paths change?
- Did conflict rates change?
- Did mate goals change?

A visibility change can look like a perception improvement while actually
rewiring social history.

## Time And Space In Save/Load

Save/load behavior is a strong test of time and coordinates because it asks
whether the engine can pause the world without changing it. A saved state should
preserve enough temporal and spatial information that the next cycle after load
is equivalent to the next cycle before save.

That is hard in subtle ways. Land date and time must survive. Being locations
must survive at the correct scale. Social entries need spacetime. Episodic
memories need event time and location. Territory familiarity must remain tied to
the intended place. Weather and terrain state must not be regenerated in a way
that contradicts the saved world.

If a loaded run diverges immediately from an uninterrupted run, the first places
to inspect are time, random state, location conversion, and transfer fields.

## A Worked Reading Exercise

To understand this chapter in the source, choose one being and follow a single
cycle:

1. Identify current land time and date.
2. Record the being's ape-space location.
3. Convert the location to map-space.
4. Inspect local terrain and water state.
5. Check whether the being is awake.
6. Check whether nearby beings are within social or visibility range.
7. Advance one cycle.
8. Compare time, location, state, and any new events.

The exercise is deliberately small. It teaches the core habit: every behavior is
located in time and space before it is interpreted as intention.

## Designing Better Tests For This Layer

Good tests for time and coordinates should be narrow:

- rollover tests;
- conversion round-trip tests;
- wraparound tests;
- line-of-sight boundary tests;
- save/load continuity tests;
- timeout and expiry tests.

Avoid tests that assert an entire long-run story unless every source of
variation is controlled. Long runs are valuable for exploration, but the
foundation should be tested with small deterministic fixtures.

## Case Study: A Dawn Behavior That Might Not Be About Dawn

Suppose an observer notices that beings appear to gather or change movement near
dawn. This might be a meaningful light-driven behavior. It might also be an
artifact of update order, rendering, sleep/wake state, or terrain visibility.

The first step is to define dawn in engine terms. Which time range counts as
dawn or dawn/dusk? Which macro or weather value is involved? Does the behavior
begin exactly at the boundary or drift around it? If it begins exactly at a
boundary, suspect threshold logic. If it drifts, suspect gradual environmental
or social coupling.

The second step is to inspect state before and after the boundary:

```text
time T-1
light/weather value
selected being awake state
drive values
location
nearby beings
current goal

time T
same values again
```

If only the visual state changes, the behavior may be rendering. If wake state
changes, body scheduling may be the cause. If drive values change, internal
pressure may be involved. If nearby-being count changes, movement or terrain may
be responsible.

This example shows why time code should not be read alone. Time is the index
that lets every other subsystem be compared.

## Case Study: A Boundary Crossing On A Wrapped Map

Wrapped maps remove hard edges, but they create interpretive traps. Imagine a
being near the maximum ape-space coordinate and another near zero. On a wrapped
map, they may be close. On an unwrapped mental map, they look far apart.

If a social event occurs across that boundary, a new reader may think the
distance check is broken. The model is instead applying wrapped topology. The
right debugging approach is to compute the wrapped delta, not the naive
difference.

This matters for rendered views too. A line drawn across the whole screen may
look like a long-distance relationship when it actually represents a short
wrapped connection. Rendering code may need special handling to display wrapped
relationships intuitively. Otherwise the visual translation can mislead the
observer.

Whenever a position is near a boundary, inspect:

- raw coordinates;
- wrapped coordinates;
- map-space conversion;
- territory conversion;
- rendered representation.

## Time Scale And Biological Scale

The simulation calendar defines the scale at which biological constants make
sense. Maturity, gestation, weaning, forgetting, and goal timeout all depend on
time units. If time scale changes without retuning biological constants, the
model's life history changes.

For example, doubling the rate at which cycles are interpreted as minutes would
not merely speed up display. It would change how often drives update relative to
age, memory, and weather. A being might mature, tire, or forget under a
different rhythm.

This is why time constants should be treated as high-blast-radius changes. They
touch almost everything. A time change needs tests for low-level rollover and
scenario checks for body, memory, and population effects.

## Spatial Scale And Social Meaning

Social ranges are defined in space. If `METRES_TO_APESPACE` or related
conversion logic changes, social behavior can change even if social code is
untouched. A grooming range, shout range, squabble range, or mating range is
only meaningful relative to map size, movement speed, and visibility.

Spatial scale also affects density. A fixed population in a smaller effective
space encounters itself more often. A fixed population in a larger effective
space may become isolated. Density then changes social graph pressure, episodic
memory, mate search, and conflict.

This is a good example of cross-layer causality:

```text
coordinate scale -> encounter rate -> social graph -> memory -> future movement
```

If social behavior changes after a map-scale change, the social code may be
innocent.

## Designing Time/Space Documentation

Future documentation should make time and coordinate assumptions visible in any
experiment. A good scenario note includes:

- map dimension;
- coordinate scale used in logs;
- population size;
- seed;
- time span;
- sample interval;
- whether the world wraps;
- relevant range constants;
- save/load moments.

Without these details, comparing two runs becomes difficult. A population of 100
on one map size is not equivalent to a population of 100 on another. A 10-day
run is not equivalent if the update cadence changed. A screenshot is not enough.

## Chapter Three Checklist

Before moving on, make sure you can:

- explain the simulation calendar;
- find time comparison helpers;
- identify the major coordinate scales;
- explain what wraparound implies;
- describe why visibility affects social behavior;
- design a boundary test for time or coordinates;
- explain why save/load tests are strong tests of this layer.

With that foundation, the environmental chapter becomes easier: terrain and
weather are fields over this clock and map.

## Open Design Questions

Time and space invite future extensions. Some would be straightforward, and
some would alter the model deeply. A non-wrapped world would enable boundaries,
islands, and migration fronts, but it would also introduce edge artifacts that
the current topology avoids. Variable day length or seasonal light could create
richer environmental rhythms, but it would complicate comparison across runs. A
more realistic calendar might feel familiar, but it could obscure the current
regularity that makes experiments easier.

The best question is not "which model is more realistic?" It is "which model is
more useful for the behavior being studied?" A simple regular calendar is better
for many controlled experiments. A more complex calendar is useful only when the
extra complexity becomes evidence rather than decoration.

## Practical Exercise

Pick a location near a coordinate boundary. Identify the same place in
ape-space, map-space, and territory-space. Then ask what happens if a being
moves one step across the boundary.

Record:

- raw coordinate before movement;
- raw coordinate after movement;
- wrapped coordinate;
- map-space coordinate;
- territory-space coordinate;
- rendered expectation.

This exercise makes wrapping and scale conversion tangible. It also prepares the
reader for debugging social or visibility events that occur across boundaries.

## A Note On Naming Time In Logs

Logs should name time with enough detail to be useful. A cycle count alone is
not always enough. A date and minute alone may be less useful without version
and seed. A good log line for behavior should include:

```text
version, seed, date, minute, being name, location, state, event
```

That format lets later readers reconstruct where in the world and where in the
calendar the event occurred. Without those anchors, long-run logs become hard to
interpret.
