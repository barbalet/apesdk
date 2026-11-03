# Strict scenario staging (cycle 03)

`bronze_scenario.c` owns an opaque instance per graphical scene model. Loading
parses/compiles a candidate with the canonical strict compiler, applies graphical
host admission checks, then atomically replaces the active config. Failure
frees the candidate and retains the previous config/name/seed. It does not run
an agent loop or dispatch pilot economic actions.

The Swift controller in `../ui/` resolves the three known filenames through its
Bundle (production default: Bundle.main). It starts with first_slice, exposes
active-file/error state, and invokes the world-reset callback only after success.
The guarded shared shell uses the active seed for native startup/New Simulation.
The default and selected configurations are owned for the scene model's lifetime.
Reset follows the native New Simulation path and respects its processing/pause
behavior. Failed selection never requests a reset.

Both C and Swift BRONZE_SIM compilation conditions exist only on bronze-mac.
The shared Swift shell's additions are all `#if BRONZE_SIM`. There are no new
shared engine C hooks. No independent authoritative time, needs or position is
introduced by loading. Cycle 06 also applies validated terrain thresholds. See the
[landscape policy](../hybrid/README.md) for terrain keys and bounds. Cycle 08
adds [occupation settings](../actors/PROFESSIONS.md) and a validated roster.
Cycle 09 stages the retained compiled program for the [task executor](../tasks/README.md).

## Authoring checks and diagnostics

- Strict parsing, name-to-ID compilation and reference validation remain in
  `dsl/`. `brz_parse_file_report` supplies an instance-owned FILE diagnostic sink;
  the loader never redirects process stderr, exits the app or retries as legacy.
- Numeric tokens must be finite. All conversions to bounded integer fields are
  checked before casting. Task nesting is limited to 64. Partial task/rule/
  vocation AST nodes share the compiler's destruction functions on failure.
- Graphical action admission counts numeric arguments as well as words and
  rejects word arguments where economic quantities must be numeric.
- The graphical loader limits files to 2 MiB and retains at most 16 KiB of
  diagnostics. Displayed messages identify the attempted filename plus compiler
  source lines; zero denotes a file-level error (missing file/host policy).
- Population/place authoring maxima are 100,000/10,000. Economic capacity,
  market targets and quantities are bounded to 1e9 units. Recipe inputs/outputs
  must be positive. Renew/nutrition/recovery/need and probability values use
  0..1; chance blocks use 0..100 percent. Map dimensions are whole cells in
  1..4096; durations/report intervals are whole days up to 1,000,000.
- Supported scalar parameters are checked by graphical scenario admission. Unknown
  names and string-valued parameters are rejected rather than silently ignored
  by this host. Service-profile/map/place/role/need/service/disruption declarations
  are rejected until their execution integration exists.
- These checks establish authoring/configuration bounds, not performance claims
  at the maximum values or persistent actor/place identities. Numeric kind IDs
  are scoped to the compiled config; later save integration must fingerprint it.

The full scenario remains rejected for the cycle 01 semantic mismatches. The
other two bundled files compile. Success is labeled as validated content with
profession tasks inactive; it does not substitute for cycles 04–18.

## Reproducible checks

`make -C bronze contract-test` covers transactional replacement, repeated loads,
missing/malformed files, numeric conversion safety, resource bounds, unsupported
profiles/parameters, seed changes and unchanged cycle 01/02 snapshots.

`test_bundle.sh /absolute/path/to/BronzeSim.app` compiles the real Swift
controller/UI source with a small Foundation/SwiftUI test driver. It resolves the
actual Debug/Release bundle resources from a temporary working directory,
verifies startup, selection, full-scenario rejection and successful-only reset
callbacks. It exercises the controller, not mouse clicks or live window layout.
The Mac CI build matrix runs it after packaging. The original Catalyst target
is also built separately without BRONZE_SIM to verify the shared shell split.
