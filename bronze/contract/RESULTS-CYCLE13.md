# Cycle 13 verification

The hybrid harness covers private and communal access at physical entrances,
recipe reservations, the daily ration limit and reset, and separate spoilage
and consumption accounting. It also checks that native feeding blocks a Bronze
meal in the same tick and that native fatigue recovery is applied during rest.

| Check | Result |
| --- | --- |
| Clang repository test suite | Passed |
| Hybrid harness with AddressSanitizer and integer division checks | Passed |
| Deterministic contract for original, thunder-hidden and Bronze builds | Passed with existing baselines |
| Bronze Mac Catalyst Debug build | Passed |
| Packaged scenario resource and controller tests | Passed |
