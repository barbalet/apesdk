# BronzeSim on ApeSDK: full implementation plan

Updated: 2026-11-03

## Goal and completion criteria

Deliver `toolchains/bronze-mac/bronze-mac.xcodeproj`, shared scheme
`bronze-mac`, as a complete Bronze scenario running on real ApeSDK beings,
terrain, weather, time, graphics, memory and social systems. Professions,
workplaces, inventories and tasks must belong to the same individuals the
user selects in the native View, Terrain and Control windows.

The full implementation is complete when the bundled
`content/bronze_age_full.bronze` drives the graphical app, every declared
operation and vocation has an explicit supported behavior, work produces and
consumes accounted resources, agents can reach their workplaces, economic
interactions affect individuals, and saving/resuming restores the combined
world deterministically. A large occupation list or decorative map markers
alone does not satisfy this goal.

This document defines 18 ordered development cycles. Cycles 01–05 are completed
(2026-11-02–03); cycles 06–18 remain planned. The existing prototype is a starting point, not proof
that the full scenario works. Each cycle should finish with working code,
focused validation, updated documentation and a recorded result before its
dependents begin. These are units of development, not calendar estimates.

## Non-negotiable split: original ApeSDK versus BRONZE_SIM

- Historical ApeSDK targets must keep their current behavior when
  `BRONZE_SIM` is undefined. Do not define it in a common engine header.
- The `bronze-mac` target defines `BRONZE_SIM=1` for Debug and Release.
  Bronze implementation lives under `bronze/`; the toolchain directory owns
  Xcode metadata, build settings and resource references.
- Every Bronze-specific include, callback, state field, action override,
  lifecycle hook or rendering hook added to shared ApeSDK C code must be
  enclosed in `#ifdef BRONZE_SIM`. Put the substantial implementation in
  `bronze/`, keeping guarded core hooks small.
- Preserve the original native path. An explicit `#else` is appropriate when
  Bronze behavior replaces a decision; an additive guarded hook is appropriate
  when native behavior still runs. Do not run both action executors for one
  actor in the same simulation interval.
- Keep `simulated_being` and historical native save layouts unchanged.
  Bronze state is part of each agent's combined simulation state through a
  stable identity-linked sidecar, not a second independent population.
- ApeSDK owns biological state, energy, drives, actual positions, the clock,
  terrain and native social/brain state. `BRONZE_SIM` owns vocation, learned
  occupational skills, workplace/home assignments, economic inventories,
  task progress, resource patches, settlements and economic accounting.
- Bronze reads native needs through an adapter and applies work/food effects
  through native APIs. Do not maintain an independently drifting second
  hunger, fatigue, position or time simulation.
- Add a Swift `BRONZE_SIM` compilation condition only if shared Swift needs
  a Bronze branch; use `#if BRONZE_SIM` there. A C preprocessor definition
  alone does not enable a Swift compilation condition.
- Keep `HIDE_THUNDER` independent of `BRONZE_SIM`. Both bronze-mac target
  configurations define `HIDE_THUNDER=1`; historical targets leave it undefined.
  Preserve ordinary weather in Bronze and lightning in historical builds.
- Do not modify the separate `/Users/barbalet/bronzesim/` checkout as part of
  this plan. Any required source reconciliation is performed in this repository.

Typical shared C integration:

```c
#ifdef BRONZE_SIM
#include "../bronze/hybrid/bronze_hybrid.h"
#endif

/* Native biology, environment and bookkeeping continue to run. */
#ifdef BRONZE_SIM
/* Submit/arbitrate an economic intention against native urgent needs. */
bronze_actor_update(...);
#else
/* Retain the original action-selection path where it is replaced. */
#endif
```

The snippet shows the boundary, not an existing API or implementation.

## Current baseline and known limitations

- `bronze-mac` builds a separate `BronzeSim.app`, bundle identifier
  `com.apesdk.bronze-mac`, using the existing Catalyst Swift shell and native C
  sources. It has a shared Debug/Release scheme with LLDB Run support.
- `hybrid/` currently places ten point sites by scan order above height 150,
  with spacing. Site types repeat; slope, moisture and geography do not yet
  distinguish fields, woodland, mines or townships.
- Nearby awake native beings contribute to aggregate resource counters.
  Cycle 04 now links actor IDs and sidecar records to real beings, with the
  schema for future professions, inventories and tasks. Those fields are
  unassigned/empty; production behavior is not implemented by adding the schema.
  The counters do not represent a completed settlement logistics model.
