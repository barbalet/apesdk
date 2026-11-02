# Bronze scenario package

This optional package is ApeSDK's landing zone for the BronzeSim economic and
cultural scenario. It depends on ApeSDK; ApeSDK core has no dependency on it.

It contains the migrated runtime and content plus an adapter over ApeSDK land,
clock, and `simulated_being` identity/position. Economic inventory,
occupations, and settlement stores remain Bronze sidecar state, preserving
ApeSDK's core types and native save compatibility.

The initial deterministic farmer/fisher/smith slice executes `move`,
`gather`, `craft`, `deposit`, `eat`, and `rest` from compiled `.bronze`
content. One Bronze day is exactly 1,440 ApeSDK land-time cycles.

`make test` runs both migrated Bronze scenarios and the headless harness
linked against real ApeSDK sources. The harness compares event traces from two
fresh runs, and verifies versioned Bronze sidecar snapshot/event-log round
trips. These persistence files belong to this package; they do not alter
ApeSDK's native save format.

## BronzeSim Mac Catalyst app

Open `../toolchains/bronze-mac/bronze-mac.xcodeproj`, select the shared
`bronze-mac` scheme and **My Mac (Mac Catalyst)**, then Run or Debug. The product
is `BronzeSim.app` with its own bundle identifier. Select your development team
in Xcode when signing is needed.

The target shares ApeSDK's Catalyst View, Terrain and Control windows and all
native rendering/being behavior. It defines `BRONZE_SIM=1` and compiles the
Bronze-specific layer from `hybrid/`. Both target configurations also define
`HIDE_THUNDER=1`, suppressing lightning generation, flashes and bolt icons while
retaining clouds, rain and other weather. Historical targets leave it undefined. Townships, fields, copper/tin mines and
woodland appear on the native maps; the overview adds an economic status panel.
Townships use white roof outlines, mines dark boxes with white ore marks,
fields blue rows, and woodland blue tree outlines.

This first hybrid uses proximity-based resource work alongside native behavior;
it does not yet bind the full Bronze DSL occupation scheduler. Economic stores
are session state, reconstructed when creating/loading a native world. Native
save files do not persist them. See `PLAN.md` for the implementation boundary
and follow-up work.

Unsigned command-line validation:

```sh
xcodebuild -project ../toolchains/bronze-mac/bronze-mac.xcodeproj \
  -scheme bronze-mac -configuration Debug \
  -destination 'platform=macOS,variant=Mac Catalyst' \
  -derivedDataPath /private/tmp/bronze-mac-derived CODE_SIGNING_ALLOWED=NO build
```

`make test` includes `hybrid/test.sh`, which links the hybrid against the real
ApeSDK engine and checks deterministic placement, symbol bounds, bounded
production, economic cadence, reset behavior, and the HUD buffer boundary.
