# Cycle 09 validation record

Validated locally on 2026-11-03 with Apple clang on Darwin arm64.

| Check | Result | Log |
| --- | --- | --- |
| Full deterministic suite | Passed | `/tmp/bronze-cycle09-suite-final.log` |
| Native and Bronze compatibility contract | Passed | `/tmp/bronze-cycle09-contract-final.log` |
| Actor integration sanitizer suite | Passed | `/tmp/bronze-cycle09-sanitizers-final.log` |
| Bronze Catalyst Debug build | Passed | `/tmp/bronze-cycle09-build-final.log` |
| Packaged Debug scenario and controller | Passed | `/tmp/bronze-cycle09-bundle-final.log` |

The hybrid harness stages a strict compiled scenario, releases the scenario
controller, and runs the retained program on native actors. It checks weighted
selection and the exact random draws for rule and branch evaluation. It also
checks repeatability after reset, nested conditions, arrival on a real patch,
survival interruption and resumption, one-time stock accounting, skill growth,
rest, task completion, and explicit failures for missing travel and unavailable
operations. Package checks cover retaining the current program after a failed
scenario load.

Both Linux jobs in GitHub Actions run 41705478454 reported an undeclared
`size_t` in the shared Bronze diagnostics declaration. That API now uses
ApeSDK’s `n_uint`. The C implementation converts at the `snprintf` boundary,
and the compatibility audit compiles the shared header on its own. Linux CI
verification awaits a fresh run. The local compatibility checks matched
original and Bronze baselines.