- Cycle 05 now gates both duration intentions and prototype gathering through
  native clock/needs/action arbitration. Native movement no longer earns
  opportunistic work simultaneously. Professions and DSL dispatch remain
  unassigned; explicit stationary intentions are exercised by real-engine tests.
- The graphical app now strictly validates bundled `first_slice.bronze` on
  startup and provides selection, active-file status and diagnostics. Successful
  loads reset native state with the scenario seed; other scenario behavior is
  staged, not executed. The full scenario still fails strict compilation.
  The separate legacy `bronze-sim` CLI defaults to `content/bronze_age_full.bronze`
  when run from `bronze/`.
- Cycle 02 consolidated compiler/AST/support ownership in `dsl/` and native
  binding ownership in `adapter/`. The CLI explicitly chooses the legacy-runtime
  profile; strict compiled callers share the same parser with stricter semantics.
  See `OWNERSHIP.md`. Some pilot actions remain placeholders: reporting movement
  completion is not evidence of actual travel.
- Existing `entity/profession.{h,c}` offers optional interaction/training
  helpers, but its fixed civilian profession enum is not the Bronze vocation
  registry and must not silently substitute for it.
- Prototype economic state resets on native new/load; native saves do not
  persist it. The full implementation will replace that provisional contract.

## Cycle 01 — Establish the compatibility and scenario contract

**Status:** completed 2026-11-02. See [compatibility and testing contract](contract/README.md),
[parsed capability inventory](contract/content_inventory.json) and
[fixed-seed baselines](contract/baselines.json).

**Depends on:** the existing prototype only.

Inventory shared engine hooks, the two adapters, both DSL copies, runtime
operations and every construct in the three existing content files. Build a
capability table identifying implemented, stubbed, rejected and missing
semantics. Record state ownership, units, coordinate conversions, action
cadence, supported hosts and the identity strategy to investigate in cycle 04.
Capture fixed-seed native and prototype baselines before replacing behavior.

**BRONZE_SIM boundary:** audit every existing core modification for an enclosing
`#ifdef BRONZE_SIM`; confirm historical targets do not define it. Verify both
Bronze configurations define `BRONZE_SIM` and `HIDE_THUNDER`.

**Exit evidence:** checked-in contract/capability documentation, reproducible
baseline commands, passing native tests and both toolchain build configurations.
Do not claim unimplemented content verbs work because parsing succeeds.

## Cycle 02 — Reconcile the Bronze compiler, runtime and adapter ownership

**Status:** completed 2026-11-02. See [canonical source ownership](OWNERSHIP.md)
and [validation record](contract/RESULTS-CYCLE02.md). Standalone compatibility
is explicit; the graphical app compiles the shared compiler but loading remains
cycle 03. The full scenario's strict rejection has not been hidden or relaxed.

**Depends on:** cycle 01.

Choose one canonical parser/compiler and one set of domain types inside
`bronze/`. Reconcile useful differences between `runtime/`, `dsl/`, the root
adapter and `adapter/`; migrate callers incrementally. Keep the standalone CLI
and current harnesses functional. Retain the small scenario-runtime contract
only where it helps interoperability. Remove duplicate implementation paths
only after their callers and behavior are covered.

**BRONZE_SIM boundary:** shared ApeSDK headers must not acquire Bronze types or
parser dependencies outside `#ifdef BRONZE_SIM`. The original build must not
compile or link the Bronze runtime to satisfy ordinary engine symbols.

**Exit evidence:** CLI, adapter tests and Catalyst app use the canonical compiler;
no ambiguous ownership of actions or competing adapter implementations remains.

## Cycle 03 — Load and validate bundled scenarios in bronze-mac

**Status:** completed 2026-11-02. Bundled strict loading, explicit reset-on-load
selection, active-file status and captured diagnostics are implemented. See
[validation record](contract/RESULTS-CYCLE03.md). Validation/staging does not
claim that scenario-defined professions, resources or tasks drive the prototype.

**Depends on:** cycle 02.

