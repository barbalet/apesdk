# Chapter 14 - Bronze: Occupations and Economies on Native Beings

## Extending the world through a build profile

The earlier chapters explain ApeSDK's native island simulation. `BRONZE_SIM`
adds an economic and cultural layer to that engine: occupations, learned skills,
work intentions, resource stores, recipes, settlements and resumable tasks.
The useful architectural change is that these systems work through actual native
beings. A worker has the same body, position, energy and survival pressures that
the rest of the engine observes.

The profile is selected by the consuming build. Shared-core hooks are guarded
with `#ifdef BRONZE_SIM`, and shared Swift branches use their own compilation
condition. Ordinary targets omit the definition and do not acquire a dependency
on Bronze content or symbols. Defining a presence-tested macro to zero does not
disable it. `HIDE_THUNDER` is a separate choice: the Bronze Catalyst target sets
it to suppress lightning while retaining the remaining weather behavior.

Start with the [Bronze package guide](../bronze/README.md), then use
[source ownership](../bronze/OWNERSHIP.md) and the module guides below. Some early
cycle documents retain the limitations of their original milestone. Later task,
township and save documentation records the implemented extensions.

## One compiler, distinct execution contracts

The canonical parser, AST and semantic compiler live in
[`bronze/dsl/`](../bronze/dsl/). The standalone interpreter in
[`bronze/runtime/`](../bronze/runtime/) and the native adapter consume that shared
implementation. The standalone program has its own world and agent types; it
is a compatibility host, not the graphical application's native population.
A successful standalone run therefore does not establish that the same source
has complete graphical semantics.

Strict compilation and legacy-runtime parsing are explicit contracts. Strict
callers require a compiled program. The standalone compatibility path retains
its historical interpretation and defaults. Failed strict compilation does not
silently retry through compatibility, and a legacy AST cannot simply be marked
compiled. This keeps an accepted declaration from masquerading as implemented
behavior.

The graphical scenario controller stages definitions and diagnostics before
replacing the active scenario. Successful selection resets the native world
with the chosen seed; a rejected file leaves the preceding world active.
The bundled extended scenario contains 120 vocations and 840 authored tasks,
within the native vocation limit of 128. Its declaration-only compatibility
defaults supply missing economic settings. Descriptive world labels remain
annotations, and missing recipes still produce an unavailable result.

## Identity that survives movement in memory

[`bronze/actors/`](../bronze/actors/) gives each live native being a stable
registry-scoped actor ID and an owned sidecar. The sidecar contains vocation,
skills, task state and economic information without adding those fields to
the native being's historical serialized layout. Native position and energy
remain on the body.

An array index is a location in storage, not an identity. Death can compact the
native array; the binding moves while the actor ID stays with its person.
Birth allocates a new ID, retirement releases dependent claims, and resetting
a registry advances its identity history. Reusing a record's memory does not
reuse its former actor ID. Names, genetics and dates likewise make unsuitable
substitutes for the registry handle.

Bindings also depend on the active world. Metadata may be inspectable in an
inactive registry, but native-body resolution must wait until the matching
snapshot is restored and rebound. Keeping a body pointer across session
replacement bypasses that protection. Extensions should retain logical IDs
and reacquire short-lived bindings at the serialized simulation boundary.

## Work must fit inside a native minute

[`bronze/work/`](../bronze/work/) measures time from native land date and time.
A native day contains 1,440 minute cycles. Scenario durations become bounded
integer work requests; a second wall-clock scheduler never grants additional
production while the world is paused.

The work gate samples native needs and position. Hunger, sleep, biological or
social obligations and movement can prevent a stationary work grant. A pending
task records its remaining duration and reason for interruption. When conditions
permit work again, it resumes from that remainder. Completion charges and
effects must occur once, even if the interface observes the actor many times
during the interval.

