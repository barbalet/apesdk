# Cycle 12 validation record

Validated locally on 2026-11-03 with Apple clang on Darwin arm64.

| Check | Result | Log |
| --- | --- | --- |
| Full deterministic suite | Passed | `/tmp/bronze-cycle12-suite.log` |
| Native and Bronze compatibility contract | Passed | `/tmp/bronze-cycle12-contract.log` |
| Hybrid task harness with AddressSanitizer and division checks | Passed | `/tmp/bronze-cycle12-sanitizers.log` |
| Bronze Catalyst Debug build | Passed | `/tmp/bronze-cycle12-build.log` |
| Packaged Debug scenario and controller | Passed | `/tmp/bronze-cycle12-bundle.log` |

The strict scenario fixture deposits supplies into township storage and
runs the compiled food, bronze-tool, pottery and glazing tasks. It verifies
missing-input, tool and skill failures, duplicate recipe input aggregation,
competing claims, survival interruption, cancellation without consumption,
batch quantities, food consumption, item deposit, reusable tools and
workplace crafting from personal cargo.

Recipe policies default to four native work minutes per batch at the township
service entrance. Authoring can choose a personal-cargo workplace, require a
carried tool or specify a minimum vocation skill. The fixture supplies clay
and other inputs directly through deposits. Bundled scenarios continue to
depend on their configured supply and vocation chains. No bundled recipes
define tool wear or repair. Original and Bronze compatibility baselines matched.
