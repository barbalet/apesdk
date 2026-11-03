# Cycle 03 validation record

Date: 2026-11-02 (America/Los_Angeles). Darwin arm64, Apple clang 21.0.0,
Xcode 27.1. No scenario content or pinned cycle 01/02 baseline was updated.

## Implemented and tested

All three .bronze files are actual app resources. The default is first_slice;
vertical_slice can also be selected. Strict compilation still rejects
bronze_age_full with its original source-line diagnostics. One scene-owned
loader stages a compiled candidate; successful selection invokes the native
world-reset route with its seed, and failed selection retains active state.
The View status panel identifies the active file and exposes diagnostics.
The Bronze Scenario menu labels successful selection as a world reset.

The canonical parser now supports per-instance diagnostic FILE streams instead
of process stderr redirection. It checks numeric conversion safety and bounds
nesting, and reuses canonical AST disposal for incomplete task/rule/vocation
nodes. Host admission checks quantities, units, references compiled by the
canonical compiler, supported parameter names and unsupported profile features.
Only guarded BRONZE_SIM Swift branches change the shared shell; no shared
engine C source or native save/being layout changed.

| Check actually run | Result | Local diagnostic log |
| --- | --- | --- |
| `bash test.sh` | PASS; original C modules plus legacy/adapters/hybrid | `/private/tmp/bronze-cycle03-tests.log` |
| `make -C bronze contract-test` | PASS; unchanged snapshots and scenario tests | `/private/tmp/bronze-cycle03-contract.log` |
| Scenario loader C test with AddressSanitizer and UndefinedBehaviorSanitizer | PASS, no sanitizer errors | Reproduction described below |
| bronze-mac Debug, Mac Catalyst, no signing override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle03-bronze-Debug.log` |
| bronze-mac Release, Mac Catalyst, no signing override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle03-bronze-Release.log` |
| Actual Debug bundle + production Swift controller | PASS outside repository cwd | `/private/tmp/bronze-cycle03-bundle-Debug.log` |
| Actual Release bundle + production Swift controller | PASS outside repository cwd | `/private/tmp/bronze-cycle03-bundle-Release.log` |
| Original ApeSim-MacCatalyst Debug, no BRONZE_SIM | BUILD SUCCEEDED | `/private/tmp/bronze-cycle03-original-catalyst.log` |
| Original sim-mac Debug and Release, no BRONZE_SIM | BUILD SUCCEEDED | `/private/tmp/bronze-cycle03-native-{Debug,Release}.log` |

The contract tests now audit both Swift/C Bronze definitions, exactly-once
resource references and loader/UI source ownership. They still check the
separate native/HIDE_THUNDER/Bronze probes, native link isolation, original CLI
outputs and all existing AST inventories without re-recording them.

Scenario tests cover missing files, missing bundle entries, malformed syntax,
excessive nesting, oversize files, unsafe integer conversion, resource bounds,
argument units/counts, unknown parameters, unsupported service declarations,
repeated replacements, seed changes and retention after failed loads. Bundle
tests compile the same Swift controller/UI source as the app, read the real
packaged resources, and verify that only successful selections invoke the reset
callback. They do not synthesize menu clicks or inspect live window layout.

Reproduce the packaged checks with:

```sh
bash bronze/scenario/test_bundle.sh /absolute/path/to/BronzeSim.app
```

For the sanitizer check, compile `bronze/dsl/*.c`,
`bronze/scenario/bronze_scenario.c` and `bronze/scenario/test_scenario.c` with
`-std=c99 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer` and the
`bronze/dsl` include directory. Run from a temporary directory with absolute
paths to first_slice, vertical_slice and bronze_age_full, in that order.

## Limits and CI

The active config is validated/staged; only its seed is applied to native
startup/reset. Scenario-defined populations, workplaces, inventories and
profession tasks are NOT executed by this cycle. Native behavior and the
prototype economy remain authoritative. Persistent actor IDs and combined
save/resume are later cycles. A valid file is not evidence of full simulation
semantics or historical accuracy.

A Mac CI Debug/Release job now builds the Bronze app without a signing override
and runs the packaged controller/resource tests; Linux gcc/clang contract jobs
include the portable loader tests. These remote CI jobs were configured, not
executed locally. Existing compiler/SDK warnings remain. No live GUI visual
acceptance, signed distribution or long-run profession simulation is claimed.
