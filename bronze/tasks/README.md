# Resumable Bronze tasks

Cycle 09 runs the strict compiled scenario on native ApeSDK actors. A scenario
controller, pending world and active world can each retain the compiled
program. The active actor cursors therefore remain valid while a new scenario
waits for a native reset. Reset establishes a fresh execution state.

## Selection and progress

The scheduler takes the same hunger and fatigue readings as the native work
gate. It visits each rule once per selection and samples eligible weights with
a weighted reservoir. Boolean conditions short-circuit. Each actor owns a
SplitMix64 stream seeded from the scenario seed and the actor’s ordinal within
the world. Native randomness and other actors do not consume that stream.

The actor keeps a nested statement cursor, the completed operation count, the
pending operation, its target place ID and the minutes left. A branch is
evaluated when reached, and its result stays in the cursor during interruptions.
Traversal is capped at 32 statement or stack steps per native tick. Graphical
admission allows up to 256 tasks and 256 rules per vocation.

The guarded scheduler prepares operations before native movement and commits
after the native work gate. A paused world has no native work intervals.
Survival, sleep and social obligations can interrupt a pending operation.
When the native work gate grants time again, the operation continues from its
remaining duration. Death or reassignment cancels its pending work. A failure
ends the current task, records the operation and source line, and lets native
behavior continue. The actor retries rule selection after one native hour.
Normal completion allows selection again on the next native tick.

## Operations available in this cycle

| Operation | Behavior |
| --- | --- |
| `move_to` | Resolves an owned matching site and confirms arrival after one gated minute when the actor is already on its footprint. An actor outside the footprint reports travel required. |
| `gather` | Requires the assigned production patch and its matching resource. Takes one gated minute per requested unit, rounded up. On completion the full quantity moves from the patch to its township store and the actor gains skill. |
| `rest` | Takes two stationary minutes and reduces native fatigue by one drive unit on completion. It incurs native metabolism with zero additional work energy cost. |

Gathering supports grain, copper, tin and charcoal, the existing township
stores. Stock and arrival are checked before work and again at commit.
Competing workers can spend time attempting the same stock, but only available
stock is transferred and only a successful commit awards skill. Gathered goods
go directly to the owning township pending actor cargo and transport.

Other admitted operations report their name and an unavailable status when
reached. Cycle 10 adds travel, and later cycles add inventories, consumption,
crafting and trade. The old proximity loop no longer grants production. The
selected-actor panel shows task, operation, source line, progress and reason.
