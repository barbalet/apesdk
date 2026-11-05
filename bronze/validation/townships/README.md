# Township cycles 7–8 qualification

Run date: 2026-11-05. All work is Bronze-owned; shared hooks in `shared.h`,
`gui/shared.c`, `universe/universe.h` and `universe/transfer.c` are inside
existing `BRONZE_SIM` regions. No GitHub workflow or intentional wait was added.

## Functional results

- An adult smith walks from home to an assigned professional yard, consumes
  two copper, makes a tool and walks to the separate store to deposit it.
- A gatherer walks from home to the assigned field, gathers two grain and
  returns to the store. Patch decrease equals store increase; cargo/reservations
  clear. Both journeys integrate native movement without teleporting arrivals.
- Route tests cover useful tracks, excessive detours, solids and blocked steps.
- Schema 2 preserves changed building/path IDs, geometry, jobs and inventories.
  Legacy schema 1 migrates. Invalid owners/versions and wet geometry fail checks.
- Example Large, First Slice, Vertical Slice and Bronze Age Full pass geometry
  validation, building picking/inspection and paired native/Bronze save/Open.
- Actor/session and hybrid suites pass with sanitizers. The historical contract
  suite passes native/native_hide/bronze observable equivalence for both seeds.
- Bronze Mac Catalyst Release builds successfully.

The new optional whole-number `world` controls are `town_house_capacity`
(default 8, range 4–16) and `town_work_yards` (default 4, range 1–4).
Omitted settings retain prior behavior. Fractional/out-of-range values reject
the staged scenario without replacing its predecessor. Housing can still be
limited by the 32-house cap or terrain; reports show the shortage.

## Local measurements

Single samples of CPU time from the local arm64 macOS C test build (`-O1`),
without sanitizers. Map: 768×512. Terrain: 768×432. These are not sustained FPS
or worst-case benchmarks. Layout time includes premises, paths and cell indexes;
route time is one local home-to-store search.

| Scenario | Layout ms | Route ms | Map ms | Terrain ms | Buildings | Paths |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Example Large | 1.880 | 0.047 | 27.424 | 14.592 | 44 | 49 |
| First Slice | 1.660 | 0.029 | 28.586 | 13.960 | 41 | 45 |
| Vertical Slice | 1.663 | 0.007 | 28.554 | 13.859 | 41 | 45 |
| Bronze Age Full | 1.264 | 0.016 | 29.002 | 15.707 | 34 | 38 |

Bounds remain: two agent route searches per tick, 16,384 visited cells per
agent search, 256 waypoints, 80 buildings, 128 paths and 256 cells per path.
Path generation searches at most `MAP_AREA` cells for each bounded endpoint.
Render dimensions remain bounded by the existing workspace limit of 768.

## Visual evidence and limits

The previous map, Terrain and agent PNG captures have been removed because
they predate the separate building/mine geometry and current partially
transparent water rendering. Inspect the running app for the current output;
the historical qualification results below remain as recorded.

Live Catalyst inspection showed township membership, housing and access data
beside the actor's occupation/task, the new township recenter control, and the
Save dialog. Later input/AX calls repeatedly timed out or reported changed UI
state, including after closing the superseded disposable test instance. Process
sampling showed normal event-loop/rendering work, not a demonstrated deadlock.

**Still open:** complete the live filename entry → Save → Open sequence, inspect
an individual building after restore, and verify pan/resize with multiple live
windows. Automated tests cover the corresponding records, paired restoration,
independent sessions, viewport sizes, panned centers and moving actor positions;
they do not substitute for these live UI checks.

The reference image remains substantially more naturalistic. This implementation
has data-aligned footprints, doors, yards, paths, thatched/ridged roofs and simple
shadows. Materials, building silhouettes and landscaping still need visual
polish. Functional acceptance passes; full visual acceptance is not closed.

## Reproduction

```
bash bronze/hybrid/test.sh
bash bronze/actors/test.sh
SANITIZE=1 bash bronze/hybrid/test.sh
SANITIZE=1 bash bronze/actors/test.sh
make -C bronze contract-test
```

Set `BRONZE_TOWNSHIP_EVIDENCE` to an existing temporary directory when running
the actor suite to export fresh raw PPM frames and a timing log from the current renderer.
