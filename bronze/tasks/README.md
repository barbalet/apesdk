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
| `move_to` | Routes to an owned matching site, the assigned `workplace`, or the township `service` entrance. Arrival is confirmed after one gated minute on the target. |
| `gather` | Routes to the assigned production patch if needed. Once there, gathering its matching resource takes one gated minute per unit, rounded up. On completion the full claimed quantity moves into actor cargo and the actor gains skill. |
| `craft` | Routes to the recipe workplace, reserves the inputs, and credits its output to cargo after the required work time. |
| `deposit` | Routes to communal storage or the actor’s household store. Transfers carried resources or items after one granted minute at the entrance if storage has room. |
| `fetch` | Uses the same entrances and work gate to transfer available resources or items into personal cargo, subject to capacity and communal food rations. |
| `eat` | Consumes carried food after one granted minute and applies its scenario nutrition through native energy. |
| `trade` | Exchanges carried goods with another eligible actor at the township service entrance. |
| `serve` | Delivers a carried good for payment at the provider’s assigned service entrance. |
| `rest` | Takes two stationary minutes. The native drive cycle handles recovery, with zero additional work energy cost. |

Gathering follows the active resource definitions and patch habitat. Stock,
carrying space and arrival are checked before work and at commit. Claims
prevent competing actors from reserving the same stock. See the
[inventory and renewal rules](../hybrid/README.md).

Other admitted operations report their name and an unavailable status when
reached. The selected-actor panel
shows task, operation, source line, progress and reason.

## Trade and paid delivery

Declare `action trade 2 3` or `action serve 2 3` in the scenario. Both
operations take two distinct declared resource or item names and an optional
positive quantity that defaults to one.

- `trade copper grain 2` exchanges two copper for one grain.
- `serve meal copper 2` delivers one carried meal for two copper. The
  provider must be assigned to the service entrance.

The scenario author authorizes these exchanges for eligible township
participants. Both beings must be assigned, ready for native work, stationary,
and present at the entrance. The lowest stable actor ID among participants
who can complete the exchange takes part. Both inventories are checked again
after the work gate grants a minute. Missing participants, insufficient
stock, and full cargo defer delivery and leave both inventories intact.

The actor sidecar records the latest exchange outcome and partner separately
from the beings’ episodic memories. Successful exchanges can add a paired
memory at most once per native hour per participant. Successful gathering
and crafting have a separate hourly limit for work memories.

The graphical executor supports paid delivery through `serve`. The separate
service-profile declarations, disruption simulation and the full scenario
remain outside the graphical admission rules.

## Routing

Search uses the four-way wrapped native map. A route starts on dry ground and
can cross a cell edge when both cells are above the maximum tidal waterline
and their height difference is at most 18. The search chooses the first
reachable cell on the target footprint, or the actual anchor for a service
entrance. The chosen place is retained while the operation runs.

The scheduler allows at most two searches per tick, 16,384 visited cells per
search, and 256 waypoints per route. A blocked route aborts the current task
and uses its one-hour retry. Sixteen consecutive rejected steps or 2,048
steering attempts also defer the route.

Steering runs immediately before native movement and can only lower the native
speed to a maximum of 12. It checks the predicted step against the current or
next route cell and the native movement capability callback. Native movement
integrates the step and charges its normal energy cost. Survival, sleep, social
obligations, native goals and external movement take precedence. Pauses retain
the route and the task cursor. When the actor leaves the expected cell, routing
starts again from the observed native position. Reassignment cancels the route.

The actor panel distinguishes travelling, paused travel, arrival and blocked
routes. A site operation earns stationary work time after the actor reaches its
target. The dry-land and search limits intentionally defer some routes that a
future travel policy might support.


## Recipes and tools

`craft NAME [BATCHES]` accepts a declared recipe name or an item with a unique
producing recipe. Batches are whole numbers from 1 to 1,024 and default to
one. The graphical host accepts up to 64 recipes and 64 item kinds. Recipes
require positive inputs and output, with each kind declared unambiguously as a
resource or item.

| Optional `world` setting | Default | Behavior |
| --- | --- | --- |
| `recipe_<name>_minutes` | 4 | 1–1,440 granted native minutes per batch |
| `recipe_<name>_workplace` | `service` | `service` uses township storage at the service entrance. `workplace` uses personal cargo at the assigned workplace. |
| `recipe_<name>_tool` | Omitted | Requires one carried unit of a declared item |
| `recipe_<name>_minimum_skill` | 0 | Requires this level of the current vocation skill |

Shared inputs are reserved together before work starts. Personal inputs stay
locked in cargo while their recipe is active. Completion rechecks location,
inputs, skill, tool and carrying capacity, allocates space for the output,
then debits the inputs and credits the full output in one commit. A
cancellation or expiry releases the reservation. Successful crafting adds one
practice gain. Tools required by policy remain in cargo unless explicitly
listed as a recipe input. The bundled recipe language has no wear or repair
operations.

Resources and items use Bronze inventory units. The native body-slot inventory
continues to serve native being behavior. A crafted item can be deposited at
township storage, where items and resources share capacity.
