# World-owned actor identity (cycle 04)

`bronze_actors.{h,c}` links one Bronze record to each live native ApeSDK being.
It never creates a second being population, changes native being/save layouts,
or derives identity from a pointer, array index, name, date of birth or genetics.

## Identity and ownership

A `BronzeActorId` is an unsigned 64-bit number scoped to its registry/world
session. Zero means none. Allocation starts at one and is monotonic; death,
compaction, reordering, native import and New Simulation never reuse an ID in
that registry. A fresh registry is a fresh namespace. Determinism means the
same initial population and ordered lifecycle history produce the same IDs;
restarting a world in the same session intentionally allocates fresh IDs.
The registry generation increments when state is retired/reset. Counter
exhaustion fails rather than wrapping. The native `LARGE_SIM` capacity bounds
live records; the authoring limit of a staged scenario is not a promise of a
larger executable native population.

Use `(owning registry, actor ID)` as the runtime handle. A registry owns fixed
record storage and temporary native-slot bindings. The slot is a locator, not
the identity. Live record addresses survive compaction but are borrowed: they
become invalid when that actor dies, resets or its registry is destroyed. A
reused record address does not imply a reused ID. Do not persist pointers.

`find(registry,id)` can inspect metadata in an inactive world. `resolve` and
`for_being` return no native binding for inactive/invalid worlds, even though
all shared sessions use the same native array address. Activation invalidates
physical bindings until the matching native snapshot has been restored and
rebound. A failed restore exposes no stale native mapping.

The private shared_core_context owns one registry per session/default context,
under `#ifdef BRONZE_SIM`. Internal native snapshots preserve population order;
the corresponding in-memory registry retains that order and its allocator.
Session restoration rebinds it after the paired native restore. This is not a
persistent Bronze save format. Direct headless engine use has a separately
owned fallback registry, released by sim_close.

## Lifecycle hooks and original-build split

Every shared-core addition is inside `#ifdef BRONZE_SIM`:

| Site | Effect |
| --- | --- |
| `universe/sim.c`, population setup | Reset registry after native beings are initialized, before their first ordinary step. |
| `universe/loop.c`, successful native birth | Register initialized newborn before invoking the existing external birth callback; native code then publishes its new population count. |
| `universe/loop.c`, death removal | Retire record independently of whether an external death callback exists. Existing callback delivery is unchanged. |
| `universe/loop.c`, compaction copy | Move the slot binding together with the native being, without copying/reassigning the actor record. |
| `universe/sim.c`, KIND_LOAD_FILE | Native-only imports allocate fresh identities immediately; deferred first-cycle load setup does not reset them twice. |
| `universe/command.c`, failed native transfer | Invalidate sidecars because the legacy reader may have partially changed native state. Do not attach old IDs to partially replaced beings. |
| `gui/shared.c`, context activation/restore/destruction | Own, select, rebind and free each world's registry alongside its native context. |
| `shared.h` / `gui/shared.c` | Read-only selected-actor-ID session bridge. |

Native external callbacks are neither replaced nor wrapped. They can observe
the newborn's assigned ID, and death notification still occurs before removal.
ID registration itself consumes no native random numbers and changes no
native biology, positions, needs or behavior. Current loop execution is serial;
registry APIs require the same serialized lifecycle boundary and do not add a
new parallel simulation/threading model.

A complete arbitrary permutation must carry its ordered actor IDs through
`bronze_actors_bind_order` at a safe boundary. Unknown/duplicate IDs are rejected
without changing bindings. The native compaction path has its own direct move
hook. Future code that changes native array membership/order must call the
corresponding lifecycle API; raw byte replacement is not a supported identity
operation.

Death releases all owned claim records once, frees dynamic actor data and
clears other live actors' mentor/apprentice references to the retired ID.
Reset/invalidation/destruction release all remaining claims. Physical bindings
are invalidated before reset release callbacks. Release callbacks must be
non-reentrant and refer to the owning world's stores, not whichever native
world is globally active. No real resource reservation system exists yet;
the release interface and its lifecycle guarantees are ready for cycle 11.