Bundle the three `content/*.bronze` files as Xcode resources. Load a small
explicit development scenario first; provide a deliberate scenario-selection
path and diagnostics. Resolve bundle paths instead of assuming a working
directory. Compile names to stable IDs, validate references, units and numeric
bounds, and report file/line errors without terminating the graphical app.
Reject unsupported constructs explicitly. Treat a scenario change as a world
reset unless a later migration contract explicitly supports it.

**BRONZE_SIM boundary:** scenario loading and the selection UI exist only in
Bronze. Shared shell hooks use `#ifdef BRONZE_SIM` / Swift `#if BRONZE_SIM`;
historical New Simulation and file behavior remain unchanged.

**Exit evidence:** a packaged app loads content outside the repository working
directory, reports malformed/missing scenarios, and identifies the active file.
Loading does not yet imply full behavior support.

## Cycle 04 — Give every real being a stable Bronze identity and sidecar

**Status:** completed 2026-11-02. See [identity/schema contract](actors/README.md)
and [validation record](contract/RESULTS-CYCLE04.md). Real beings now have
registry-scoped IDs; professions and economic behaviors remain later cycles.

**Depends on:** cycles 01–03.

Implement a world-owned actor registry mapping stable Bronze actor IDs to live
ApeSDK beings. Investigate existing identity fields for uniqueness and lifetime;
use a guarded identity hook if they are insufficient. Never use array index,
raw pointer or a potentially duplicated display name as persistent identity.

A sidecar contains actor ID, vocation ID, skill records, home township ID,
workplace ID, resource/item inventories, current task, instruction/progress
state, reservations, assignment history and mentor/apprentice links. Add these
incrementally as their owning cycles arrive; specify the schema now. Register
births, remove or archive deaths, release claims, and survive population
compaction. Persist the ID allocator later in cycle 16.

**BRONZE_SIM boundary:** birth/death/compaction hooks are `#ifdef BRONZE_SIM`;
`simulated_being` and historical serialization remain unchanged. There is one
live population, with Bronze records linked to its members.

**Exit evidence:** no sidecar follows the wrong being after death, birth,
reordering or selection changes; deterministic identity allocation is tested.

## Cycle 05 — Build one clock, needs adapter and action arbitration policy

**Status:** completed 2026-11-03. See [clock/needs/action contract](work/README.md)
and [validation record](contract/RESULTS-CYCLE05.md). Stationary intentions and
prototype gathering share a bounded native-tick budget; full task dispatch
and navigation remain their later cycles.

**Depends on:** cycle 04.

Use the ApeSDK clock as the sole authority; explicitly define conversion to
scenario days and task durations. Schedule Bronze work from simulation ticks,
never frame/window redraws. Map native energy, hunger, wakefulness and relevant
drives to scenario conditions. Define precedence: death, urgent survival,
sleep, biological/social obligations, then available work. Record why work is
blocked, interrupted, deferred or resumed. Bound work per tick.

**BRONZE_SIM boundary:** economic intentions enter action selection only inside
`#ifdef BRONZE_SIM`. Preserve native biology and essential behavior, and prevent
native movement plus economic movement from executing twice in one step.
Without `BRONZE_SIM`, retain the original decision path exactly.

**Exit evidence:** pause freezes both layers; extra windows do not advance work;
starvation/sleep interrupt work; changing render rate does not change outcomes.

## Cycle 06 — Derive a coherent Bronze landscape from ApeSDK terrain

**Depends on:** cycles 03 and 05.

Replace scan-order site typing with deterministic terrain analysis. Derive
height bands, slope, water/coast distance and a documented moisture proxy or
weather measure. Use the native coordinate and toroidal-wrap conventions.
Fields favor flatter habitable land; woodland forms contiguous suitable
patches; deposits favor documented geological/height/slope rules. Store areas,
capacity and depletion separately from graphical symbols. Parameterize thresholds
in scenario content; do not treat elevation alone as geology or forest cover.

**BRONZE_SIM boundary:** Bronze suitability and land-use overlays live in
`bronze/`; any native terrain/weather access additions are guarded by
`#ifdef BRONZE_SIM`. Do not rewrite native terrain to paint economic features.

**Exit evidence:** fixed seeds reproduce patches; fields satisfy slope/water
constraints; woodland is an area; deposits are bounded and have explicit types.

## Cycle 07 — Place townships, workplaces and settlement ownership

**Depends on:** cycles 04 and 06.

