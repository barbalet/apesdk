# SimulatedUniverse Development Cycles 031-040

These cycles finish the first bridge-contract pass. The implementation artifacts
are `SimulatedUniverse/Bridge/UniverseBridgeContracts.swift` and the draft C
facade header `SimulatedUniverse/Bridge/SimulatedUniverseBridge.h`.

## Cycle 031: C Facade Plan

Decision: the bridge should expose a small C facade with explicit context
ownership and JSON snapshot reads before Swift reaches deeper into ApeSDK.

Facade shape:

- `sua_context_create` and `sua_context_destroy` own simulation lifetime.
- `sua_context_step` serializes cycle stepping.
- `sua_context_version` reports ApeSDK version, snapshot contract version, and
  compile-time capabilities.
- `sua_snapshot_*_json` reads copied snapshots for universe, beings, terrain,
  memory, and social graph state.
- `sua_bridge_free` releases copied memory returned by facade calls.

The draft header is intentionally narrower than the current ApeSDK surface so
SwiftUI cannot duplicate engine logic.

## Cycle 032: Lifetime Management

Ownership rules:

- Simulation context is created and destroyed only through the facade.
- Transfer files are copied or written by the bridge, not retained by views.
- C strings returned by the facade are copied into Swift `String` immediately.
- Byte arrays returned by the facade are copied into `Data` or typed arrays.
- Callback pointers are installed during runtime setup and removed during
  teardown.

Implementation: `BridgeOwnedResource`.

## Cycle 033: Bridge Errors

The bridge now has explicit error codes and Swift mapping for:

- Failed initialization.
- Missing simulation context.
- Invalid selection.
- Invalid terrain sample.
- Transfer failure.
- Script failure.
- Unsupported capability.
- Snapshot contract mismatch.
- Rejected command.
- Memory ownership violation.

Implementation: `BridgeErrorCode` and `UniverseBridgeError`.

## Cycle 034: Bridge Tests

The first bridge test plan uses a deterministic mini-world fixture and checks
stable fields after 0, 1, 10, 100, and 200 cycles.

Stable fields:

- Snapshot contract version.
- Population count.
- Selected identity.
- Land date.
- Land time.
- Map metadata.

Failure cases cover initialization, selection, terrain sampling, and contract
mismatch.

Implementation: `BridgeFixtureProfile.deterministicMiniWorld` and
`BridgeTestPlan.initialDeterministic`.

## Cycle 035: Version Handshake

The version handshake reports:

- App snapshot contract version.
- Facade snapshot contract version.
- ApeSDK version number.
- ApeSDK short version name.
- Enabled bridge capabilities.

The app treats mismatched snapshot contract versions as a bridge error, not a UI
warning.

Implementation: `BridgeVersionHandshake`.

## Cycle 036: Compile-Time Capabilities

Compile-time capabilities are represented as data rather than inferred by UI:

- ApeScript.
- Brain.
- Braincode.
- Immune.
- Territory.
- Alpha weather draw.
- High-resolution terrain.
- Binary transfer.
- JSON transfer.

This is especially important because `BRAIN_ON` depends on `APESCRIPT_INCLUDED`
in the current headers and may not be available in every build.

Implementation: `BridgeCapability`.

## Cycle 037: Bridge Fixture Mode

The deterministic mini-world fixture defines:

- Scenario profile.
- Cycle count.
- Expected population range.
- Required capabilities.

This fixture will support bridge tests, renderer tests, panel tests, and replay
tests without requiring a full user session.

Implementation: `BridgeFixtureProfile`.

## Cycle 038: String And Byte Conversion

String policy:

- Null-terminated C strings are copied into Swift immediately.
- Fixed-length byte buffers are copied by explicit byte count.
- Optional C strings map nil to absent values, not empty strings.

Byte policy:

- Transfer payloads and image buffers copy to `Data`.
- Compact numeric snapshot fields copy to typed arrays.
- Borrowed read-only pointers can exist only within bridge implementation calls.

Implementation: `BridgeStringConversionPolicy` and
`BridgeByteConversionPolicy`.

## Cycle 039: Command Queue

The command queue rules are:

- Single writer: only the runtime controller submits facade commands.
- Ordered execution: commands execute in submission order.
- Snapshot after mutation: state-changing commands request fresh snapshots.
- Capability rejection: unsupported commands fail before reaching the engine.
- Mutation logging: every state-changing command emits a log or intervention
  event.

Implementation: `EngineCommandEnvelope`, `SimulationCommand.requiresSimulationMutation`,
and `CommandQueueRule`.

## Cycle 040: Contract Coverage Review

Every planned panel and overlay now declares the snapshot kinds it depends on:

- World viewport: universe and being snapshots.
- Selected ape panel: being snapshots.
- Memory panel: memory and event snapshots.
- Social graph panel: social and event snapshots.
- Drive timeline: being and drive snapshots.
- Environment inspector: terrain sample snapshots.
- Run log and script panel: event snapshots.
- Social, memory, weather, and resource overlays: their matching snapshot kinds.

Implementation: `ContractConsumer` and `SnapshotKind`.
