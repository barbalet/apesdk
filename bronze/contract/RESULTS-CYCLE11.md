# Cycle 11 validation record

Validated locally on 2026-11-03 with Apple clang on Darwin arm64.

| Check | Result | Log |
| --- | --- | --- |
| Full deterministic suite | Passed | `/tmp/bronze-cycle11-suite.log` |
| Native and Bronze compatibility contract | Passed | `/tmp/bronze-cycle11-contract.log` |
| Hybrid task harness with AddressSanitizer and division checks | Passed | `/tmp/bronze-cycle11-sanitizers.log` |
| Bronze Catalyst Debug build | Passed | `/tmp/bronze-cycle11-build.log` |
| Packaged Debug scenario and controller | Passed | `/tmp/bronze-cycle11-bundle.log` |

The hybrid harness checks scenario patch capacity, competing claims, carrying
limits, claim release on death, and stock conservation across gathering, eating
and deposit. It exercises the DSL deposit operation, rejects direct remote
transfers and rejects deposits that exceed storage capacity. It verifies
native nutrition, bounded field renewal, no duplicate renewal on the same
day, and zero renewal for a mineral configured with a zero rate.

The implementation uses configurable total cargo and township storage caps.
Renewal uses scenario rates and the terrain moisture proxy for field and
woodland resources. Consumption and death are recorded as resource sinks.
Original and Bronze compatibility baselines matched.
