# Cycle 01: compatibility, scenario and testing contract

Established 2026-11-02. **The source-ownership tables below describe cycle 01's
pre-consolidation audit.** Cycle 02 supersedes that layout; see
[canonical ownership and profiles](../OWNERSHIP.md). The snapshots remain
unchanged, and the test runner now checks canonical ownership and both explicit
profiles, plus pre-consolidation CLI output and profile/AST tests.

This is an audit of the current prototype, not an
assertion that the graphical app executes the full Bronze scenario. Planned
behavior is distinguished from implemented behavior below. No production
simulation behavior is changed by this cycle.

## Execution paths and ownership of source

| Path | Sources / caller | Current responsibility |
| --- | --- | --- |
| Original ApeSDK | `toolkit`, `script`, `sim`, `entity`, `universe`, `render`, `gui`; historical toolchains | Native population, biology, drives, movement, memory, relationships, terrain, weather, time and graphics. Builds and links without Bronze. |
| Bronze graphical hybrid | `hybrid/bronze_hybrid.c`, compiled by `bronze-mac` | Ten site markers, bounded proximity-based resource sampling, aggregate counters and HUD. Does not parse content or assign vocations. |
| Standalone Bronze | `runtime/brz_*.c`, `runtime/bronze_main.c`, `make bronze-sim` | Own dense world, `BrzAgent` population, occupations, inventories, daily rules and task interpreter. Uses string operation dispatch. Not the native being population. |
| Compiled pilot | `dsl/`, root `bronze_apesdk_{adapter,compiled}.*`, `test.sh` / land harness | Newer compiled DSL definitions, opaque read-only native views, manually invoked top-level tasks and a six-resource economic sidecar. Not linked into the graphical target. |
| Basic adapter | `adapter/bronze_apesdk_adapter.*`, `make adapter-check` | Native position getter/setter, energy/fatigue getters and terrain/clock callbacks. Currently compile-checked, not the full scheduler. The setter changes position directly; it is not navigation. |
| Shared service pilot | `scenario-runtime/` | IDs, read-only views, events and a small travel/service fixture. Fixture equivalence is not real native routing or a production service system. |

`runtime/` and `dsl/` are not interchangeable: `OpDef`, conditions and config
layouts differ. The compiled copy also has version/profile/place/role/need/
service/disruption declarations not used by these three content files. Do not
mix object files or headers from the two copies. Cycle 02 will choose canonical
ownership; this cycle preserves both and inventories their actual results.

## Shared-core boundary audit

| Core file / function | Conditional hook | Result with BRONZE_SIM undefined |
| --- | --- | --- |
| `universe/sim.c` include, `sim_cycle` setup and end | `#ifdef BRONZE_SIM`: include hybrid header, reset after population setup, sample work before `sim_time` | No Bronze calls or headers; original native loop remains. |
| `gui/draw.c` include, `draw_bronze_pixel`, `draw_apes` | `#ifdef BRONZE_SIM`: draw site symbols before agents; native high-resolution undraw bookkeeping | Original map/terrain drawing remains. |
| `gui/shared.c` include, `shared_draw_into_buffer` | `#ifdef BRONZE_SIM`: overlay HUD after the shared buffer conversion | Original image output remains. |
| `sim/tile.c`, `tile_lightning` | Independent `#ifndef HIDE_THUNDER`, with a no-op implementation otherwise | Normal lightning generation when HIDE_THUNDER is also undefined. |
| `sim/land.c`, `weather_lightning` | Independent `#ifdef HIDE_THUNDER`: return zero | Original lightning query when undefined. |
| `gui/draw.c`, bolt helper/call and flash blending | Independent `#ifndef HIDE_THUNDER` | Original lightning visuals when undefined. |

There are no current Bronze additions to `simulated_being`, native transfer
layouts, native file format, birth/death callbacks or native action arbitration.
The preprocessor audit checks actual output for all three Bronze-hook source
files, and native test binaries link without any `bronze/*.c` implementation.

Both bronze-mac target configurations define `BRONZE_SIM=1` and
`HIDE_THUNDER=1`, and disable signing. Other checked-in Xcode projects define
neither. The symbols are independent: `BRONZE_SIM` alone does not imply
`HIDE_THUNDER`, and HIDE_THUNDER alone must not enable Bronze state. No Swift
BRONZE_SIM condition is required yet because no shared Swift branch was added.

## Scenario inventory and capability matrix

