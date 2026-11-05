# Spatial townships and walking networks

Status: cycles 1–7 implemented. Cycle 8 functional qualification passes; live
UI and full visual acceptance remain open. Details are recorded below. This phase precedes the Victorian
extension. It develops the settlement visible in the
[workspace reference](ui-redesign/reference/bronzesim-workspace-concept.png).
It does not claim that the reference's visual fidelity has been achieved.

## Intended behavior

A township occupies an actual area of suitable ground. It contains homes,
shared storage, a gathering or exchange area, workshops and outdoor work yards.
A smith has an identifiable workplace with an entrance and working position.
Paths connect those entrances to each other and to fields, woods and mines.
Agents walk those connections, arrive at their assigned work area, and perform
the existing resource-accounted tasks there. The map and Terrain depict the
same buildings and network that the simulation uses.

For this Bronze Age phase, the smith is a copper/bronze metalworker; an iron
blacksmith and Victorian buildings belong to later scenario content.

## Starting baseline

Townships already own stable home, storage and service place IDs, membership,
capacity and inventory. These places currently share a township anchor.
Production patches have separate places. Native dry-land routing already
steers real beings and confirms observed arrival before advancing work.
The workspace draws a fixed cluster of houses and track colors procedurally;
those shapes do not constitute individual buildings or a walking network.

## Scope and ownership

Keep records, layout, routing policy, persistence, render snapshots, tests and
scenario extensions under `bronze/`. Any required shared C integration must be
inside `#ifdef BRONZE_SIM`. Preserve native movement as the authority for actual
position. Preserve historical ApeSDK builds and behavior. Do not change native
terrain elevations to draw a road or teleport an actor to a workplace.

Existing scenario files must load without new mandatory declarations. Generate
compatible layouts deterministically from terrain and existing settlement data.
Use declared occupations and recipes; a decorative workshop must not invent
production capability. Expose unavailable workplaces explicitly.

## Development cycles

1. **Spatial records and constraints — complete.** Add world-owned building, work-yard,
   entrance and path records with stable IDs, township ownership and explicit
   limits. Define footprint occupancy, slope and maximum-tide clearance rules.
   Test wrapped seams, insufficient space, deterministic layout and separate
   simultaneous worlds. Retain existing place IDs as compatibility references.

2. **Township layout — complete.** Place a shared store and central open area, then homes
   and work plots on suitable connected land. Use modest footprints appropriate
   to map resolution; do not impose a regular square grid on every settlement.
   Prevent overlapping buildings and blocked entrances. Report capacity or
   placement shortages rather than silently placing buildings underwater.
   Test tiny islands, steep ground, dense populations and the large default.

3. **Professional premises — complete.** Bind supported occupations to specific workshops
   or work yards: metalworking, pottery, woodworking and food preparation where
   the scenario supplies the relevant tasks. Separate storage, entrance and
   working positions. Preserve assignments across reviews and release capacity
   on death or reassignment. Test an individual smith arriving and completing
   an existing recipe with correct inputs, output and workplace attribution.

4. **Connected footpaths — complete.** Generate deterministic terrain-aware connections
   between building entrances and the central area, then to owned production
   sites. Prefer shared segments. Avoid water, excessive slopes and occupied
   footprints; retain access to doorways. Represent width and surface explicitly:
   narrow worn footpaths and wider earth tracks, initially without bridges.
   Test connectedness, unreachable sites, seams and bounded generation cost.

5. **Route choice and movement — complete.** Extend Bronze route selection with positive
   traversal costs: usable paths are cheaper than comparable off-path ground,
   while slope and total distance still matter. Keep off-path travel possible.
   Respect native movement capabilities and observed arrival; replan blocked
   routes with bounded work. Do not grant speed bonuses until separately
   justified and measured. Test that a useful path is preferred, an excessive
   detour is rejected, and agents neither cross buildings nor jump to targets.

6. **Live map and Terrain — implemented; visual qualification limited.** Replace decorative house clusters and tracks with
   geometry from the authoritative records. Draw distinguishable roofs, yards,
   doors, stores and worn paths in both views; clear vegetation from footprints
   and routes. Keep paths on the interpolated ground and preserve beaches and
   translucent tides. Test alignment during panning, resizing, actor movement
   and site selection. Compare a settlement capture against the reference.

7. **Inspection, scenarios and saves — implemented; live UI checks open.** Show township membership, selected
   building, occupation, assigned worker count and access status. Add optional
   scenario controls with validation and compatible defaults. Version sidecar
   changes; preserve building/path IDs and assignments on save/resume. Document
   old-save migration, rejecting incompatible data transactionally. Test paired
   native/Bronze restore, multi-window use and independent sessions.

8. **Integrated acceptance — functional checks pass; visual/live checks open.** Follow a worker home → workshop → store and a
   gatherer township → resource site → store, using real movement and inventory.
   Run the large default and contrasting small scenarios. Measure generation,
   route-search and frame costs with explicit bounded limits. Build Bronze
   Catalyst and run its focused tests plus historical non-BRONZE_SIM contract
   checks. Record visual evidence and remaining gaps before marking complete.

## First visible milestone

Cycles 1–4 establish actual settlement and path data. Cycle 6 delivers the
visible replacement for decorative townships; it must use those records.
A diagnostic overlay may expose footprints and paths earlier. Full photographic
appearance is not an automatic consequence of these simulation changes.

## Acceptance principle

A rendered doorway is the doorway used by routing. A displayed workshop is the
workplace used by its agent. A drawn path is a usable connection. Geography,
occupation, movement, inventory and visual presentation must agree.