This changes how a failed task should be investigated. A worker with a valid
occupation and recipe may still be travelling, hungry or unable to reach the
site. Increasing a production counter would conceal the actual cause. The
selected-actor view exposes task, operation, source line, progress and blocking
reason, connecting the economic result to the body's available time.

## Executing a task without inventing travel

The [task executor](../bronze/tasks/README.md) retains nested statement cursors,
branch outcomes, target places and remaining minutes. Each actor has its own
deterministic task-selection random stream. Bounded traversal prevents an
authored program from monopolizing a native tick, and failed operations record
their source before the retry policy becomes eligible.

Supported operations include travel, gathering, crafting, fetching, depositing,
eating, exchange, paid delivery and rest. They have concrete location and
inventory requirements. Gathering acquires actual patch stock; eating consumes
carried food and changes native energy; a delivery requires participants and
goods at the appropriate entrance. An unsupported operation reports that state
instead of granting fictional progress.

Routes use the wrapped native landscape and bounded search. Dry-ground and
slope restrictions qualify cell transitions. Township tracks influence weighted
route choice and solid footprints block entry. The scheduler steers toward
waypoints, but native movement integrates the physical step and charges its
movement cost. Arrival is checked against the observed body. Survival and
native obligations can interrupt travel, and a displaced actor must replan
from its actual position.

## Making resources, places and recipes agree

The [hybrid layer](../bronze/hybrid/README.md) owns economic sites and settlement
state. Fields, woodland and mines attach stock to places in the native world.
Township homes, service entrances, storage and tracks supply operational
destinations. Their identity and geometry matter to allocation, routing and
rendering, so an arbitrary screen symbol is not a replacement for a real site.

Resource claims stop competing actors from promising the same stock twice.
Crafting reserves all required inputs, checks the workplace, skill, tool and
capacity conditions, and credits outputs only at completion. Cancellation
releases the outstanding reservation. Personal cargo and communal storage have
bounded capacity; an actor cannot carry unlimited resources simply because a
scenario lists them.

Bronze economic inventories are separate from the native body-slot inventory
used by original being behavior. That separation preserves the original model
while making ownership explicit. Tools remain in cargo unless a recipe actually
consumes them. The bundled recipe language does not imply a wear or repair
system merely because a crafted item is called a tool. Read the operation's
implemented contract before inferring behavior from its name.

## Saving bodies and commitments together

The [combined-save format](../bronze/SAVE-FORMAT.md) preserves native state with
its matching Bronze sidecars. Keeping only native bodies would lose stable
economic identities, inventories, reservations and task cursors. Conversely,
restoring a sidecar against different bodies or content could assign obligations
to the wrong actors.

The format and loader therefore validate the paired state and its supported
content contract. A useful continuation test saves during an interrupted task,
restores, and compares subsequent inventory, identity and progress with an
uninterrupted run. Merely reopening a file and seeing agents on a map does not
test those obligations. A Canterbury host adds further city-owned records and
needs its own atomic coordinator, as Chapter 15 explains.

## Building and checking the profile

Open [`toolchains/bronze-mac/bronze-mac.xcodeproj`](../toolchains/bronze-mac/bronze-mac.xcodeproj/)
and choose the shared `bronze-mac` scheme for My Mac (Mac Catalyst). The project
builds `BronzeSim.app`, bundles its scenarios and supplies the required profile
definitions. Local Debug and Release builds have signing disabled in the
checked-in project; distribution signing is a separate configuration.

From the SDK root, the documented regression entry points are:

```sh
bash test.sh
make -C bronze test
make -C bronze contract-test
```

These exercise different boundaries: native deterministic behavior, Bronze
scenarios and native integration, and compatibility/profile contracts. A change
to shared hooks needs evidence for ordinary builds as well as Bronze. Recorded
baselines should change only when a reviewed behavior change justifies them.
For graphical work, also build the real app and check bundled resources and
presentation. The central invariant is observable throughout: economic rules
extend the native world through explicit identities, finite resources and
available native time.