Missing native files do not retire IDs when no read occurred (the old native
return code does not distinguish this reliably). Malformed native imports
retire IDs and require a new/reloaded world, or a fresh context if it cannot be
reactivated. They do not provide native-data rollback. Transactional combined
native/Bronze persistence belongs to cycle 16. Invalid .bronze selection is a
separate staged-loader path and never requests a reset.

## Sidecar schema

These fields are initialized now; later cycles populate their domain behavior.
All list pointers/counts are owned by the record, initially NULL/zero. Lists
must not alias native state or each other. There is no second hunger, fatigue,
position, biological age or simulation clock in the sidecar.

| Field | Representation / initial value | Owning development cycle |
| --- | --- | --- |
| Actor identity | Registry-scoped uint64 ID | 04 |
| Vocation | Config-scoped uint32 ID; BRONZE_KIND_NONE | 08 |
| Skills | List of kind/value records; empty; scale defined in cycle 08 | 08/15 |
| Home / workplace | World place IDs; zero unassigned | 07/08 |
| Resource / item inventory | Separate kind/amount lists; empty | 11/12 |
| Task / instruction | Config task ID BRONZE_KIND_NONE; instruction zero | 09 |
| Remaining duration | uint64 ticks, zero; timing rules defined in cycle 05 | 05/09 |
| Reservations | Place/kind/amount list; empty; release handler owned by world | 11 |
| Assignment history | Tick/from-vocation/to-vocation list; empty | 08/15 |
| Mentor / apprentice | Actor IDs, zero; cleared on retirement | 15 |
| Registry metadata | Next ID, generation, active count and temporary bindings | 04, persisted in 16 |

A vocation ID is not the fixed `entity/profession.h` enum. Config IDs need the
matching scenario fingerprint when later persisted. The allocator and actor
records are not written to historical native saves in this cycle.

The canonical native adapter now offers a registry-backed view factory. The
v1 neutral scenario contract still uses 32-bit actor IDs, so the factory
explicitly rejects IDs at/above SCENARIO_ID_NONE rather than truncating a
64-bit registry identity. Manual pilot fixture IDs remain test-only; production
binding must use the active world. The neutral contract's NONE value differs
from the registry's zero sentinel. Factory views are borrowed per operation;
re-resolve the stable ID after native membership/order changes or a world switch.
Never retain a native pointer as the persistent actor handle.

## Inspection, tests and remaining limits

The overview shows the selected native being's actor ID through the guarded
session bridge. Vocation is still unassigned; schema fields do not implement
profession assignment, inventories, services or a task scheduler. The aggregate
prototype economy remains separate, including its previously documented
file-static site/economy limitation for independent worlds. This cycle isolates
actor records, not all future Bronze world state.

Cycle 05 adds registry-owned native clock observations and per-actor work
diagnostics/budget state. Duration intentions can now progress through guarded
native arbitration without implementing the DSL occupation scheduler. These
fields follow the same identity, world-switch and retirement rules described
above; see [work policy](../work/README.md).

```sh
bash bronze/actors/test.sh
SANITIZE=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 bash bronze/actors/test.sh
bash test.sh
make -C bronze contract-test
```

The actor harness uses real native initialization, pregnancy/birth, death
removal/compaction and shared sessions. It tests duplicate native identities,
metadata/inventory retention, explicit permutation, retirement, link/claim
cleanup, reset/teardown, selection, two-world restoration including native
positions, inactive-world access refusal, native imports and malformed-import
invalidation/recovery. It is included in make test and therefore the normal CI
suite; source/preprocessor/link audits and existing snapshots remain in the
contract gate.

Session tests enable Linux `MALLOC_PERTURB_` and macOS `MallocScribble` so
unwritten allocation tails contain nonzero bytes. Bronze snapshots bound their
readable size to the writer's actual byte count before storage; parsing spare
capacity previously caused repeated session restores to fail on Linux CI.

Sanitizer mode uses ASan across the full native integration and UBSan on the
new registry and harness. Full-engine UBSan independently fails without
BRONZE_SIM at the existing signed shift in entity/being.c:1599; it has not been
silently suppressed or fixed in historical code. See the cycle 04 results.
