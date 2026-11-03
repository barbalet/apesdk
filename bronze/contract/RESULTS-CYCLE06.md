# Cycle 06 validation record

Validated locally on 2026-11-03 with Apple clang on Darwin arm64.

| Check | Result | Log |
| --- | --- | --- |
| Full deterministic suite | Passed | `/tmp/bronze-cycle06-suite.log` |
| Native and Bronze compatibility contract | Passed | `/tmp/bronze-cycle06-contract.log` |
| Actor integration sanitizer suite | Passed | `/tmp/bronze-cycle06-sanitizers.log` |
| Bronze Catalyst Debug build with project signing settings | Passed | `/tmp/bronze-cycle06-build.log` |
| Packaged Debug scenario and controller | Passed | `/tmp/bronze-cycle06-bundle.log` |

Terrain tests check field slope and distance to water, wrapped connectivity,
disjoint patches, height bands for both ores, bounded capacity and persistent
ore depletion. They also check that stricter policy removes unsuitable fields,
that the same terrain and policy reproduce the full footprint, and that
analysis leaves native heights intact. Session tests exercise distinct policy
and stock in two native worlds.

The contract hashes patch geometry and stocks for seeds 42 and 1337. Six
Bronze landscape rows changed. Native body, terrain, weather and lightning
rows and standalone CLI snapshots stayed identical. The first-slice inventory
records seven new terrain parameters in both parser profiles.

Ore classification is a procedural proxy based on native relief, slope and
elevation. The generator can place fewer patches on terrain that lacks enough
suitable contiguous cells. Township placement and ownership are cycle 07.
