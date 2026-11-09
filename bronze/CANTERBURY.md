# CANTERBURY_SIM city-host profile

Define **CANTERBURY_SIM=1** together with **BRONZE_SIM=1** to build the Canterbury
host supplied by the parent Nosferatu rules project. All translation units must
use the same definitions: they change the native group/registry layout.

Ordinary builds retain `LARGE_SIM=256`, their original autonomous native loop,
Bronze township generation and existing save bounds. Canterbury defines
`CANTERBURY_CAPACITY=30000` and `LARGE_SIM=30000`, defaults to 27,000 initial residents with no special reserved slot.
Canterbury Details starts with 27,000 ordinary people and reserves 3,000 slots for growth. Other hosts may
request a smaller initial population; actor semantics remain host-owned.
Its current native body array occupies about 2.11 GB; actor storage adds about
113 MB. This is not a compact/mobile-memory implementation.

## Host contract

Call `sim_canterbury_configure(population, handler, context)` before `sim_init`.
Valid initial counts are 1 through 27,000; a NULL handler, invalid count or live
host reconfiguration fails. Call `sim_cycle` to initialize and advance the native
minute. The callback receives the actual `simulated_group`, after native time and
its BronzeClock advance. The city owns routines, human needs policy and movement
integration for that interval. The island's autonomous biology, social scans,
weather and movement loop do not also run. Project code must implement the human
and environmental behavior it requires; the flag alone is not a complete city.
The guarded `being_awake` getter returns the city-assigned awake state, so native
inspection does not recompute island day/water sleep policy. Native energy is
still the single human energy field; the parent supplies meals, sleep/fatigue, bounded
illness/care, mortality cleanup and pedestrian movement exactly once per minute.
`sim_close` clears the registered callback/context; configure again for a new run.

The parent host creates/activates a Bronze registry, stages an occupation policy,
then binds project person records to native IDs during its first callback. City
buildings are project-owned, not invented Bronze township/place IDs. Stock Bronze
settlement generation is skipped at Canterbury startup. Native coordinates,
actor identity and native time remain the shared authorities.

## Canterbury work arbitration

The guarded city branch calls `bronze_work_begin` before the project callback and
`bronze_work_canterbury_close` after it. The callback nominates stationary work
through `bronze_work_canterbury_claim(body, city_reason)`. The bridge verifies the
open native tick, sample, native needs, position and prior consumption, then
reuses the ordinary `consume` accounting and one-unit native energy cost. City
survival/care/travel can refuse a grant. Pending Bronze duration requests use the
same remainder and diagnostics, rather than a parallel work counter. No generic
Bronze DSL/township dispatch is inserted into Canterbury's callback.

These entry points and the changed branch are entirely `CANTERBURY_SIM`-guarded.
Ordinary `bronze_work_begin/end/claim` scheduling and default code remain unchanged.
The parent owns intention cancellation, serial completion effects and all city
place references. Native energy is only a compatible body facade for vampires;
parent supernatural state owns their survival, not island metabolism.

## Scale changes

- Native initialization avoids scanning 27,000 existing names for uniqueness.
  Native display names may repeat; stable actor IDs and project person records
  provide identity. Ordinary naming behavior is unchanged.
- City-mode actor lookup uses a bounded 65,536-entry stable-ID hash index and
  record-to-native-slot mapping. Deletion uses tombstones, reuse advances IDs,
  compaction updates physical bindings and restore rebuilds the index. Tombstone
  reclamation bounds lookup degradation during long-running birth/death churn.
  Sparse retirement examines live slots; rejected resets preserve the live registry.
- Full rebind validates duplicates in linear time before changing bindings.
- Bronze sidecar bound is 256 MiB in Canterbury, 16 MiB otherwise. Restore rejects
  duplicate actor IDs without an all-pairs scan. Capture also rejects inconsistent
  public actor-ID records rather than emitting an invalid payload. The encoded schema is unchanged;
  the larger profile is required to admit a full-population payload.
- Initial population requests stop at available capacity rather than looping
  forever when no slot remains.

## Validation and remaining work

The parent `rules/` executable's `--test` covers 26,999 humans plus the native Count, movement/clock,
full rebind, death/compaction/rebirth and sidecar roundtrip/corruption. The ordinary
SDK `bronze/contract/check.py`, actor and hybrid harnesses remain compatibility
checks. The parent `rules/tools/check_profiles.py` additionally verifies matching
ordinary layouts and preprocessed baseline code when CANTERBURY_SIM is absent.
All new support remains behind CANTERBURY_SIM. The parent `rules/PLAN.md` lists the 30 implementation cycles.

Do not treat the existing paired native/Bronze save alone as a Canterbury game
save. Household, route, community, supernatural, order and event state need an
atomic project extension with content hashes and validation. The current parent
CLI now has project-coordinated `--save`/`--load` checkpoints; see `rules/PERSISTENCE.md` in the parent project. Detailed human physiology, complete production/services,
weather, new-city-birth admission, graphical presentation and campaign
systems also remain project work. Dense native body storage and multi-session
save staging require memory qualification before production use.

## Project action layer

Parent cycles 16–20 add queued orders, feeding/damage, physical earth boxes and
paid cart custody, mode-specific traversal and direct sensory observations in
`rules/src/`. They consume the existing guarded work bridge and native bodies;
no additional SDK source changes were required. The world host derives vampire
rest from a usable co-located owned earth container. Cart/passenger integration
still has one movement owner and uses stable IDs across compaction. Rendering
additions are diagnostic project SVG/CSV; the native live interface remains a
parent project milestone. Full order/earth/perception state is captured by the parent project atomic
coordinator together with native bodies and this sidecar.


## Parent Cycle 27 checkpoint seams

Under `CANTERBURY_SIM` only, `land_canterbury_snapshot/install` provides complete
pointer-free land capture/install at a serialized checkpoint boundary.
`bronze_actors_canterbury_slot_id` exposes a staged registry's slot order without
activating it. The parent validates all project/native/registry state before
installing land, bodies and bindings. These additions change no native/Bronze
layouts or non-Canterbury preprocessed behavior; profile checks now include
`sim/land.c`. Production format, migration and memory qualification belong to
`rules/PERSISTENCE.md` and `rules/build/persistence-validation.json`.

Cycle 28 adds a guarded `draw_canterbury_map` indexed plot target for native
city hosts (512–4096 pixels). It calls the existing Bronze hybrid draw boundary;
project policy, basemap palette/resource, camera and picking remain in `rules/`.
The overlay receives distinct plot-buffer and world contexts. Canterbury returns
before default island-agent compositing so camera-transformed city agents are
not drawn a second time in island coordinates. Ordinary profiles are unchanged.

City checkpoints may use the guarded caller-owned registry checkpoint and stopped
`sim_canterbury_install_storage` seams. They reject dynamic DSL/program pointers,
preserve complete pointer-free actor records, native bodies, land and closed work
clock, and install staged storage without a startup tick. Other profiles are unchanged.
