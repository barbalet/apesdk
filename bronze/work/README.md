# Native clock, needs and work arbitration (cycle 05)

This module is the production Bronze work gate. The separate legacy CLI and
compiled-task pilot retain their compatibility semantics and are not called by
this scheduler. Professions, DSL instruction dispatch, navigation and accounted
food/inventory effects remain their later plan cycles.

## One clock

One native `sim_cycle()` advances `land_time()` by one minute, with native date
rollover. The absolute timestamp is computed in 64 bits as
`land_date() * TIME_DAY_MINUTES + land_time()`. A registry owns only an origin
and the last observed native timestamp, not an independently incremented clock.
Scenario day zero starts at native world initialization/reset/import. Elapsed
scenario days are `(observed_tick - origin) / TIME_DAY_MINUTES` (1440 ticks/day).
Internal session restore keeps that world's origin and observations alongside
its actor records. Native-only imports establish a new origin and identities;
paired persistence remains cycle 16.

`bronze_work_duration(days, &ticks)` converts explicit scenario-day durations
using ceiling, minimum one minute, rejecting negative/nonfinite/overflowing
input. This does not reinterpret quantities such as `gather grain 4` or execute
DSL statements. The future dispatcher must use explicit duration semantics.

Only guarded `sim_cycle()` hooks open/close work intervals. Pause and redraws
do not open intervals. Repeated calls at the same timestamp cannot grant extra
work. Forward time discontinuities process at most one unit, never a catch-up
loop. A backward timestamp fails closed with a clock reason until time exceeds
the last observation or the world is reset. Hosts replacing native state must
use the registry reset/import path rather than editing the clock.

## Native needs and precedence

Needs are read on demand from real beings; Bronze maintains no second hunger,
fatigue, position, wakefulness or energy integrator. Hunger is the larger of
the native hunger drive / 255 and `1 - native_energy / BEING_FULL`, clamped to
0–1. Fatigue, social and sex use native drives / 255. Wakefulness uses native
`being_awake()` (including time-of-day and water behavior). Native state bits
are also observed. Precedence is:

1. Dead (`energy <= BEING_DEAD`): no work.
2. Survival: energy at/below `BEING_HUNGRY`, normalized hunger >= 0.90, or
   native hungry, eating or swimming state. Native eating is respected even
   when the being is topping up rather than critically starving.
3. Sleep: not fully awake, or native fatigue drive >= 0.90.
4. Biological/social obligations: social or sex drive >= 0.90, or grooming,
   suckling, aggression/showforce, speaking or shouting state.
5. Available stationary work; otherwise native movement has priority over
   opportunistic prototype production.

Both the initial and post-native-action observations apply. An urgent initial
need cannot be cleared to authorize work during the same tick by a later meal
or recovery. A newborn waits until its first complete sampled interval.
Thresholds above are explicit conservative policy constants for this cycle,
not scenario-bound parameters or a claim to reproduce legacy CLI needs.

## One action budget and locomotion owner

`bronze_work_request(registry, id, task, ticks)` submits a stationary intention
to a live actor without replacing outstanding work. Native awake behavior,
social interactions, feeding, reproduction, drives, brains and metabolism
continue. After native awake decisions, a guarded hook may stop native
locomotion for a safe pending stationary intention, before the engine's single
movement integration. Survival/sleep/obligations bypass this hold. It does not
teleport or implement economic navigation. Existing native movement remains
unchanged when there is no intention.

At the end of the native cycle, work is permitted only if needs allow it and
the being actually stayed in place with zero speed. One unit decrements
`remaining_ticks` once and charges one energy through `being_energy_delta`;
normal native metabolic costs still apply. There is no second movement
executor. On completion the task becomes unassigned; advancing a DSL program
counter remains the future task dispatcher.

The hourly proximity economy uses `bronze_work_claim()` for the same per-actor
budget: a task and prototype gathering cannot both consume one tick. Mere
proximity or wakefulness no longer suffices. Site renewal and the aggregate
prototype recipe remain prototype behavior, not actor inventory accounting.
The known file-static site/economy ownership limitation is unchanged.

Each actor records current reason/phase, last blocking reason, transition tick,
transition/interruption/resumption counts, work units and duration remaining.
Phases distinguish blocked, interrupted, deferred, started, resumed, running
and completed. This is bounded diagnostic state, not an unbounded event log;
dead actors' records are retired by the existing identity lifecycle.

## Validation

`bash bronze/actors/test.sh` includes `test_work.c`, using real native beings
and sessions. It covers duration bounds, needs precedence, one-unit budgets,
actual feeding interruptions/resumptions, stationary locomotion ownership,
clock rollover/discontinuities, pause, real map/terrain draws and differing
window schedules with identical native/work outcomes. Existing lifecycle and
two-world tests remain part of the same harness. Sanitizer mode instruments
the new work module and harness with ASan+UBSan and the native engine with ASan
(the separately documented original genetics UBSan limitation remains).

`bash test.sh` includes these checks. `make -C bronze contract-test` audits
guarded hooks, unique Xcode source ownership, native baseline preservation and
independent ordinary-engine linking. Bronze-only prototype resource baselines
change in this cycle because native movement/needs now defer opportunistic
gathering. No original-engine golden values are changed.
