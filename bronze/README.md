# Bronze scenario package

This package is the apeSDK landing zone for the BronzeSim economic/cultural
scenario.  It intentionally depends on apeSDK; no existing apeSDK core module
depends on this directory.

The staged contents are:

* `adapter/` — the narrow bridge from apeSDK terrain, beings, and simulation
  time to Bronze domain ports;
* `content/` — `.bronze` scenarios;
* `dsl/`, `economy/`, `world/`, and `runtime/` — the compiler, settlement
  economy, resource overlay, and daily scheduler as they are migrated from
  BronzeSim; and
* `test/` — deterministic adapter tests.

The first adapter deliberately maps only position, energy, fatigue, terrain,
and the simulation clock. Economic inventories, occupations, and settlements
remain sidecar Bronze state instead of changing `simulated_being` or apeSDK
save formats.