`content_inventory.json` is generated through each real parser, rather than
inferred from comments. It records the source SHA-256, acceptance/rejection and
diagnostics, counts, vocation names, every distinct parsed construct/argument/
condition/parameter and a hash of the complete ordered AST inventory. Repeated
constructs remain represented in the AST hash and counts. Rejected files may
have populated ASTs; their inventory is evidence of syntax/data only, not a
usable compiled program.

| Scenario | Vocations / tasks / rules | Operations | Resource kinds / item kinds / recipes | Runtime parser | Compiled parser |
| --- | --- | --- | --- | --- | --- |
| `first_slice.bronze` | 3 / 3 / 3 | 8 | 7 / 2 / 2 | Accepted | Accepted |
| `vertical_slice.bronze` | 3 / 3 / 3 | 14 | 6 / 1 / 1 | Accepted | Accepted |
| `bronze_age_full.bronze` | 63 / 126 / 126 | 159 | 14 / 3 / 3 | Accepted | Accepted |

None contains nested task `when` or `chance` blocks. Their absence is a
coverage gap, not evidence that either interpreter handles every nesting case.
All declared and used verbs, recipe inputs, habitats and scalar values are
listed in the machine-readable inventory; this table describes their semantics.

Legend: implemented = observable behavior exists in the named path; partial =
behavior exists with material limitations; stub = completion is reported without
the promised effect; rejected = compiler refuses it; missing = no binding.

| Construct / behavior | Standalone runtime | Compiled native pilot | Graphical hybrid |
| --- | --- | --- | --- |
| `kinds`, `resources`, capacity/renew/nutrition/market target | Parsed; dense resource arrays and simplified world/market behavior | Definitions compiled, but adapter inventory is fixed to grain/fish/wood/copper/tin/charcoal | Missing content binding; five hard-coded site types |
| `world`, `agents`, `settlements`, `sim` | Parsed; typed seed/counts and selected parameter lookups | Parsed; caller creates native world, supplies actor ID and place | Missing content binding; native world creates population |
| `actions` declaration/arity | Validated declaration, string dispatch | Validated and compiled to action codes | Missing content binding |
| `vocation`, `task`, weighted `rule`, `do` | Executed on separate BrzAgents; rule may target a declared action | Task selected manually by caller; no rule selection or per-being vocation | Missing |
| Rule conditions `true`, hunger/fatigue comparisons, `and prob` | Own normalized needs; probability eligibility is rerolled in two weight passes (defect/risk) | Compiled conditions exist; pilot does not evaluate rules | Missing; only checks native wakefulness |
| Nested `when` / chance | Interpreter paths exist; no coverage from these three fixtures | Compiler understands nested AST; pilot rejects any non-top-level operation | Missing |
| `move_to`, `roam`, `wander` | Partial: tag search, direct cell steps, no obstacle routing; movement may also occur in the end-of-day target step | **Stub:** reports move completed, does not change native position | Native movement only; no economic destination |
| `gather` | Partial: debits stock at matching tag, credits agent, otherwise sets target | **Stub/partial:** credits sidecar without world depletion, terrain or proximity check | Partial: awake beings within map radius 10 debit stock and increment aggregate counter hourly |
| `craft`, `recipes`, `input`, `output` | Partial: recipe inputs consumed and outputs credited; fallback can mint declared items without a recipe; event amount need not match actual output | Partial: fixed copper/tin/charcoal consumption; no crafted item output | Partial: one aggregate tool recipe, independent of actor/workplace/content |
| `trade` | Partial: settlement prices/stocks and proximity; shortages can change effective exchange ratio; validate accounting before reuse | Rejected by executor (no action mapping) | Missing |
| Explicit `deposit`, `eat` in vertical slice | **Missing dispatch despite accepted parsing**; automatic food/deposit policy is a different path | Transfers/debits sidecar stocks; no native nutrition or arrival requirement | Missing actor inventory and consumption |
| `rest` | Own normalized fatigue adjustment | Own sidecar fatigue adjustment, not native recovery | Native sleep only, no Bronze rest action |
| `settlement_policy` | Automatic food/deposit/rest behavior; food_fallback is not fully honored as in newer standalone BronzeSim | Not applied by pilot | Missing content binding |
| Snapshot/map/report parameters | Partial; see parameter mismatches below | Raw struct sidecar snapshot/event round trips, not combined native save/resume | Reset on new/load; no economic persistence |
| Professions, skills, services, apprenticeship, social/economic memory | Named occupations exist; no full real-being integration | No scheduler binding; optional entity profession helpers are separate | Missing |