Place settlements on suitable terrain near accessible food, woodland and water.
Use reachability, spacing and capacity checks rather than arbitrary scan order.
Define home membership, workplaces, storage locations, production sites and
service places with stable place IDs. Assign each actor a valid home and resolve
crowding or unavailable settlements deterministically. Establish per-settlement
resource/item stores; aggregate counters become derived reports.

**BRONZE_SIM boundary:** settlement types, ownership and membership remain
Bronze sidecars. Guard any shared place-inspection hook with `BRONZE_SIM`.
Original ApeSDK remains a settlement-free simulation unless it already supplies
an independent native concept.

**Exit evidence:** homes/workplaces refer to existing reachable places; world
reset clears assignments; no resources appear merely from site creation unless
specified as initial scenario stock.

## Cycle 08 — Assign professions and track individual skills

**Depends on:** cycles 04, 05 and 07.

Resolve vocation definitions from `.bronze` into actor sidecars. Introduce
scenario-controlled initial occupation distribution, eligibility, skill scales,
workplace requirements and reassignment policy. Handle unassigned actors,
children, missing workplaces and labor shortages explicitly. Avoid a hard-coded
C profession list. Evaluate `entity/profession` helpers for compatible reuse;
keep Bronze vocation IDs distinct from the existing civilian enum.

**BRONZE_SIM boundary:** each being gains a Bronze vocation through the registry,
not a new historical core struct field. Profession assignment and skill changes
run only under `BRONZE_SIM`; native genetics and learned preferences retain their
existing ownership.

**Exit evidence:** selected being → stable actor → vocation/skills/home/workplace
is inspectable in diagnostics; assignments survive compaction and do not fall
back to anonymous proximity-based labor.

## Cycle 09 — Execute resumable profession rules and tasks

**Depends on:** cycles 03, 05 and 08.

Bind compiled conditions, weighted rules, nested `when`/chance blocks and task
operations to real actor sidecars. Evaluate eligibility once per selection;
random conditions must not be rerolled inconsistently between weight passes.
Use a deterministic Bronze RNG with explicit ownership. Track program position,
remaining duration, targets and interruption state across ticks. Define task
failure, cancellation and fallback behavior. Remove the prototype's unconditional
proximity production when the scheduler becomes authoritative.

**BRONZE_SIM boundary:** task dispatch is guarded by `BRONZE_SIM` at the shared
loop boundary. Execute against real beings through adapters; do not step a
parallel `BrzAgent` population beside them.

**Exit evidence:** identical seed/content/input yield identical task traces;
interrupted tasks resume correctly; unsupported operations fail visibly rather
than reporting false completion.

## Cycle 10 — Route workers to real places and resources

**Depends on:** cycles 06, 07 and 09.

Implement movement intentions and route progress using native positions and
movement rules. Respect water, slope, obstacles, world wrapping and actor
capabilities. Choose reachable patch access points and workplace entrances.
Distinguish travel started, arrived, blocked and cancelled. Work requiring a
site cannot complete before arrival. Bound route-search cost and retry behavior.

**BRONZE_SIM boundary:** route requests/arbitration hooks use `#ifdef BRONZE_SIM`.
Do not teleport workers, overwrite native positions after every step, or keep a
second authoritative Bronze position. Preserve manual selection/input routes.

**Exit evidence:** workers visibly travel to assigned sites; unreachable jobs
are deferred; native survival can interrupt travel without corrupting task state.

## Cycle 11 — Implement inventories, gathering and renewable resources

**Depends on:** cycles 09 and 10.

Use scenario resource definitions for capacity, nutrition, regeneration and
habitat. Add per-actor carrying limits, transfers, gathering duration and
resource claims. Gathering debits a real patch and credits the same amount to
an actor; storage deposits require actual presence and capacity. Renew fields
and woodland according to the agreed time/environment model; keep mineral
renewal explicit rather than assuming all resources regrow.

**BRONZE_SIM boundary:** resource stores and claims are Bronze-owned. Guard
native energy/work-cost effects with `BRONZE_SIM` and apply them once through
native APIs. Never create resources from a successful-looking event alone.

**Exit evidence:** conservation/accounting tests, no negative stocks, bounded
regrowth, deterministic contention and no remote gathering/depositing.

## Cycle 12 — Implement recipes, tools and production chains

**Depends on:** cycle 11.

