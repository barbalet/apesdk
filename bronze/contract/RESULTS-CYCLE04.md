# Cycle 04 validation record

## Linux CI follow-up (2026-11-03)

The gcc, clang and coverage jobs in [run 41584858068](https://github.com/barbalet/apesdk/actions/runs/41584858068)
all failed at the same repeated-session selection assertion in test_actors.c.
Transfer output uses `n_file.size` as allocation capacity, while the restore
parser reads it as content length. Unwritten allocation tails were therefore
parsed as native state. Reproduced locally with `MallocScribble=1` before the
fix. Bronze snapshot capture now sets size to the written location, guarded
by `BRONZE_SIM`; historical builds and serialization stay unchanged.

The session harness now enables both macOS allocation scribbling and Linux
allocation perturbation. Local Apple-clang checks passed: the full deterministic
suite (`/private/tmp/ci-suite-fixed.log`), compatibility contract with unchanged
baselines (`/private/tmp/ci-contract-fixed.log`), and targeted actor sanitizers
(`/private/tmp/ci-actors-sanitized-fixed.log`). These are local checks; the fixed
Linux GitHub jobs have not yet run. The installed local gcc 14 cannot compile
against the current macOS SDK, so it supplied no additional validation.

Date: 2026-11-02 (America/Los_Angeles). Darwin arm64, Apple clang 21.0.0,
Xcode 27.1. No scenario content, pinned native/CLI/AST baseline, simulated_being
layout or native serialization format was changed.

## Scope delivered

World-context-owned actor registries, monotonic 64-bit IDs, owned sidecar schema,
real birth/death/compaction hooks, explicit reorder binding, claim/link cleanup,
context restore/rebind/destruction, native import identity reset and malformed
native transfer invalidation. Selected-actor identity is exposed through a
BRONZE_SIM-only session bridge and shown in the overview. All shared C/Swift
additions are guarded. Original external birth/death callbacks are preserved.

Metadata lookup works for inactive worlds, but native pointer resolution is
refused until that world's matching native state is active and rebound. This
avoids associating A's record with B's being at the same global memory address.

## Checks actually run

| Check | Result | Local evidence |
| --- | --- | --- |
| `bash test.sh` | PASS, including the new real-engine actor lifecycle/session harness | `/private/tmp/bronze-cycle04-tests.log` |
| `bash bronze/actors/test.sh` | PASS | `/private/tmp/bronze-cycle04-actors.log` |
| `make -C bronze contract-test` | PASS with original snapshots unchanged | `/private/tmp/bronze-cycle04-contract.log` |
| Targeted sanitizer actor integration | PASS: whole integration ASan, registry/harness UBSan | `/private/tmp/bronze-cycle04-targeted-sanitizers.log` |
| Full-engine UBSan attempted | FAIL at pre-existing native signed shift, independently reproduced without BRONZE_SIM | `/private/tmp/bronze-cycle04-sanitizers.log`, `/private/tmp/bronze-cycle04-native-ubsan.log` |
| bronze-mac Debug/Release, no signing override | BUILD SUCCEEDED | `/private/tmp/bronze-cycle04-bronze-{Debug,Release}.log` |
| Debug/Release packaged controller/resource checks | PASS | `/private/tmp/bronze-cycle04-bundle-{Debug,Release}.log` |
| Original sim-mac Debug/Release, no BRONZE_SIM | BUILD SUCCEEDED | `/private/tmp/bronze-cycle04-native-{Debug,Release}.log` |
| Original Catalyst Debug, no BRONZE_SIM | BUILD SUCCEEDED | `/private/tmp/bronze-cycle04-original-catalyst.log` |

## Identity cases covered

- Initial native population has one record per being and deterministic IDs.
- Deliberately duplicated native names, birth dates and genetics still receive
  different identities. No such attribute is used as a registry key.
- Real native pregnancy/birth delivers a registered newborn to the unchanged
  external birth callback. Repeated birth notification is idempotent, while an
  out-of-population pointer is rejected.
- Real death removal/compaction preserves survivors' record identity, skill,
  cargo, workplace/home, task progress and assignment history. Retirement
  releases claims once and clears mentor/apprentice links, including when no
  external death callback is installed.
- Explicit permutations preserve ID/state; duplicate/unknown ID ordering is
  rejected. Selection resolves to the actual selected native being.
- Reset, invalid reset and teardown retire IDs, release claims and never reuse
  IDs inside the same registry. Teardown release callbacks cannot resolve the
  retired actor to a newly replaced native body.
- Two real shared sessions retain independent sidecars and native positions
  across repeated snapshot switches. Inactive registries refuse native binding
  resolution. Closing one world does not destroy the other's records.
- Native save/open uses the unchanged native format and initializes fresh
  identities, without a second reset during deferred load setup. A missing file
  that performs no read preserves identities. Malformed native transfer retires
  them rather than attaching records to partially replaced data; a new world
  recovers the current session.
- The native adapter's registry factory supplies checked v1 IDs and rejects
  missing bindings. It does not silently truncate wide persistent identities.
- Existing fixed-seed native/hide/Bronze traces still match their pinned files;
  sampled Bronze probes now additionally assert complete actor/native bindings
  at cycles 1/60/120. No baseline re-recording was used.

## Sanitizer limitation and reproduction

A full address/undefined run stopped in the original genetics initializer:
`entity/being.c:1599`, `value[loop] |= 1 << loop2`, when loop2 is 31.
The same failure occurs while initializing the non-BRONZE_SIM engine with seed
42, before any actor-registry hook runs. Historical code was not modified to
hide/fix that unrelated issue as part of identity integration.

Reproduce the full-native failure by compiling the real toolkit/script/render/
sim/entity/universe sources plus contract/baseline.c with
`-DCOMMAND_LINE_EXPLICIT -fsanitize=undefined`, no BRONZE_SIM, linking zlib/math/
pthread, and running with seed 42 and `UBSAN_OPTIONS=halt_on_error=1`.

The targeted command is:

```sh
SANITIZE=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 bash bronze/actors/test.sh
```

Its instrumentation split is explicit in the script: ASan covers every linked
native integration source, and UBSan also covers the registry and new harness.
This passed. It is not a claim that the complete historical engine is UBSan
clean. Existing compiler/SDK warnings remain.

## Remaining boundaries

Professions are unassigned, inventories/skills/tasks are initialized schema,
and the aggregate prototype economy still does not run per-actor jobs. Those
belong to cycles 05 onward. The actor registry is world-isolated; the old
file-static prototype sites/economy are not made fully world-owned by this cycle.
Combined persistent actor IDs/allocator/save recovery remain cycle 16. Native
file failure invalidation is fail-closed identity handling, not transactional
native-data rollback. Callers must recreate a context if it cannot be restored.

No live GUI visual acceptance or signed distribution is claimed. Packaged tests
exercise the actual Swift scenario controller and compiled UI source; the native
session harness verifies the selected-ID bridge. Remote CI is configured, not
executed locally in this record.
