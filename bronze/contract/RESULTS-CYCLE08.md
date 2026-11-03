# Cycle 08 validation record

Validated locally on 2026-11-03 with Apple clang on Darwin arm64.

| Check | Result | Log |
| --- | --- | --- |
| Full deterministic suite | Passed | `/tmp/bronze-cycle08-suite.log` |
| Native and Bronze compatibility contract | Passed | `/tmp/bronze-cycle08-contract.log` |
| Actor integration sanitizer suite | Passed | `/tmp/bronze-cycle08-sanitizers.log` |
| Bronze Catalyst Debug build with project signing settings | Passed | `/tmp/bronze-cycle08-build.log` |
| Packaged Debug scenario and controller | Passed | `/tmp/bronze-cycle08-bundle.log` |

The real-engine harness verifies a 3:1 distribution across four slots, native
age eligibility, an unavailable coastal role, a full workplace, and vacancy
assignment on the next review. It also verifies capped skill growth on an
assigned field, no gathering at an unrelated mine, and per-town production
credit. The actor suite covers compaction and cross-session identity, plus
the selected-actor summary and roster staging across a pending world reset.
Scenario admission exercises roster extraction and rejects an unknown vocation
setting.

Original body, terrain, weather and lightning traces match their prior golden
rows. The Bronze trace reflects removing provisional workplaces from worlds
with no active occupation roster. The two supported bundled scenarios record
explicit occupation settings. Standalone CLI snapshots passed without changes.

The authoring and assignment choices are documented in the occupation policy.
Coastal jobs await coastal workplaces, and service jobs await task execution.
Skill practice currently follows successful attributed gathering. Cycle 09 adds
DSL task execution, and cycle 10 adds travel.
