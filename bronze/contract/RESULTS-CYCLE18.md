# Cycle 18 release qualification

The full scenario remains bundled as a regression fixture. The startup
scenario stays `first_slice.bronze`. The strict graphical loader also
admits `vertical_slice.bronze` and reports diagnostics when the full
scenario is selected, preserving the active scenario and world.

## Supported graphical scope

- Native agents, map and weather with Bronze township, field, woodland and mine overlays.
- Native-clock vocation assignment, work, gathering, storage, recipes,
  consumption, rest and supervised practice where enabled by scenario policy.
- Native-only save and import. Import initializes a fresh Bronze economy.

## Local results

| Check | Result |
| --- | --- |
| Clang deterministic suite, including the native lightning mask | Passed |
| Native, thunder-hidden and Bronze deterministic contracts for seeds 42 and 1337 through tick 120 | Passed |
| Historical sim-mac unsigned Debug build | Passed |
| Bronze Mac Catalyst unsigned Debug and Release | Passed |
| Packaged resources and scenario controller in both Bronze bundles | Passed |

## Release gates still open

- Strict admission and execution of all 63 vocations in the full scenario.
- Trade and services with balanced inventories and native interaction histories.
- Combined save/open with deterministic continuation.
- Long-duration full-scenario stress tests, visible View/Terrain/Control
  checks and LLDB breakpoint verification.
- Signed distribution is outside the unsigned local build qualification.

## Reproduce local qualification

From the repository root, run the deterministic suite and the three-mode
contract. These are bounded checks and require no scheduled CI job.

```sh
CC=clang bash test.sh
make -C bronze contract-test
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
Scenario menu to confirm successful slice loads and the diagnostic shown
when the full scenario is rejected.
