# SimulatedUniverse Development Cycles 141-150

These cycles begin the ApeScript and extensibility block. The implementation
artifact is `SimulatedUniverse/Runtime/UniverseScripting.swift`.

## Cycle 141: ApeScript Inventory

Decision: ApeScript examples should be classified before the UI presents them.

Implementation:

- `ApeScriptExampleClassification`
- `ApeScriptExampleInventoryItem.starterInventory`

Examples are classified as safe demos, engine manipulation, brain experiments,
or debug utilities.

## Cycle 142: Script Loading

Decision: scripts can originate from local files, bundled examples, recent
scripts, or experiment attachments.

Implementation:

- `ScriptLoadingSource`
- `ScriptDocumentReference`

Script documents expose a deterministic content hash for logs and replay.

## Cycle 143: Script Panel

Decision: the script panel needs source text, parse status, diagnostics, and
execution controls.

Implementation:

- `ScriptParseStatus`
- `ScriptExecutionControl`
- `ScriptPanelModel`

The panel can report whether execution is currently allowed.

## Cycle 144: Parser Diagnostics

Decision: parser diagnostics should include line, column, severity, message,
and help text.

Implementation:

- `ScriptDiagnosticSeverity`
- `ScriptDiagnostic`

## Cycle 145: Dry Run And Validation

Decision: validation should be safe even if the engine cannot perform a true
dry run yet.

Implementation:

- `ScriptValidationMode`
- `ScriptDryRunCapability.conservative`

The conservative fallback is parse-only validation.

## Cycle 146: Script Execution Logging

Decision: script execution logs need name, hash, cycle, selected being, touched
variables, and result.

Implementation: `ScriptExecutionLogRecord`.

## Cycle 147: Replay And Experiment Metadata

Decision: scripts must participate in deterministic replay and experiment
metadata.

Implementation: `ScriptReplayPolicy.deterministic`.

The policy records script hash, avoids local path leakage, blocks replay on hash
mismatch, and attaches to experiment metadata.

## Cycle 148: Capability Checks

Decision: builds without ApeScript support should fail clearly before showing
unsafe controls.

Implementation: `ScriptCapabilityCheck`.

The check returns missing bridge capabilities and a user-facing unavailable
reason.

## Cycle 149: Bundled Examples

Decision: bundled examples should demonstrate brain activity, weather
experiments, and social observation.

Implementation: `BundledScriptExample.defaults`.

## Cycle 150: Output Routing

Decision: script output should route into the run log and script panel without
blocking the simulation loop.

Implementation:

- `ScriptOutputRoute`
- `ScriptOutputMessage`
- `ScriptOutputBuffer`

Messages are non-blocking and capped by buffer size.