## Cycles 1–2 results (2026-11-05)

Implementation: [spatial records](hybrid/bronze_hybrid.h),
[layout generation](hybrid/bronze_townships.inc), and
[model rules and limits](hybrid/README.md).
The original scenario schema, place IDs, job assignments and native movement
are retained. Version-1 geometry is immutable derived data and regenerates on
import; mutable persistence belongs to cycle 7.

Focused tests cover dense 256-slot housing, both wrapped edges, resource
footprint exclusion, neighboring settlements, doorway reachability, small
islands, all-water and steep ground, stable IDs, retired identities and separate
worlds. The actor/session suite verifies exact layout regeneration after paired
save/Open and loads the large default: each of its two townships places 128/128
housing slots, a store, open center and all four work yards. Historical contract
tests pass with unchanged native observable behavior. The Bronze Catalyst
Release build and sanitized actor/session suite pass. No visible rendering
change is claimed for these cycles.

## Cycles 3–4 results (2026-11-05)

Professional work yards have distinct owned place IDs, working positions,
entrances and scenario-defined worker capacity. Supported occupations require
real craft recipes before receiving premises. Shared-stock craft tasks route
to those stations; explicit resource-workplace and personal-cargo policies are
preserved. Work plots reserve space before housing. Store/service entrances
now refer to actual settlement locations. Missing premises are reported.

A deterministic network connects the open center to buildings and owned
production places. Searches avoid water and solid footprints, honor slope
limits and prefer shared track segments. Clear connections may become earth
roads; narrow connections remain footpaths. Disconnected destinations and
search/path limits are reported without creating phantom connections.

Validation includes a smith arriving through native movement, accounted tool
production, stable assignment, vacancy replacement after death/age change,
and exact sidecar place/layout/network reconstruction. Independent path tests
cover wrapped seams, door alignment, shared segments, wide-road clearance,
unreachable sites, length bounds, repeatability and world isolation. Focused
hybrid and actor suites pass with sanitizers; historical deterministic contract
checks and the Bronze Catalyst Release build pass. See
[implementation rules](hybrid/README.md#professional-premises-and-paths-township-cycles-34).

Route preference and solid-footprint avoidance by agents remain cycle 5. Live
Map/Terrain rendering remains cycle 6. The stored network is not yet a visual
or movement-speed upgrade, and derived geometry remains immutable until the
versioned persistence work in cycle 7.

## Cycles 5–6 results (2026-11-05)

Agents now plan weighted dry-land routes that prefer useful tracks while
retaining off-path travel and distance/slope costs. Solid footprints are excluded
from search and cached-step validation; blocked steps replan within the existing
search/retry budgets. Native steering, movement capabilities, survival priority,
energy and arrival checks remain in charge. No path speed bonus was added.

Map and Terrain now sample the same world-owned footprints, entrances,
professional yard fixtures, paths and widths. Fixed decorative house clusters
are removed. Homes and stores have distinct roofs, entrances have thresholds,
and structures have simple directional shading. Vegetation is cleared from
footprints and tracks; interpolated ground, sandy beaches, translucent tides
and half-height relief remain. Selected townships use a wider camera framing.

Focused route tests cover useful-path preference, excessive detours, solid
avoidance and blocked replanning. Rendering tests cover shared coordinates,
roofs, door thresholds, path heights, vegetation clearance, wrapped seams,
viewport sizes, panning, moving actor positions and frame bounds. Actor and
hybrid suites pass under sanitizers; historical observable-equivalence checks
and the Bronze Catalyst Release build pass. Shared C wiring is contained in
the existing BRONZE_SIM region of gui/shared.c.

Live Example Large inspection confirmed rendered settlements, site selection,
map zoom and the wider township Terrain framing. Interactive pan/resize checks
were interrupted by changing UI state; automated viewport/pan checks provide
coverage but are not a claim of full live UI qualification. The rendering is
procedural and still falls short of the reference's photographic fidelity.
Retain these visual checks and polish as explicit cycle 8 acceptance work.

## Cycles 7–8 results (2026-11-05)

Building footprint selection now shows township membership/housing, building
identity/type, professional area, assigned worker count, capacity and
walking access. Values are copied from the active window's own session.
Map selection highlights the corresponding entrance and Terrain frames it.
The township recenter control and startup-center correction make settlements
accessible without searching from an unrelated actor's position.

Optional `town_house_capacity` (4–16, default 8) and `town_work_yards` (1–4,
default 4) controls validate transactionally. Sidecar schema 2 explicitly stores
buildings and paths and reconstructs only cell indexes. Version 1 remains
readable through documented geometry regeneration. Paired Open validates
structural links and staged native terrain before installation. Tests preserve
changed IDs, migrate legacy records and reject corrupt references/versions.

Native-movement smith and gatherer journeys deliver correctly accounted goods
to separate stores. All four bundled scenarios pass geometry, inspector and
paired save/Open checks. Local generation, route and frame measurements and
actual rendered PNGs are in the [qualification report](validation/townships/README.md).
Sanitized focused suites, historical contract checks and Catalyst Release pass.

Live inspection confirmed township inspector data, recenter controls and the
Save dialog. Repeated changing-state errors and timeouts prevented completion
of live filename entry, Save/Open, restored building inspection and multi-window
pan/resize checks. These checks remain open rather than being inferred from
headless success. Procedural visual fidelity still falls short of the reference;
full visual acceptance remains open as well. Cycles 6/8 must not be described
as fully visually accepted on the strength of the automated tests alone.