Execute data-defined recipes with resource/item inputs, output quantities,
workplace/tool requirements, skill effects and production duration. Reserve
inputs; specify atomic completion or explicitly accounted staged consumption.
Credit actual crafted items. Support transfer, wear and repair if required by
scenario semantics. Establish tested food, pottery and copper/tin/fuel-to-tool
chains before expanding the full occupation set.

**BRONZE_SIM boundary:** recipe/tool logic lives in `bronze/`. Any influence on
native carried objects or action effectiveness is a guarded adapter conversion;
Bronze inventory units must not be confused with native body-slot inventory.

**Exit evidence:** missing inputs block production, cancelled work releases
reservations appropriately, and no item appears without accounted inputs.

## Cycle 13 — Complete household, communal storage and consumption rules

**Depends on:** cycles 07, 11 and 12.

Define ownership and access rules for individual, household (if the scenario
uses it) and communal stores. Implement fetching, depositing, ration policies,
food processing and consumption. Translate nutrition into native energy/needs
without also running duplicate Bronze hunger reduction. Rest uses the native
wakefulness/recovery model. Handle spoilage, waste and shortages only through
explicit scenario rules and accounting.

**BRONZE_SIM boundary:** ownership/access policy is Bronze-specific. Food/rest
integration points use `#ifdef BRONZE_SIM`; historical feeding and sleeping remain
unchanged in builds without it. Arbitration prevents double consumption.

**Exit evidence:** stores balance, inaccessible food cannot be consumed remotely,
shortages affect real beings, and accounting distinguishes transfers from sinks.

## Cycle 14 — Add trade, services and economic social memory

**Depends on:** cycles 08, 10, 12 and 13.

Implement exchange quantities/prices from scenario policy, stock checks,
recipient/provider eligibility and place requirements. Make exchanges atomic or
explicitly resumable. Services require a real provider, recipient and delivered
outcome; deferred service delivers zero. Connect completed work, exchanges and
services to appropriate native episodic/social events. Bound memory effects so
repeated work does not flood episodic storage. Keep diagnostic economic events
separate from subjective agent memories.

**BRONZE_SIM boundary:** reuse compatible profession interaction helpers through
guarded calls; add any Bronze-only native event handling under `BRONZE_SIM`.
Do not change historical event meanings or serialized enum values.

**Exit evidence:** shortages cannot mint goods; both parties' stores balance;
completed interactions identify the right beings and influence their histories.

## Cycle 15 — Add apprenticeship and lifetime profession transitions

**Depends on:** cycles 08, 09 and 14.

Implement mentor selection, supervised practice, skill gains and occupational
qualification. Define transitions for maturation, reassignment, disability or
work incapacity, missing workplaces and death. Separate learned skills from
genetic traits. Preserve or transfer belongings and release work/mentor claims
according to an explicit death/ownership policy. Reuse native births, aging,
health and death rather than simulating a second demographic system.

**BRONZE_SIM boundary:** training and profession transitions remain in sidecars;
all lifecycle and native memory integration hooks use `#ifdef BRONZE_SIM`.
Without it, native birth/death/aging behavior remains the original code path.

**Exit evidence:** training involves eligible co-present actors, skills change
within bounds, and death/reassignment leaves no dangling links or reservations.

## Cycle 16 — Save and resume the combined native and Bronze world

**Depends on:** cycles 04–15.

Specify a versioned Bronze save package containing the native world snapshot,
scenario identifier/content hash, stable actor/place mappings and ID allocator,
skills, inventories, tasks/routes, reservations, settlements, patch depletion,
policy state, clock relationship and RNG state. Use explicit portable fields,
length limits and validation, not raw C struct dumps. Pair both layers with a
shared generation/checksum; stage writes so a failed save cannot silently pair
unrelated native and economic states. Restore at a safe paused boundary.

Define native-only import as a fresh Bronze initialization, visibly distinct
from a full resume. Reject incompatible scenarios or offer an explicit supported
migration; never silently attach an old sidecar to the wrong population.

**BRONZE_SIM boundary:** historical native save bytes and file workflows remain
unchanged. Combined save/open routes are `BRONZE_SIM`-only extensions in shared
code or Bronze-owned UI. Remove the prototype's unconditional reset on a valid
full Bronze resume, while keeping reset for New/native-only imports.

**Exit evidence:** uninterrupted and save/resume runs produce matching state and
event traces; truncated, mismatched and unsupported saves fail safely.

