# Cycle 18 release qualification

The Catalyst app starts with `bronze_age_full.bronze`. Both smaller slices
remain selectable and all three files pass strict graphical admission.

The 63 labels describe a roster of shared behaviors. Fields stock fish and
clay as proxies for coastal and river production. Forestry and mining roles
use the existing forest and ore patches. Craft workers use township stock.
The trader fetches wood and offers one wood for one grain. Roles without
specialized work use rest, and infant, child, and elderly labels have zero
occupation weight. Native age and population rules govern the simulation.
The legacy map dimensions, population, and settlement counts describe the
standalone profile. Named service profiles and disruptions remain unsupported.

## Supported graphical scope

- Native agents, map and weather with Bronze township, field, woodland and mine overlays.
- Native-clock vocation assignment, work, gathering, storage, recipes,
  consumption, rest and supervised practice where enabled by scenario policy.
- Atomic trade and paid delivery between eligible actors at township service
  entrances. Completed exchanges can add paired native memories, and the
  actor sidecar records the latest outcome and partner.
- Combined save and resume. Native-only import initializes a fresh Bronze economy.

## Local results

| Check | Result |
| --- | --- |
| Clang deterministic suite, including the native lightning mask | Passed |
| Native, thunder-hidden and Bronze deterministic contracts for seeds 42 and 1337 through tick 120 | Passed |
| Historical sim-mac unsigned Debug build | Passed |
| Bronze Mac Catalyst unsigned Debug and Release | Passed |
| Packaged resources and scenario controller in both Bronze bundles | Passed |

## Cycle 14 results

The hybrid harness verified balanced inventories on completed trades and paid
deliveries, stock and capacity rejection, zero delivery when the recipient is
absent, and native memories naming the correct counterpart. The deterministic
contracts and unsigned Bronze Mac Catalyst Debug build passed with these changes.

## Interactive Catalyst checks

- Observed the first slice render in View, Terrain and Control. View kept the
  map overlays and legend when switching to full screen. Next-being selection
  updated the actor inspector and recentered the overview.
- Loaded `vertical_slice.bronze` from the scenario menu and confirmed the
  active scenario label.
- Exported a combined save from the first slice and opened it with View,
  Terrain, and Control open. After selecting a different being, Open
  returned View to actor #1 with the saved energy of 3783 and fatigue of 13.
  Control rendered the native being and memories after the restore.
- Recentered Terrain on a being near a mine and township. The symbols
  followed the terrain projection through recentering and a switch from
  windowed to full-screen rendering. Control was minimized to expose
  Terrain during the close-up and remained open.
- LLDB stopped at both `shared_session_cycle` and `bronze_tasks_end` in the
  Debug app with the startup scenario.

## Full-scenario qualification

| Seed | Native minutes before checkpoint | Assigned actors | Work units |
| --- | ---: | ---: | ---: |
| 42 | 1440 | 6 | 359 |
| 1337 | 1440 | 24 | 2844 |

Both runs verified forced death cleanup, live bindings, stock bounds,
and matching native and Bronze state after 30 further ticks from the
checkpoint. The focused suites covered scarcity, reservation contention,
capacity, birth, and combined disk save and resume. Strict compilation
covered the whole roster. The live samples exercised the actors who
found reachable jobs.

Observed the full scenario in View, Terrain, and Control after selecting
it in the running Debug app. After a live switch to the vertical slice,
LLDB stopped in `bronze_tasks_end` and confirmed its `smith` vocation.
Signed distribution requires a separate signing step.

## Reproduce local qualification

From the repository root, run the deterministic suite and the three-mode
contract. These are bounded checks and require no scheduled CI job.

```sh
CC=clang bash test.sh
make -C bronze contract-test
bash bronze/hybrid/test.sh --qualify-full
```

Build and check both unsigned Catalyst bundles.

```sh
for config in Debug Release; do
  derived="/private/tmp/bronze-qualification-$config"
  xcodebuild -project toolchains/bronze-mac/bronze-mac.xcodeproj \
    -scheme bronze-mac -configuration "$config" \
    -destination 'platform=macOS,variant=Mac Catalyst' \
    -derivedDataPath "$derived" build || exit
  bash bronze/scenario/test_bundle.sh \
    "$derived/Build/Products/$config-maccatalyst/BronzeSim.app" || exit
done
```

For a local debug session, open the same project in Xcode, choose
`bronze-mac` and **My Mac (Mac Catalyst)**, and run Debug. Useful breakpoint
targets are `shared_session_cycle` and `bronze_tasks_end`. Use the Bronze
Scenario menu to switch between the two slices and the full scenario.
