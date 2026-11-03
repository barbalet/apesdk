# Cycle 01 validation record

Date: 2026-11-02 (America/Los_Angeles)

Baseline source revision: `2c6cf4398ad35b59bf3340010e4e841862ac2ad0` plus this
cycle's documentation/test-only changes. No production C or scenario edits.
Host: Darwin arm64; Apple clang 21.0.0 (clang-2100.3.34.2); Xcode 27.1 (27A9275).

| Check actually run | Result | Local diagnostic log |
| --- | --- | --- |
| `bash test.sh` | PASS; native modules and existing Bronze tests | `/private/tmp/bronze-cycle01-tests.log` |
| `python3 bronze/contract/check.py --record` | PASS; initial reviewed snapshots captured | `/private/tmp/bronze-cycle01-contract.log` |
| `make -C bronze contract-test` | PASS; existing snapshots verified without recording | `/private/tmp/bronze-cycle01-check.log` |
| bronze-mac Debug, Mac Catalyst, no signing override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle01-bronze-Debug.log` |
| bronze-mac Release, Mac Catalyst, no signing override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle01-bronze-Release.log` |
| Original sim-mac Debug, macOS, signing disabled for validation | BUILD SUCCEEDED | `/private/tmp/bronze-cycle01-native-Debug.log` |
| Original sim-mac Release, macOS, signing disabled for validation | BUILD SUCCEEDED | `/private/tmp/bronze-cycle01-native-Release.log` |

Temporary diagnostic logs are not required to run the checks and are not durable
repository artifacts. The committed scalar traces and content inventory are the
reproducible evidence. Compiler/SDK warnings remain; the tests do not require a
warning-free legacy source base.

Both record and check runs verified seeds 42 and 1337, cycles 1/60/120, twice
in fresh processes per variant. Sampled native state/layout matched native,
native+HIDE_THUNDER and BRONZE_SIM+HIDE_THUNDER. Separate lightning masks differ
as expected. Prototype stock/economy traces were deterministic.

Known negative capability result: `dsl/` rejects `bronze_age_full.bronze` while
`runtime/` accepts it. This is intentionally recorded, not fixed in cycle 01.
The three input scenarios contain no nested when/chance task blocks. See the
contract matrix for other unsupported or misleadingly accepted behavior.

The gcc/clang Linux CI gate has been configured, not executed locally. Linux
currently has no pinned host baseline; its gate performs inventory, isolation,
repeatability and differential checks and reports the missing host snapshot.
No new GUI smoke test, sanitizer run, long-duration scenario or distribution
signing validation was performed for this documentation/test-only cycle.