## Cycle 17 — Complete inspection, uniform graphics and authoring feedback

**Depends on:** cycles 06–16; use read-only diagnostics in earlier cycles.

Present selected being's vocation, skills, home, workplace, cargo, task/progress,
needs, mentor and last economic outcome. Add site/store inspection and clear
failure reasons. Render fields and woodland as coherent areas; use a consistent
legend and distinct, scalable township/mine symbols in both map resolutions.
Keep overlays aligned with native selection, centering, terrain projection and
input coordinates. Expose resource stocks/depletion without altering native
terrain data. Provide accessible text equivalents for icons and counters.

Show the loaded scenario, compile errors and unsupported capability messages.
Preserve existing weather, brains, braincode, social views, menus and controls.
Any genuine inherited Catalyst parity gap should be recorded and resolved with
a clear compatibility path, not hidden by a claim of complete sim-mac parity.

**BRONZE_SIM boundary:** Bronze panels, legends and inspectors use
`#ifdef BRONZE_SIM` in C and `#if BRONZE_SIM` in shared Swift. Original ApeSDK
retains its original appearance and command behavior.

**Exit evidence:** a user can trace a selected native being to its profession,
visible workplace and actual inventory; overlays survive resize, movement,
new/load and multi-window use without stale pixels or mixed identities.

## Cycle 18 — Enable the full scenario and qualify the release

**Depends on:** cycles 01–17.

Use the capability table to finish every required behavior in the 63-vocation
`bronze_age_full.bronze`. Content names do not guarantee historical fidelity:
document model assumptions and simplify/reject unsupported claims explicitly.
Remove development-only fallbacks and make the full scenario the packaged
Bronze default only when it runs through the same proven executor as the small
scenarios. Keep all three content files as regression fixtures.

Run multi-seed, long-duration, scarcity, contention, population-turnover and
save/resume scenarios. Verify bounded work/memory, route and observer costs,
resource accounting, valid profession assignments and reproducibility. Build
Debug/Release, exercise Xcode LLDB breakpoints in shared C and Bronze code,
verify packaged resources, and perform visible View/Terrain/Control checks.

**BRONZE_SIM boundary:** validate the build matrix with and without
`BRONZE_SIM`. Historical sim-mac and the canonical native suite must retain their
baseline behavior. Confirm `HIDE_THUNDER` suppresses flashes only where defined,
with ordinary weather still active and default lightning tests still passing.

**Exit evidence:** recorded acceptance results, supported/unsupported feature
list, user run/debug/save instructions and a reproducible release procedure.
Signed distribution is a separate packaging step requiring the appropriate
team credentials; unsigned build success is not distribution validation.

## Validation discipline throughout the 18 cycles

- Run the narrow behavioral checks for the cycle first, then the required
  native/Bronze regression checks affected by it. Avoid tests that merely
  duplicate implementation details.
- Exercise both `BRONZE_SIM` and original builds whenever shared C changes.
  Absence of `BRONZE_SIM` must not require Bronze symbols, content or sidecars.
- Keep deterministic inputs, scenario fingerprints and canonical event formats.
  Log requested and completed amounts separately, including deferred outcomes.
- Validate real native movement, actual stock changes and actual item outputs;
  an emitted completion event is not sufficient evidence.
- Use products under `/private/tmp` for development verification. Do not replace
  an installed historical app as part of routine validation.
- Update this plan after each cycle with completed scope, evidence, limitations
  and any justified dependency changes. Do not mark a cycle complete because
  it compiles or its UI displays a placeholder.

## Recorded prototype validation (before these cycles)

These results describe the initial hybrid, not completion of the full plan.

- PASS: bronze-mac Debug and Release, Mac Catalyst arm64, signing disabled.
  Initial products were under `/private/tmp/bronze-mac-derived` and
  `/private/tmp/bronze-mac-release`; HIDE_THUNDER build checks used
  `/private/tmp/bronze-hide-thunder-Debug` and
  `/private/tmp/bronze-hide-thunder-Release`.
- PASS: historical `toolchains/sim-mac` Debug macOS, signing disabled, with
  `/private/tmp/bronze-historical-sim-mac` derived data.
- PASS: canonical `bash test.sh`, existing Bronze adapter harness and focused
  hybrid tests for placement, bounded production, cadence, reset and HUD bounds.
