# Cycle 07 validation record

Validated locally on 2026-11-03 with Apple clang on Darwin arm64.

| Check | Result | Log |
| --- | --- | --- |
| Full deterministic suite | Passed | `/tmp/bronze-cycle07-suite.log` |
| Native and Bronze compatibility contract | Passed | `/tmp/bronze-cycle07-contract.log` |
| Actor integration sanitizer suite | Passed | `/tmp/bronze-cycle07-sanitizers.log` |
| Bronze Catalyst Debug build with project signing settings | Passed | `/tmp/bronze-cycle07-build.log` |
| Packaged Debug scenario and controller | Passed | `/tmp/bronze-cycle07-bundle.log` |

The hybrid harness exercises deterministic town and resource geometry, place
ID retirement on reset, zero initial stock by default, explicit scenario stock,
owner-only production credit, reachable housing with one slot per town,
workplace ownership and actors left unassigned when housing fills. Shared
session tests switch between worlds with distinct landscapes and stores.

The compatibility trace now hashes place IDs, ownership and actor assignments
for seeds 42 and 1337. Bronze landscape and scenario inventory snapshots reflect
the new rules. Original body, terrain, weather and lightning traces match
their prior baselines. Standalone CLI snapshots passed without changes.

The siting policy uses at most two townships. Actors without a reachable
vacancy wait for housing. Initial workplaces are provisional, and prototype
gathering credits township storage directly. Profession and transport logic
belong to subsequent cycles.
