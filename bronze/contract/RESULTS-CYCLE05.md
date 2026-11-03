# Cycle 05 validation record

Date: 2026-11-03 (America/Los_Angeles). Local Darwin arm64, Apple clang and
Xcode. No live graphical acceptance or remote Linux CI run is claimed here.

## Delivered behavior

Registry-owned observations of the native clock, explicit scenario-day/duration
conversion, native energy/drive/wakefulness/state observations, and bounded
per-actor work arbitration. Precedence is death, survival, sleep, biological/
social obligations, then work. Initial urgent needs cannot be erased by a meal
later in the same interval. Sidecars retain reasons, phases, transition times,
interruptions/resumptions, work units and remaining duration.

Guarded sim-cycle hooks bracket native action execution. A guarded awake hook
lets a safe stationary intention own locomotion before native movement is
integrated. Native biology, feeding, social behavior and metabolism continue.
End-of-tick validation rejects work if the actor actually moved or has a new
urgent need. Tasks and prototype gathering share one unit per actor/tick, with
one native energy unit charged per granted unit. No per-frame scheduler or
second biological integrator was introduced.

## Checks

| Check | Result | Evidence |
| --- | --- | --- |
| Full deterministic suite, including work tests | PASS | `/private/tmp/bronze-cycle05-tests-final.log` |
| Contract, compiler/CLI/AST and native repeatability/differential checks | PASS | `/private/tmp/bronze-cycle05-contract.log` |
| Targeted sanitizers: integration ASan; registry/work/harness ASan+UBSan | PASS | `/private/tmp/bronze-cycle05-sanitizers-final.log` |
| Bronze Catalyst Debug and Release, project signing settings, no override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle05-build-{Debug,Release}.log` |
| Original non-BRONZE_SIM sim-mac Debug build | BUILD SUCCEEDED | `/private/tmp/bronze-cycle05-original-build.log` |
| Packaged Debug/Release scenario/controller checks | PASS | `/private/tmp/bronze-cycle05-bundle-{Debug,Release}.log` |

The work harness covers conversion bounds, midnight rollover, forward and
backward discontinuities, duplicate tick calls, starvation, sleep, social and
childcare state, position/speed deferral, native energy costs, blocked and
interrupted intentions, resumption including one-unit completion, and rejecting
a second task/production grant in the same interval. Real native cycles execute
a stationary intention while native meals interrupt and then release it.
Different real map/terrain redraw schedules produce the same clock, native body
and work outcomes; pause freezes native time, elapsed time and work units.
Allocator scribbling/perturbation remains enabled for session/work tests.

## Baseline review

Only four `BASE bronze` rows in the Darwin-arm64 resource baseline changed:
cycles 60/120 for seeds 42/1337. Earlier prototype gathering credited nearby
moving/occupied beings; it now defers them through the native action gate.
Those resource counts become zero and site stocks remain full at these sample
times. Focused hybrid tests still demonstrate positive, bounded production for
eligible stationary native actors. Native body, terrain, weather and lightning
rows remain identical, including the Bronze probe's sampled native state.
Original-engine, CLI and AST golden values are unchanged.

## Limits

This cycle supplies intention/arbitration infrastructure, not automatic vocation
assignment, DSL instruction dispatch, travel, cargo or accounted meals. The
native clock/sidecars survive internal world switches in memory; native-only
saves do not serialize them. Paired persistence remains cycle 16. Prototype
site/economy file-static ownership remains unchanged. No shared native being or
save layout was modified. All new shared hooks are `#ifdef BRONZE_SIM`.

Full-engine UBSan remains limited by the original genetics signed shift already
reproduced without BRONZE_SIM in cycle 04. The instrumentation split above is
explicit; a fully UBSan-clean original engine is not claimed. An initial
sandboxed Xcode attempt could not launch Swift macro services; authorized
builds with access to those services are the build evidence listed above.