- PASS: visible app launch with Control, Terrain and View, native terrain,
  agents, weather, brain graphics, Bronze point-site markers and corrected HUD.
- PASS: `HIDE_THUNDER` build checks and forced-mask query/render tests; the
  default sim suite continues to test enabled full-alpha lightning.
- Existing compiler/SDK warnings remain. Full occupation integration, terrain
  land-use areas, full-scenario graphical execution, economic persistence and
  signed distribution are not implemented or validated by these results.

## Local build signing correction (2026-11-02)

The bronze-mac target now disables signing in both Debug and Release with
`CODE_SIGNING_ALLOWED=NO`, `CODE_SIGNING_REQUIRED=NO`, manual signing style
and an empty signing identity. The inherited Apple Development identity was
removed. Local Xcode builds must not require a development team or a command-line
signing override. Distribution signing remains an explicit future configuration.

## Cycle 01 completion record (2026-11-02)

- Completed audit of all current shared-core Bronze hooks, independent
  HIDE_THUNDER guards, both adapters and both DSL/runtime paths. No production
  engine behavior, content or historical save layout was changed in this cycle.
- Documented state ownership, units, time/coordinate conversions, actual hosts,
  identity/compaction risks and the per-world registry investigation for cycle 04.
- Inventoried all three content files through both real parsers, including
  operations, arguments, rules, resources, recipes, policies and parameters.
  The newer compiler rejects the full scenario; accepted explicit deposit/eat
  statements lack standalone execution handlers; some runner parameter names
  do not match parsed names. These are recorded gaps for subsequent cycles,
  not claims of full support or reasons to silently weaken the tests.
- Added `make -C bronze contract-test` and a gcc/clang CI gate. It checks
  definitions/preprocessor/link boundaries, reviewed content inventory,
  repeatability and pinned observable snapshots. It isolates HIDE_THUNDER with
  a native-only control; the original engine links without Bronze sources.
- PASS: `bash test.sh` with default native stages, existing Bronze scenario and
  adapter tests, and BRONZE_SIM/HIDE_THUNDER hybrid checks.
- PASS: fixed seeds 42/1337 at cycles 1/60/120, repeated fresh-process native,
  native-hide and Bronze probes. Native sampled state and being layout match
  across variants; separate lightning and Bronze state traces are captured.
- PASS: bronze-mac Debug and Release Mac Catalyst builds, no signing override;
  original sim-mac Debug and Release builds without BRONZE_SIM, signing disabled
  only for local build validation. Compiler/SDK warnings remain.
- This baseline is the current checkout, not a recovered historical release.
  It does not prove full memory/social/save-format or lifetime equivalence.
  No new GUI acceptance claim is made for this documentation/test-only cycle.

## Cycle 02 completion record (2026-11-02)

- Removed duplicated parser/AST/kind/vector/utility sources and headers from
  runtime/. `dsl/` is the sole compiler and shared domain definition owner.
- Preserved strict compilation and standalone behavior through explicit
  consumer profiles in one parser. No silent strict-to-legacy fallback. The
  standalone runner and compiled executor reject the wrong profile.
- Merged basic native access and scenario views into the adapter/ implementation.
  Root headers only forward to canonical headers; duplicate root C files are
  removed. The shared neutral scenario contract remains source-only.
- Migrated CLI/harness build lists and added canonical compiler/adapter sources
  to the Catalyst target exactly once. Scenario loading remains cycle 03.
- Added profile/AST/error-path tests, wrong-profile native dispatch checks,
  source-ownership audit and pre-migration CLI output snapshots for all three
  scenarios. Existing cycle 01 inventories and native/Bronze baselines passed
  unchanged; no re-recording was used to mask differences.
- PASS: full default suite, compatibility contract, compiler ASan/UBSan tests,
  Bronze Catalyst Debug/Release without signing overrides, original sim-mac
  Debug/Release without BRONZE_SIM. No shared engine C changes were needed.
- See OWNERSHIP.md and contract/RESULTS-CYCLE02.md for exact ownership,
  compatibility policy, evidence and remaining limitations.

## Cycle 03 completion record (2026-11-02)

- Bundled all three .bronze resources. The scene-owned Swift controller loads
  first_slice through Bundle.main, not a repository-relative working directory.
- Added Bronze Scenario selection/reset commands, an active-file status panel
  and scrollable filename/source-line diagnostics. Successful commits reset with
  the compiled seed; rejected loads leave the active config/world unchanged.
