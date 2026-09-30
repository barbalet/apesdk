# Scenario Runtime C99 pilot

This package owns the small scenario-neutral contract shared by BronzeSim and
Mosul. It intentionally contains no simulation-owned structs, map formats,
navigation code, ApeSDK dependency, allocation, or mutation API.

## Version 1 vector

`scenario_service_fixture_run()` reads opaque world, actor, and place views and
emits two events: a recipient travels to the place at tick `T`, then a provider
performs the named service at `T + 1`. A completed service delivers the requested
amount; a deferred service delivers zero. The fixture records the recipient's
arrived place and position as observable state. Both projects run this exact
vector for `water_provision` at tick 60 with provider 7, recipient 8, and place
2.

The package is the pilot's shared ownership location. It remains source-only;
each repository compiles it with its own C99/C11 build and provides local
read-only view callbacks.
