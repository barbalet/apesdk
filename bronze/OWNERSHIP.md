# Bronze source ownership after cycle 02

Established 2026-11-02. This supersedes the duplicate-source layout recorded by
cycle 01. The original ApeSDK engine remains independent of these modules.

## Canonical modules

| Module | Owns | Consumers |
| --- | --- | --- |
| `dsl/brz_parser.{h,c}` | One lexer/parser and explicit parse profiles | Standalone CLI, adapter tests, contract inventory; Bronze scene controller for strict loading |
| `dsl/brz_dsl.{h,c}` | One AST/config/type layout, lifetime functions, shared validation body and strict semantic compiler | Every Bronze host |
| `dsl/brz_{kinds,vec,util}.{h,c}` | Shared registries, collections, utilities | All Bronze code; no copies remain in runtime/ |
| `runtime/` | Standalone BrzWorld, BrzAgent, settlements, local ports and compatibility interpreter | CLI only; these are not native graphical beings |
| `adapter/bronze_apesdk_adapter.{h,c}` | Native access primitives, scenario views, pilot economic sidecar/actions and pilot persistence | Native harness and bronze-mac build |
| `adapter/bronze_apesdk_compiled.{h,c}` | Restricted compiled-task pilot executor | Native harness and bronze-mac build |
| `actors/` | Registry-scoped native being IDs, owned sidecar schema and lifecycle/binding APIs | BRONZE_SIM hooks, native adapter factory and selected-being bridge |
| `hybrid/` | Current graphical site/economy/HUD prototype | BRONZE_SIM native hooks only |
| `work/` | Native clock/needs observation, bounded stationary intentions and action arbitration | Guarded sim-cycle/awake hooks; registry-owned state |
| `scenario-runtime/` | Small neutral view/event/service-fixture contract | Adapter; source-only, no ApeSDK-owned structs |

The old root `bronze_apesdk_adapter.h` and `bronze_apesdk_compiled.h` are
header-only compatibility includes. They declare no independent types or
functions. Their old root `.c` implementations are removed. Both old native
access and scenario-view APIs now resolve to the same adapter implementation;
the lower-case actor typedef aliases `BronzeApeActor`. Native view position
reads use the common native access primitive.

The adapter is the only ApeSDK binding owner. Standalone ports in runtime/
bind standalone world/agent types, not ApeSDK. The graphical prototype is still
a separate additive policy, not a second implementation of the DSL scheduler.
Do not wire pilot economic action stubs into it as a replacement for native
movement or bounded resource work.

## One AST, two explicit compatibility contracts

`brz_cfg_init()` and `brz_cfg_free()` own all AST/config allocations. Both
profiles call the same parser implementation and produce the same C types.
Profile flags are ephemeral host metadata, not a new historical native format.

### Strict / compiled

`brz_parse_file(path, &config)` parses, validates and compiles. Success sets
`compiled=true` and `legacy_runtime=false`. Strict default policy remains
`deposit_threshold=2`, `rest_recovery=0.04`. Existing compile diagnostics and
rejection of the full scenario are intentionally unchanged.

The pilot compiled-task executor rejects uncompiled or legacy configurations
before mutating sidecars or emitting actions. This prevents zero-initialized
legacy operation fields being mistaken for compiled `gather` codes.

### Explicit standalone compatibility

`brz_parse_file_legacy_runtime(path, &config)` uses the same parser and shared
validator with the old standalone validation policy. It restores zero defaults
for omitted settlement policies, preserves explicit policy values and leaves
`compiled=false`, `legacy_runtime=true`. The standalone runner requires this
profile. It retains the original interpreter behavior, including known missing
handlers and fallback semantics; it does not claim compiled semantic support.

A legacy AST cannot be promoted by calling `brz_cfg_compile()`; that call fails.
Reparse with the strict API when compiled behavior is required. Service-profile
and map/place/role/need/service/disruption declarations are rejected by the
compatibility host rather than accepted as if its interpreter executed them.

This is a consumer compatibility policy, not a second compiler or a build-time
fork. In particular, there is no silent retry from failed strict compilation to
the legacy profile. The CLI chooses compatibility explicitly; future graphical
scenario loading must choose strict compilation and report its actual failures.

## Behavior intentionally preserved

- All three standalone scenario stdout/stderr snapshots match the executable
  captured before consolidation; `contract/cli_baselines.json` pins them.
- `contract/content_inventory.json` remains byte-for-byte unchanged. Its
  historical `runtime` and `dsl` keys now mean compatibility and strict profiles
  of the canonical compiler; the full file remains rejected in strict mode.
- `contract/baselines.json` remains unchanged for original native, native with
  HIDE_THUNDER, and (through cycle 04) the Bronze graphical prototype. Cycle 05
  updates only Bronze resource rows for the new work gate; original values
  remain pinned.
- No resource identifiers, action meanings, native being layout or native save
  fields changed. Parameter-prefix mismatches and incomplete explicit eat/deposit
  behavior are still documented gaps, not fixed incidentally by this refactor.
- Cycle 02 compiled the canonical compiler, adapter and neutral contract exactly
  once. Cycle 03 now adds `scenario/` as owner of staged compiled configurations
  and diagnostics, and `ui/` as owner of bundle lookup, selection and status UI.
  Only small `#if BRONZE_SIM` hooks live in the shared Swift shell. Profession
  execution binding remains later work; validation is not a second scheduler.

## Tests and split from the original engine

Run from the repository root:

```sh
bash test.sh
make -C bronze contract-test
```

The contract audit checks source ownership and Xcode source references to prevent
new duplicate compiler/adapter implementations. Profile tests exercise nested
AST compilation, repeated initialization/freeing, strict rejection, compatibility
defaults and rejection of unexecutable service declarations. The native harness
checks the merged access APIs and rejects legacy/uncompiled task dispatch.

A compiler-only AddressSanitizer/UndefinedBehaviorSanitizer run also covers the
profile/AST/error cleanup paths. See `contract/RESULTS-CYCLE02.md` for actual
results and limits. Old full-scenario acceptance is not proof of full semantics.

All app-only native hooks remain behind `#ifdef BRONZE_SIM`. No shared engine C
source needed modification for cycle 02. Original sim-mac still uses no Bronze
objects, compiler headers, content or runtime symbols. `HIDE_THUNDER` and local
no-signing settings remain unchanged and independently audited.

Cycle 04 adds a registry pointer only to the guarded private shared-core context,
not to simulated_being or native saves. Its actor records contain the schema
for later profession/task/inventory work. Registry identity and neutral v1
scenario IDs have a checked conversion boundary; see actors/README.md.