- Added transactional strict config ownership in scenario/, bounds/units and
  host-capability admission, per-parser diagnostic streams, numeric conversion
  checks, nesting limits and partial AST cleanup. No legacy fallback is used.
- UI and shared-shell changes are #if BRONZE_SIM; both C and Swift definitions
  are target-owned. The original shell's startup/New/file behavior is unchanged
  when the symbol is absent. No shared engine C hooks were needed.
- PASS: full default suite; unchanged contract/native/CLI snapshots; loader
  ASan/UBSan tests; real Debug/Release bundle/controller tests outside the repo
  cwd; Bronze Debug/Release without signing overrides; original native sim-mac
  Debug/Release and original Catalyst Debug builds.
- Added Mac CI packaged-resource/controller checks alongside portable contract
  tests. UI layout still requires visual acceptance; automated controller tests
  do not claim that a live menu/sheet was manually inspected.
- The full scenario remains strictly rejected with diagnostics. Config loading
  applies only the seed today; professions, inventories, places and task-driven
  native behavior remain cycles 04 onward. See scenario/README.md and
  contract/RESULTS-CYCLE03.md for scope, commands and results.

## Cycle 04 completion record (2026-11-02)

- Added actors/ with a world/session-owned registry, monotonic scoped 64-bit
  actor IDs and the sidecar schema for vocation, skills, home/workplace, cargo,
  task/progress, reservations, assignment history and mentoring. Defaults are
  unassigned/empty; these are not implementations of later profession behavior.
- Added guarded native bootstrap/import, birth, death and compaction hooks.
  Existing native callbacks remain in place. No ID derives from a pointer,
  array slot, display name, birth date or genetics; no native being/save field
  was added. Claim cleanup and cross-actor mentor/apprentice cleanup are tested.
- Added per-shared-core-context registry ownership and restore/rebind/teardown.
  Inactive worlds cannot resolve their IDs to another world's native array.
  Native imports allocate fresh IDs; malformed transfer invalidates records.
- Added a checked registry-backed adapter factory and a guarded selected-ID
  bridge/status line. Neutral v1's narrower IDs are never silently truncated.
- PASS: actual native birth/death/compaction/reorder and two-session tests,
  full default suite, unchanged contract/CLI/native snapshots, Bronze and
  original Debug/Release app builds, original Catalyst build and packaged
  scenario/controller tests.
- PASS: ASan across actor integration and UBSan on new registry/harness code.
  Full-engine UBSan independently fails in original entity/being.c:1599 without
  BRONZE_SIM. That native signed-shift issue is recorded, not silently fixed or
  claimed clean. See contract/RESULTS-CYCLE04.md for the instrumentation split.
- Skills/jobs/inventories are ready to populate in their owning cycles; the
  allocator and actor records are not yet persisted across process/native-only
  loads. Full paired save/resume remains cycle 16. Prototype global economics
  and visual acceptance remain separately documented limitations.

## Cycle 05 completion record (2026-11-03)

- Added work/ with registry-owned native clock observations, day/duration
  conversion, a native needs adapter, and explicit survival/sleep/obligation
  precedence. No parallel biological or time integrator was introduced.
- `#ifdef BRONZE_SIM` brackets work around native cycles and allows a safe
  pending stationary intention to hold locomotion after native awake decisions.
  Native feeding, reproduction, social actions, brains and metabolism continue;
  without `BRONZE_SIM` the original path is retained exactly.
- Sidecars record reasons, transitions, interruptions/resumptions, consumed
  units and duration remaining. At most one unit per actor/native tick is
  charged through native energy APIs; one-unit resumption/completion is covered.
- Hourly prototype gathering uses the same grant. Only four Bronze resource
  baseline rows change because moving/occupied beings no longer gather; native,
  lightning, CLI and AST golden values remain unchanged.
- Added real-engine tests for duration bounds, day rollover, discontinuities,
  starvation/sleep/social interruption, actual feeding/resumption, locomotion
  ownership, pause and differing real map/terrain draw schedules. Existing
  identity/lifecycle/multi-session checks remain enabled with allocator poisoning.
- Professions, DSL instruction scheduling, geographic navigation, actor cargo
  and accounted food effects remain later work. Clock/sidecar persistence is
  still paired-save cycle 16; prototype site/economy globals remain a limitation.
