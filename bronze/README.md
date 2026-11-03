# Bronze scenario package

This optional package is ApeSDK's landing zone for the BronzeSim economic and
cultural scenario. It depends on ApeSDK; ApeSDK core has no dependency on it.

It contains a canonical compiler/AST in `dsl/`, the standalone interpreter in
`runtime/`, and the single native binding implementation in `adapter/`.
See [source ownership and compatibility profiles](OWNERSHIP.md).
The root adapter headers are compatibility includes; implementation lives only
in `adapter/`. The standalone runtime no longer owns duplicate compiler sources.

The standalone CLI explicitly uses the legacy-runtime parse profile, preserving
its previous scenario behavior. Native compiled-task callers use strict parsing;
there is no automatic fallback when strict compilation fails. The full scenario
still needs semantic reconciliation before strict graphical execution is possible.

`make test` runs migrated Bronze scenarios and the native/hybrid harnesses.
Pilot economic snapshots are separate from ApeSDK native saves; the pilot's
reported movement and craft events do not establish complete travel/item semantics.
One native day is 1,440 land-time cycles. Economic inventories and future
occupations remain Bronze sidecar state, preserving native being/save layouts.

## BronzeSim Mac Catalyst app

Open `../toolchains/bronze-mac/bronze-mac.xcodeproj`, select the shared
`bronze-mac` scheme and **My Mac (Mac Catalyst)**, then Run or Debug. The product
is `BronzeSim.app` with its own bundle identifier. Debug and Release disable code signing by default; no development team is
required for local builds. Distribution signing must be configured separately.

The target shares ApeSDK's Catalyst View, Terrain and Control windows and all
native rendering/being behavior. It defines `BRONZE_SIM=1` and compiles the
Bronze-specific layer from `hybrid/`. Both target configurations also define
`HIDE_THUNDER=1`, suppressing lightning generation, flashes and bolt icons while
retaining clouds, rain and other weather. Historical targets leave it undefined. Townships, fields, copper/tin mines and
woodland appear on the native maps; the overview adds an economic status panel.
Townships use white roof outlines, mines dark boxes with white ore marks,
fields blue rows, and woodland blue tree outlines.

On startup the app loads and strictly validates bundled `first_slice.bronze`.
Use the **Bronze Scenario** menu to select a bundled file; successful selection
resets the native world using that file's seed. The overview identifies the
active file and offers **Details**. Failed loads show filename/line diagnostics
and retain the active scenario and world. `vertical_slice.bronze` also validates;
`bronze_age_full.bronze` is bundled but still fails strict compilation.

The graphical host applies the scenario seed and terrain thresholds. Eligible
stationary actors can gather on terrain-derived field, woodland and ore patches.
Townships own reachable production sites and their stores. Actors receive homes,
scenario-defined vocations and matching workplaces as capacity allows. Their
skills grow through successful work. The DSL task executor and paired economic
saves are later cycles. See the [landscape rules](hybrid/README.md),
[occupation policy](actors/PROFESSIONS.md) and [implementation plan](PLAN.md).

Cycle 05 adds [native clock and work arbitration](work/README.md): stationary
intentions yield to survival, sleep and social obligations; task progress and
hourly prototype gathering share one per-actor native-tick budget. Drawing and
pausing cannot advance work. The app still does not assign professions or
dispatch the full scenario's tasks.

Command-line validation (signing is disabled in the project settings):

```sh
xcodebuild -project ../toolchains/bronze-mac/bronze-mac.xcodeproj \
  -scheme bronze-mac -configuration Debug \
  -destination 'platform=macOS,variant=Mac Catalyst' \
  -derivedDataPath /private/tmp/bronze-mac-derived build
```

`make test` includes `hybrid/test.sh`, which links the hybrid against the real
ApeSDK engine and checks deterministic placement, symbol bounds, bounded
production, economic cadence, reset behavior, and the HUD buffer boundary.

## Compatibility and regression contract

Cycle 01's [contract](contract/README.md) distinguishes the standalone runtime,
compiled adapter and graphical hybrid, and records their current capability
gaps. Run `make -C bronze contract-test` from the repository root in addition to
`bash test.sh` after Bronze/shared-core changes. The contract test checks native
build isolation, all three scenario inventories and fixed-seed native/Bronze
baselines. CI runs it under gcc and clang. Baseline updates require explicit
review and `python3 bronze/contract/check.py --record`; never refresh them merely
to clear a regression.

The scenario loader tests are included in `make -C bronze contract-test`.
On macOS, verify an actual built app's resources and the same Swift controller
used by the UI, from an independent temporary working directory:

```sh
bash bronze/scenario/test_bundle.sh \
  /private/tmp/bronze-mac-derived/Build/Products/Debug-maccatalyst/BronzeSim.app
```

CI builds Bronze Debug/Release and runs this packaged-resource check in addition
to the original Catalyst tests. This does not replace visual UI acceptance.

## Native being identities

Cycle 04 adds a [per-world actor registry](actors/README.md). The overview shows
an actor ID for the selected real ApeSDK being. IDs and linked sidecar state
survive native births, deaths, population compaction, reordering and session
switches. Death/reset releases claim records and clears mentorship links.
The registry does not yet assign professions or replace prototype work with
per-being inventory/task execution; those remain later cycles.

`bash test.sh` now includes the actor lifecycle/session harness. Targeted memory
and undefined-behavior checks run with `SANITIZE=1 bash bronze/actors/test.sh`;
see the registry documentation for the existing native UBSan limitation.
