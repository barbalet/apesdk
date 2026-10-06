# Bronze scenario package

This optional package is ApeSDK's landing zone for the BronzeSim economic and
cultural scenario. It depends on ApeSDK; ApeSDK core has no dependency on it.

It contains the migrated runtime and content plus an adapter over ApeSDK land,
clock, and `simulated_being` identity/position. Economic inventory,
occupations, and settlement stores remain Bronze sidecar state, preserving
ApeSDK's core types and native save compatibility.

The initial deterministic farmer/fisher/smith slice executes `move`,
`gather`, `craft`, `deposit`, `eat`, and `rest` from compiled `.bronze`
content. One Bronze day is exactly 1,440 ApeSDK land-time cycles.

`make test` runs both migrated Bronze scenarios and the headless harness
linked against real ApeSDK sources. The harness compares event traces from two
fresh runs, and verifies versioned Bronze sidecar snapshot/event-log round
trips. These persistence files belong to this package; they do not alter
ApeSDK's native save format.
