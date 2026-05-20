# Chapter 4 - Weather, Topography, And Resource Geography

The environmental core of ApeSDK lives primarily in [`sim/land.c`](../sim/land.c),
[`sim/tile.c`](../sim/tile.c), [`sim/sim.h`](../sim/sim.h), and shared constants
in [`universe/universe.h`](../universe/universe.h).

This layer answers the question: what kind of world do the beings inhabit before
any being makes a decision?

## Noble Ape Landscape Context

The [Noble Ape reference](https://nobleape.com/) describes the older simulation
as beginning with a randomly generated landscape, weather moving across it, and
a biological layer built from environmental probabilities such as surface area,
height, moving sunlight, total sunlight, rainfall, and salt. That description is
not just historical color. It is a direct ancestor of the current environmental
chapter's concerns: terrain is a field of pressures, and resource meaning comes
from the interaction of multiple environmental components.

The transcript's discussion of plant-like and animal-like noise maps also helps
frame the current operator table. ApeSDK does not need every resource to be a
fully simulated organism for it to matter. A compact probabilistic or
operator-driven field can still create habitat, movement pressure, and social
consequences.

## Archive Evidence

- The [IEEE article](https://www.apesdk.com/doc/Barbalet_IEEE.pdf) states that
  the project aimed to "simulate a biologically diverse landscape." That phrase
  is the direct historical root of this chapter's treatment of terrain as an
  active ecological field.
- In [The Mind of the Noble Ape in Three Simulations](https://www.apesdk.com/doc/Barbalet_OriginOfMind.pdf),
  the biological layer is described through environmental properties such as
  height, water, sunlight, and salt. That supports the chapter's explanation of
  the operator table as compact ecology rather than decorative map coloring.
- The [Original Manuals](https://www.apesdk.com/doc/Barbalet_OriginalManuals.pdf)
  provide the broader island and virtual-environment framing behind this
  ecological reading.
- The auto-captioned
  [Ideas of Thunderstorms with Simulated Ape](https://www.youtube.com/watch?v=dw8FRHvx5qI&t=110s)
  points to an existing "weather system" and frames thunder and lightning as an
  atmospheric extension. That supports this chapter's view of land, weather,
  and tiles as active ecological machinery.

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

## The Environment Is An Active Participant

It is easy to describe terrain as the stage on which beings act. That language
is too passive for ApeSDK. The environment is an active participant. It presents
opportunities, blocks routes, concentrates resources, changes with time, and
creates the encounter structure from which social life emerges.

A being does not simply decide to eat, mate, travel, or fight in an abstract
space. It does so in a particular field of water, light, slope, tide, and
resource classification. Environmental structure can make two beings meet often
or rarely. It can make a food source safe or costly. It can turn a narrow
crossing into a conflict generator without adding any explicit conflict rule.

This means environmental code should be read as behavior code at one remove. A
change in terrain may not mention social behavior, but it can still change the
social graph. A change in tide may not mention fatigue, but it can make swimming
more common and thereby change fatigue. A change in resource distribution may
not mention reproduction, but it can alter energy, movement, and mating windows.

## Operators As A Compact Ecology

The operator table in `universe.h` is one of the most compressed ecological
statements in the project. Each row says how a resource-like classification
relates to environmental components. A plus sign means the component supports
the classification. A minus sign means it pushes against it. A dot means the
component is not considered for that row.

That table makes resource geography legible. Instead of hiding habitat
definitions in many branches, it gathers them into a small symbolic grammar.
This has a practical advantage: a reader can compare resources side by side.
Bush, grass, tree, seaweed, rockpool, beach, insect, mouse, parrot, lizard, and
eagle are not isolated concepts. They are different readings of the same field.

When changing an operator, imagine the ecological story you are telling:

- Should this resource prefer water or avoid it?
- Should it care about moving sun or total sun?
- Should salt matter?
- Should height matter directly, or only through slope and water?
- Should this be a broad habitat or a narrow edge habitat?

The table is small enough that every symbol should have a reason.

## Interpolation And The Shape Of Opportunity

Interpolation is often discussed as a visual technique, but in a simulation it
can become a behavioral technique. If beings sample or move through fields that
have been smoothed, sharp environmental changes become gradual. If fields remain
coarse, beings may encounter abrupt thresholds.

Neither choice is automatically better. Smooth fields can create more natural
gradients. Sharp fields can preserve meaningful barriers and habitat edges. The
right choice depends on the question.

For example, if you are studying gradual migration across a resource gradient,
interpolation may help. If you are studying shoreline bottlenecks, too much
smoothing may weaken the very bottleneck you care about. If you are debugging
rendered terrain only, interpolation artifacts may look like model artifacts
unless numeric logs are checked.

The safe approach is to treat interpolation as a parameter with downstream
effects, not as a harmless display improvement.

## Tide As A Social Force

Tide is a good example of environmental coupling. On the surface, tide is about
water level. In practice, it can alter travel, food access, crowding, conflict,
and fatigue.

Imagine a shoreline region with shellfish or seaweed. At one tide level, beings
can access a resource easily. At another, they must detour or swim. The detour
changes travel distance. Swimming changes fatigue. Detours and narrow dry routes
increase encounters. Encounters increase social events. Social events become
memory. A tide constant has become a long-term social influence.

This is the kind of chain the book wants readers to notice. Environmental
variables rarely stay environmental.

## Weather, Light, And Behavioral Timing

Weather and light create temporal texture. A world with sunny day, cloudy day,
rainy day, clear night, cloudy night, rainy night, and dawn/dusk values gives
the simulation repeated phases. Those phases can organize behavior even when no
being explicitly "understands" weather in a human sense.

Light can affect rendering, movement interpretation, resource values, and
observer attention. Rain and cloud can affect how the world is presented and how
future behavior might be extended. Lightning and wind add additional fields that
can be sampled, drawn, or used for future decisions.

When adding weather effects, avoid the trap of making every weather value
immediately dramatic. Subtle effects are often better because they let long-term
patterns emerge. A small change in movement cost, visibility, or resource
availability can produce richer behavior than a large hard-coded reaction.

## Tiles And The Move Toward Larger Worlds

Tile support shows that ApeSDK is not limited to one fixed map story. A tile is
a way of organizing larger terrain, weather, and topography into manageable
pieces. This matters for planet-style work because scale changes the questions
you can ask.

On a small map, every being is potentially part of one social ecology. On a
larger tiled world, local histories can diverge. A lineage may adapt to one
region while another lineage develops elsewhere. Resource patterns can become
regional rather than global. Migration becomes more meaningful.

The engineering challenge is to increase scale without losing determinism or
observability. Large worlds are only useful if the state can still be inspected,
saved, tested, and explained.

## Environmental Metrics

A good environmental experiment should log environmental and behavioral data
together. Do not only log final population or visual snapshots. Capture the
mediating variables:

- water level;
- local resource classification;
- tile or map coordinates;
- movement attempts;
- swimming state;
- fatigue;
- food events;
- crowding;
- social event counts;
- selected being paths.

With those variables, a result can be traced. Without them, a run may look
interesting but remain uninterpretable.

## A Concrete Change: Making Rockpools More Important

Suppose you want rockpools to matter more. The weak approach is to increase a
food reward and observe that beings gather. The stronger approach asks what
rockpools are in the environmental grammar.

You would inspect:

1. the operator row defining rockpool suitability;
2. water and tide constants that affect shore exposure;
3. food or inventory paths that use shore resources;
4. movement costs near water;
5. event storage for eating, fishing, pickup, or conflict;
6. rendering or command output needed to observe the change.

Then you would run fixed seeds with one change at a time. If beings gather near
rockpools only when tide exposes them, you have an environmental mechanism. If
they gather regardless of tide, you may have created a static food hotspot
instead.

## The Reader's Rule

Whenever a behavior changes, ask whether the environment changed first. In
ApeSDK, many social and biological effects begin as geography.

## Case Study: A Food Hotspot That Becomes A Social Hotspot

Suppose a terrain change creates a high-value food region. The immediate result
is obvious: beings may move there to eat. The less obvious result is social.
Beings that share the hotspot encounter one another more often. More encounters
can create more social graph entries, more episodic memories, more competition,
more mating opportunities, and more crowding.

If the observer only looks at the final social graph, the result may appear to
be a new social preference. In reality, the preference may be downstream of
resource geography. The hotspot caused co-location; co-location caused social
history.

This is why environmental experiments need location logs. For each social
change, ask:

- did beings spend more time in the same places;
- did food events cluster before social events;
- did crowding rise before conflict;
- did fatigue rise because beings traveled farther;
- did mate goals change after repeated co-location?

The answers determine whether the social result is truly social or environmental
in origin.

## Case Study: Weather As Observation Bias

Weather can affect the model, but it can also affect the observer. A rainy or
dark view may make certain movement patterns harder to see. A bright terrain
view may make resource boundaries appear stronger. An overlay may combine with
weather colors in ways that change visual emphasis.

Before concluding that weather changed behavior, compare numeric state. Did
movement change, or only appearance? Did food event counts change? Did the same
seed with the same weather but different overlay produce the same interpretation?

This does not diminish weather. It clarifies it. Weather can be both a model
variable and a visual variable. Good observation keeps those roles separate.

## Environmental Parameters As Experimental Treatments

A terrain or weather change should be treated like an experimental treatment.
Define the baseline, define the treatment, and decide what response variables
matter before looking at the run.

For example:

```text
Treatment: increase tide amplitude
Primary expected effect: more shoreline movement constraint
Secondary expected effects: more swimming, more fatigue, more crowding
Metrics: water crossings, fatigue, shoreline occupancy, social events
Control: same seed, same population, same food settings
```

This pre-declared structure prevents interpretation from chasing whatever looks
interesting afterward. It also helps distinguish intended effects from side
effects.

## Reading The Terrain Code Patiently

Terrain code can be dense because it combines arrays, bit shifts, interpolation,
and resource classification. Read it in layers:

1. Find the base dimensions and constants.
2. Find how land is seeded.
3. Find how topography is stored.
4. Find how high-resolution values are derived.
5. Find how water and tide are calculated.
6. Find how operators interpret environmental components.
7. Find how rendering consumes the result.

Do not try to understand every expression at once. The important first pass is
to identify which functions create fields, which transform fields, and which
sample fields.

## Rendering Environment Without Hiding Model State

Environmental rendering should reveal structure without pretending to be the
structure itself. A color map can show terrain, but the exact numeric value is in
the model. A tide overlay can show water change, but the water test is in code.
A resource visualization can show likely habitat, but the operator table defines
the meaning.

Good environmental UI gives the observer a path back to state. If a region is
colored as resource-rich, a command or debug output should eventually be able to
explain which components made it rich. This keeps visuals from becoming
decorative guesses.

## Adding A New Resource

A new resource is a cross-layer feature. It may need:

- an operator definition;
- a food or inventory representation;
- energy value;
- terrain sampling;
- behavior that seeks or uses it;
- episodic events when it is consumed or handled;
- rendering or command visibility;
- transfer support if persistent;
- deterministic tests.

Skipping any of those may be acceptable for a prototype, but the omission should
be explicit. Otherwise a "resource" may exist in one layer but not in the
model's observable life.

## Chapter Four Checklist

Before moving on, make sure you can:

- explain why terrain is active, not passive;
- read the operator table as ecological grammar;
- describe how tide can become social pressure;
- distinguish weather behavior from weather visualization;
- explain why interpolation can alter behavior;
- design a fixed-seed environmental treatment;
- trace a resource from field to food or event.

The next chapter moves from world pressure to the beings that must survive
inside it.

## Open Design Questions

The environmental model can grow in several directions. More resources could
make the ecology richer. More weather effects could make time matter more. More
tile detail could support larger worlds. More rendering could make patterns
easier to see.

Each direction carries a cost. More resources require more event, food, and
observation support. Weather effects need tests proving they do not introduce
flaky behavior. Larger worlds need better summarization because a human cannot
inspect everything manually. Richer rendering needs numeric confirmation so it
does not become visual storytelling without evidence.

The safe path is incremental: add one environmental concept, expose it, test it,
then observe its downstream effects.

## Practical Exercise

Choose one resource row in the operator table. Write a plain-language
interpretation of every symbol in that row. Then predict where the resource
should appear on the map. After that, inspect the rendered or sampled terrain and
compare prediction to output.

If prediction and output differ, do not assume the code is wrong. The difference
may come from interpolation, water level, scale conversion, or another component
of the field. The exercise is meant to train environmental reading, not merely
confirm expectations.

## Environmental Glossary

Use these distinctions consistently:

- terrain is the physical field;
- resource is an interpretation of terrain and environmental components;
- weather is time-varying atmospheric state;
- tide is water-level movement over time;
- tile is a larger organizational unit;
- rendering is presentation of state;
- movement is a being-level response to the state.

Mixing these terms makes causal analysis harder.
