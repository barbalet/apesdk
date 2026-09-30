# Optional BronzeSim adapter

This extension depends on ApeSDK but ApeSDK core has no dependency on it. The
first vertical slice exposes ApeSDK land time and `simulated_being` identity /
position through the shared scenario-runtime read-only ports. Economic state
remains Bronze-owned sidecar data.

`BronzeApeSidecar` is intentionally separate from `simulated_being`; it holds
the initial farmer/fisher/smith resource inventory and need values without
changing ApeSDK save compatibility.

One Bronze day is exactly 1,440 ApeSDK land-time cycles. The adapter derives
days from the simulation clock, never from rendering frames.

The headless harness also obtains an actual `simulated_being` from
`sim_group()` and verifies that `ScenarioActorPort` reads its production
position. The adapter keeps its stable scenario actor ID and Bronze economic
sidecar outside that ApeSDK type.

The initial deterministic vertical slice covers `move`, `gather`, `craft`,
`deposit`, `eat`, and `rest`.  It uses Bronze-owned settlement/resource and
need sidecars and emits versioned `ScenarioEvent` records.  It deliberately
does not add occupation or market fields to `simulated_being`.

The harness executes that slice twice from fresh ApeSDK initialization and
requires all event fields to match.  This is the extension's deterministic
trace gate; the eventual compiled `.bronze` runner must retain it.

`content/vertical_slice.bronze` is parsed and compiled using the copied
Bronze DSL package in `dsl/`. Its farmer, fisher, and smith tasks drive all
six vertical-slice actions—`move_to`, `gather`, `craft`, `deposit`, `eat`, and
`rest`—through the adapter. Economic state remains a Bronze sidecar; the next
extension step is a versioned sidecar-state/event persistence format that does
not alter ApeSDK's existing save compatibility.
