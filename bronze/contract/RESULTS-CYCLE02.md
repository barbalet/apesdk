# Cycle 02 validation record

Date: 2026-11-02 (America/Los_Angeles). Host/compiler: Darwin arm64,
Apple clang 21.0.0, Xcode 27.1.

## Changes validated

One compiler/AST/support implementation in `dsl/`; explicit strict versus
standalone compatibility entry points; one native adapter owner in `adapter/`;
CLI and native harness build lists migrated; canonical sources added once to
the Catalyst target. No shared engine C source, native being layout, native
save layout, scenario file or cycle 01 snapshot was changed.

## Actual results

| Check | Result | Local diagnostic log |
| --- | --- | --- |
| `bash test.sh` | PASS, including native modules and migrated native/hybrid harnesses | `/private/tmp/bronze-cycle02-tests.log` |
| `make -C bronze contract-test` | PASS with unchanged cycle 01 snapshots | `/private/tmp/bronze-cycle02-contract.log` |
| Three standalone CLI outputs captured before migration and compared afterward | PASS; stdout/stderr SHA-256 matches for all three scenarios | `cli_baselines.json` |
| Compiler profile/AST/error-path test, AddressSanitizer + UndefinedBehaviorSanitizer | PASS; expected invalid-input diagnostics, no sanitizer errors | Command described below |
| bronze-mac Debug, Mac Catalyst, no signing override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle02-bronze-Debug.log` |
| bronze-mac Release, Mac Catalyst, no signing override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle02-bronze-Release.log` |
| Original sim-mac Debug, macOS, signing disabled for local validation | BUILD SUCCEEDED | `/private/tmp/bronze-cycle02-native-Debug.log` |
| Original sim-mac Release, macOS, signing disabled for local validation | BUILD SUCCEEDED | `/private/tmp/bronze-cycle02-native-Release.log` |

The contract gate now checks that removed duplicate implementation paths stay
absent and the app source phase references canonical compiler/adapter files
exactly once. It exercises both profiles through one parser, nested when/chance
AST compilation, profile defaults, rejection paths, repeated free/reinitialize,
legacy compile rejection, and the three original CLI summaries.

The native land harness exercises merged native position/energy/fatigue/clock
APIs and checks that legacy or uncompiled configurations cannot enter compiled
action dispatch. Existing fresh-run event and persistence comparisons pass.
The cycle 01 seeds 42/1337 and native/native-hide/Bronze scalar traces are still
checked without updating `baselines.json` or `content_inventory.json`.

To reproduce the compiler sanitizer check, compile `dsl/*.c` plus
`contract/compiler_test.c`, include `dsl/`, and use:

```sh
-std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer
```

Run that binary in a temporary working directory with absolute paths to
`content/first_slice.bronze` and `content/vertical_slice.bronze` as its two
arguments. It writes/removes a local `profile-test.bronze` fixture.

## Deliberately unchanged limitations

The full scenario still fails strict semantic compilation. Known movement,
gathering, crafting, explicit eat/deposit and parameter-name gaps have not been
fixed or disguised as part of consolidation. The app compiles the canonical
compiler but does not load content until cycle 03. Full profession integration
is later work. No visual behavior changed and no new GUI acceptance is claimed.

Compiler/Apple SDK warnings remain. Linux gcc/clang CI is configured but was
not executed on this Mac; CLI snapshot comparisons may require a reviewed
host-specific baseline if a real cross-platform floating-point difference is
found, rather than weakening native compatibility checks.