### Historical mismatches recorded in cycles 01–03

- The original full scenario was rejected for numeric `roam` without a
  terrain name, `move_to beach/hill/river`, and argument-free `trade`. Cycle 18
  replaced those forms with supported graphical tasks and refreshed the
  reviewed inventory.
- `world { sim_map_w/h ... }` becomes `world_sim_map_w/h`, while the standalone
  runner reads `sim_map_w/h`; the full scenario therefore uses the runner's
  default 80×40, not the apparent 160×80 in the source.
- `sim { snapshot_every/map_every ... }` becomes `sim_snapshot_every/map_every`,
  but the runner reads unprefixed names. `cache_max` is stored but there is no
  chunk cache. These values being parsed does not mean they are effective.
- The surface condition grammar is wider than the compiled evaluator's bounded
  condition representation. Validate diagnostics and rule execution separately.
- Unknown standalone movement/habitat tags can fall back to forest, including
  unsupported terrain names; this is not real beach/hill/river/fire semantics.
- `first_slice` and `vertical_slice` have no explicit population/map/time
  settings; defaults matter. Neither is a miniature full-society validation.

## State, units and authority contract

| State / units | Authority now | Full integration rule |
| --- | --- | --- |
| Population and biological lifetime | ApeSDK for native/hybrid; separate BrzAgents for CLI | Real ApeSDK beings only in graphical app; sidecars are not a parallel population. |
| Position | Native ape-space integer coordinates; one map cell is 64 ape-space units | Read native position; convert with APESPACE_TO_MAPSPACE / MAPSPACE_TO_APESPACE. Do not interpret arbitrary scenario grid coordinates as ape space. |
| World | Normal MAP_BITS=9: 512×512 map; HI_RES_MAP_BITS=12: 4096×4096 coordinate grid, native rendering storage conventions | Native wrapping and terrain are authoritative. No physical 3-foot/30-mile promise is established by this code. |
| Time | Native land date + minute; one sim cycle advances one minute; 1,440 per day | Absolute tick = date×1440+time; sample tasks by native tick, not frames. Account for reset/load and long runs. |
| Prototype work | Once at native minute divisible by 60, duplicate-tick guard | Replace with explicit task durations in cycle 05/09; never step once per visible window. |
| Standalone work | One rule/task sequence per own day; 180 days in full content, 365 default | This daily loop is not automatically equivalent to 1,440 native action cycles. |
| Terrain/site stock | Native heights; prototype stock=200, harvest=1, renewable sites gain 5 per observed day | Bronze patch quantities are scenario-defined economic units, not height or energy. Prototype distances currently do not wrap across map boundaries. |
| Energy, hunger/fatigue, wakefulness | Native being state in hybrid; separate doubles in standalone/pilot | Native APIs own needs. Translate once; do not retain independently drifting hunger/fatigue alongside them. |
| Vocation, skill, cargo, job progress, home/workplace | Missing per-being mapping in prototype | Bronze world-owned actor sidecar with stable actor/place IDs. |
| Settlement wealth | Prototype aggregate counters | Per-place/actor accounted stocks; derive totals, not a second authoritative total. |
| Native memories, social graph, genetics and RNG | ApeSDK | Preserve native ownership; future Bronze events apply bounded effects through guarded hooks. |
| Scenario RNG | Standalone BrzRng; hybrid currently uses no economic randomness | A separately owned, persisted deterministic Bronze stream; specify call order. |
| Persistence | Native files plus unrelated pilot raw snapshots | Preserve native format; cycle 16 adds validated paired native/Bronze save state. |

## Identity decision for cycle 04

Existing first/family names and date of birth are useful descriptive attributes,
not a proven collision-free lifetime ID. Pointers and array indexes are not
stable: `universe/loop.c:being_remove_loop2` compacts live beings using
`memory_copy`. Birth/death callbacks currently belong to native debug behavior.

Investigate an external registry with monotonically allocated actor IDs and
explicit guarded hooks for creation, death and relocation during compaction.
Chain/preserve existing callbacks rather than replacing them. The registry must
be per native world/session, release claims on death, allocate IDs deterministically
and persist its allocator on full saves. New/native-only imports must explicitly
reinitialize mappings. Do not implement or claim this identity solution in
cycle 01. The current hybrid's file-static economy is also not isolated per
independent shared session; the existing shell shares one scene model across
its windows. Multi-session support needs lifecycle ownership in later cycles.

## Testing contract and commands

From the repository root:

