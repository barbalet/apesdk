# Cycle 10 validation record

Validated locally on 2026-11-03 with Apple clang on Darwin arm64.

| Check | Result | Log |
| --- | --- | --- |
| Full deterministic suite | Passed | `/tmp/bronze-cycle10-suite.log` |
| Native and Bronze compatibility contract | Passed | `/tmp/bronze-cycle10-contract.log` |
| Actor integration sanitizer suite | Passed | `/tmp/bronze-cycle10-sanitizers.log` |
| Bronze Catalyst Debug build | Passed | `/tmp/bronze-cycle10-build.log` |
| Packaged Debug scenario and controller | Passed | `/tmp/bronze-cycle10-bundle.log` |

The hybrid harness verifies route preparation, a native goal taking precedence,
survival interruption with a retained cursor, and travel across a wrapped map
seam using the native movement integrator. It checks that crossing does not
award gathering and that an isolated dry starting cell with wet and steep
neighbors produces a blocked route. Existing task tests cover gated gathering,
rest, resource accounting and deterministic replay.

The route policy uses conservative dry-land and slope thresholds and bounded
search and movement attempts. A deferred route follows the existing one-hour
task retry. The compatibility contract matched original and Bronze baselines.