```sh
bash test.sh
make -C bronze contract-test
```

`bash test.sh` remains the existing required native suite (toolkit, JSON/file,
script, sim, render, immune, entity, universe), followed by Bronze scenario,
adapter and hybrid tests. It exercises the normal engine with BRONZE_SIM and
HIDE_THUNDER undefined in the native stages. The default sim tests explicitly
retain full-alpha lightning. The hybrid test enables both symbols and checks
forced-mask suppression, bounded economics, tick cadence, reset and drawing.

The new `contract-test` is an additional required regression gate for future
Bronze/shared-core changes. CI runs it with both gcc and clang. It:

1. Audits both Bronze Xcode target configurations and verifies historical Xcode
   projects define neither flag; verifies local Bronze signing remains disabled.
2. Preprocesses every shared source containing Bronze hooks with and without
   BRONZE_SIM, checking that native output contains no Bronze symbols.
3. Parses all three files through both parser copies, comparing a reviewed
   inventory covering acceptance, diagnostics, and parsed content.
4. Builds independent `native`, `native_hide` and `bronze` probes from real C
   sources. Native probes do not link Bronze sources; `nm` checks for leaked
   Bronze symbols. The middle variant isolates HIDE_THUNDER from BRONZE_SIM.
5. Runs seeds 42 and 1337 in fresh processes twice per variant; records cycles
   1, 60 and 120. Captures population, native time, being struct size, scalar
   digests of every being's position/energy/state/facing/fatigue, terrain,
   pressure and a separate lightning mask. Bronze adds site/stock/economy state.
6. Requires repeatability, checks sampled native state equality across the
   variants, and checks pinned traces in `baselines.json` on a recorded host.
   Hash encoding uses explicit integer bytes, not pointers or struct padding.

The native equality check is appropriate for the CURRENT additive prototype;
future deliberate profession/food/movement effects will change Bronze native
outcomes. Keep original native snapshots pinned, retain the native-hide control,
and replace that particular cross-mode equality with documented expected effects
when implementing those cycles. Never silently refresh all snapshots to make a
failure pass. Preserve a no-economic-actions scenario for isolation testing.

Baseline storage is keyed by OS/architecture. Unrecorded hosts still run
repeatability and cross-mode checks, but print that no host-specific golden
trace was checked. They must not silently create new snapshots. After reviewing
an intentional change, explicitly record with:

```sh
python3 bronze/contract/check.py --record
```

This records the current source, not an independently recovered pre-hybrid
historical release. Baselines detect future regressions; they do not prove all
original behavior was preserved before cycle 01. They sample 120 minutes and
selected native fields, not complete brain/social/memory/save state or lifetime
behavior. Existing unit tests add coverage; long-run, persistence, independent
sessions, real routing and GUI acceptance belong to later cycles. Exact native
save-layout compatibility is not proven by a matching struct size alone.

Build the actual app targets for shared-core changes as well:

```sh
xcodebuild -project toolchains/bronze-mac/bronze-mac.xcodeproj \
  -scheme bronze-mac -configuration Debug \
  -destination 'platform=macOS,variant=Mac Catalyst' \
  -derivedDataPath /private/tmp/bronze-cycle01-bronze build
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj \
  -scheme sim-mac -configuration Debug -destination 'platform=macOS' \
  -derivedDataPath /private/tmp/bronze-cycle01-native CODE_SIGNING_ALLOWED=NO build
```

Repeat both for Release. Only the historical project needs the signing override;
Bronze project settings own its no-signing contract. Build success validates
compilation/linking, not visual acceptance or distribution signing. Run GUI smoke
checks when graphics/input changes; this documentation/test-only cycle does not
change pixels or require a fresh visual acceptance claim.


Cycle 04 extends the boundary audit to universe/loop.c, universe/command.c and
shared.h. Bronze fixed-seed probes also verify that every sampled native being
has a resolvable registry identity, without changing the recorded scalar traces.
The default suite includes actors/test.sh for real lifecycle, file-import and
shared-session binding tests. See ../actors/README.md for the schema and the
explicit full-native UBSan limitation.

Cycle 05 audits unique work-module ownership in the Bronze Xcode target and
links that module only into Bronze probes. The actor integration harness now
also runs native-clock/needs/arbitration and real pause/redraw schedule tests.
Only four Bronze resource rows were updated for movement/needs deferral;
original native, CLI and AST baselines remain unchanged. See
[cycle 05 evidence](RESULTS-CYCLE05.md) and [work policy](../work/README.md).
