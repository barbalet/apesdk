# Terrain-derived Bronze landscape

Cycle 06 classifies the native low-resolution heightmap on world creation or
native import. The analysis reads ApeSDK heights and uses four-way adjacency
across both wrapped map seams. It uses the maximum tidal waterline as the wet
boundary, including inland basins. A multi-source breadth-first search measures
distance to that boundary in map cells, capped at 255.

Slope is the largest height difference to a cardinal neighbor. Moisture is a
static proxy, computed as max(0, 255 − 4 × water distance − 5 × slope).
Local relief is the height range of the center and its four axial samples two
cells away. Copper and tin use a procedural geology proxy that combines relief,
minimum slope, and separate elevation bands.

| Use | Eligibility and ranking |
| --- | --- |
| Field | Dry, within the coast limit and above minimum moisture. Lower slope and higher moisture improve the score. |
| Woodland | Dry, within a more permissive slope limit and above minimum moisture. Moisture and local relief contribute to the score. |
| Copper | Height 150 up to the tin band, slope at least 2 and sufficient local relief. Relief and slope rank candidates. |
| Tin | Tin band and above, with the same slope and relief constraints as copper. |
| Township | Dry, flat ground near water, within 128 dry-land walking steps of both a field and woodland. |

For each use, the generator tries up to 16 anchors and places up to two patches.
It ranks anchors by suitability, breaks ties by native cell index, and grows
each patch with a deterministic breadth-first traversal. Areas are disjoint,
contain at most 32 cells, and require at least four suitable cells for fields
and deposits or eight for woodland. The map can yield fewer patches where
suitable contiguous terrain is scarce.

The validated scenario's `world` block can set these whole-number thresholds.
Omitted values use the defaults shown here.

| Key | Default | Range |
| --- | ---: | --- |
| `field_slope` | 6 | 0–255 |
| `field_coast` | 48 | 1–254 |
| `field_moisture` | 64 | 0–255 |
| `woodland_slope` | 18 | 0–255 |
| `woodland_moisture` | 45 | 0–255 |
| `ore_relief` | 12 | 1–255 |
| `tin_height` | 205 | 151–255 |

Patch cells define gathering eligibility. Active scenario resource definitions
set capacity and renewal for each matching resource and habitat.

The overlay draws the footprint and an anchor glyph over the native map.
It uses the existing palette and undraw path.

Each Bronze registry owns its landscape, thresholds, stock and prototype
economy. Internal session switches restore these with the matching native
world. Native imports rebuild the landscape from the loaded heightmap.
Paired persistence is cycle 16.


## Township placement, homes and stores

The same wrapped four-way map graph limits walking to cells above the maximum
tidal waterline. Woodland placement favors terrain connected to fields, and
ore placement favors terrain connected to townships. Up to two townships are
ranked by the sum of walking distances to food and woodland, distance to water,
and slope. Township anchors are at least 24 wrapped Manhattan cells apart.
When no site meets the requirements, the world can contain fewer townships.

A township owns distinct home, storage and service place IDs at its anchor.
Production patches have separate place IDs and belong to the township with
the shortest dry-land route to the patch. Isolated patches have no owner.
IDs are scoped to the Bronze world and remain stable across compaction and
internal session switches. A world rebuild retires the old IDs and allocates
new ones. The existing `BRONZE_SIM` hooks own this lifecycle.

Unassigned actors claim the nearest reachable home with capacity in native
population order. Existing residents keep their homes as they travel, and
deaths free slots. An actor whose current location has no reachable vacancy
keeps a zero home and workplace ID and can be assigned on a later native tick.
Cycle 08 assigns workplaces according to the actor’s vocation and available
capacity. Actors can have housing while waiting for work. See
[occupation policy](../actors/PROFESSIONS.md).

The township store is the source for grain, copper, tin, charcoal and tools.
The [task scheduler](../tasks/README.md) gathers through the assigned worker
on their own patch and transfers stock into cargo. Workers deposit at their
township storage entrance. HUD figures total the township stores.

| Optional `world` setting | Default | Range |
| --- | ---: | --- |
| `town_capacity` | 128 | 1–256 residents |
| `town_initial_grain` | 0 | 0–1,000,000 grain per township |
| `patch_initial_per_cell` | 0 | 0–8 stock units per resource cell |

First Slice and Vertical Slice explicitly seed resource patches with starting stock.
Township stores start empty in both bundled scenarios.


## Inventory, claims and renewal

The graphical host supports up to 64 declared resource kinds. Each patch
has a separate stock for every definition that matches its habitat. A
definition’s capacity applies per patch and resource. Explicit initial stock
from `patch_initial_per_cell` is capped by that capacity.

Each observed native day applies one renewal update. Fields and woodland
renew by capacity × scenario renewal rate × mean patch moisture on a 0–1
scale. Minerals use only their declared rate, without the moisture factor.
An omitted or zero mineral rate produces no renewal. Every update caps stock
at capacity. Headless worlds without a loaded program retain the compatibility
landscape defaults.

| Optional `world` setting | Default | Range |
| --- | ---: | --- |
| `carry_capacity` | 32 | 1–1,024 total cargo units |
| `storage_capacity` | 512 | 1–1,000,000 resource and item units per township |

A gather request reserves available stock in native actor processing order.
The reservation does not change patch stock. The final granted work minute
debits the patch and credits the same quantity to cargo. Death, reassignment,
rerouting and cancellation release a claim. A stalled claim expires roughly one native hour after its expected work duration. Deposit debits cargo and
credits storage only at the owned entrance and within capacity. Both transfers
use the requested quantity or fail with their source stock intact.

### Storage and consumption

Personal cargo travels with its owner. Communal storage is available to
township residents at the storage entrance. With a positive
`world_household_capacity`, each actor gets a private locker tied to their
first valid home. The actor can use that locker while resident there and
physically at the home entrance. Other residents have separate lockers.
Use the optional `household` operand with `deposit` or `fetch` to select
private storage.

Communal fetches respect recipe reservations. A positive
`world_ration_daily` limits communal food withdrawals per actor per native
day. A positive `world_food_spoilage` sets the fraction of stored food lost
each day. Spoilage applies to private and unreserved communal food stock.
The three settings default to zero, which disables private storage, ration
limits and spoilage respectively.

Eating uses personal cargo and credits native energy up to the energy deficit.
The native drive cycle handles hunger and fatigue. Native feeding takes
priority over Bronze eating in the same tick, and an unavailable meal yields
no Bronze energy. Consumption, spoilage and losses on death have separate
accounting ledgers. Deposits and fetches transfer stock between owners.
See the [recipe and task rules](../tasks/README.md).

## Spatial township layout (township cycles 1–2)

`bronze_townships_layout` creates a world-owned `BronzeSettlementLayout` after
patch and township generation. `bronze_hybrid_settlement` exposes the current
world's records and placement diagnostics. No mutable process-global occupancy
is retained. The existing home, storage and service IDs and coordinates are
compatibility references; this phase does not change agent jobs or movement.

Each township requests a 3×3 open center, a 2×2 store, up to 32 houses of 2×2
cells (eight housing slots each, with the final house capped to the requested
capacity), and four 2×2 reserved work yards. Work yards are unassigned plots,
not invented workshops or production recipes. Building IDs combine the home
place ID and a bounded ordinal; entrances and working positions belong to
that building. Repeating the same layout preserves IDs and geometry; a new
home ID retires the prior building identities. Paths have bounded records and
surface/width fields but remain empty until township cycle 4.

Deterministic expanding rings search within 14 wrapped map cells of the home
anchor. Every footprint and entrance must remain above the maximum tide;
footprint relief and doorway height differences may not exceed six native
height units. Resource patches, earlier footprints and reserved entrances
cannot be overwritten. A bounded local flood check uses the native walking
limit of 18 height units per edge and rejects solid footprints that disconnect
any placed entrance, including a neighboring township's entrance. Open centers
and yards remain walkable. Land heights are never modified.

`BronzeLayoutReport` distinguishes missing housing slots, store, open center
and work yards. A shortage leaves the requested legacy membership capacity
unchanged; callers must use the placed-housing report when describing physical
housing. No submerged or overlapping buildings are substituted. Existing
scenario files need no new declarations.

These immutable version-1 layouts are derived data. The current disk sidecar
format is unchanged; the first settlement query or Bronze tick after restore
rebuilds the layout from restored terrain and places. Actor session/save tests
compare the complete regenerated layout. Cycle 7 must add versioned persistence
before structures or paths become mutable. Rendering remains the existing
preview until cycle 6, and professional premises bind to agents in cycle 3.

Validation: `bash bronze/hybrid/test.sh` covers dense housing, wrapped seams,
resource exclusion, entrance accessibility, adjacent settlements, small
islands, wet/steep terrain, deterministic IDs and isolated worlds.
`bash bronze/actors/test.sh` covers native sessions, save/Open and the large
default scenario; `make -C bronze contract-test` checks historical behavior.

## Professional premises and paths (township cycles 3–4)

Work plots are now reserved before housing, so houses do not consume all the
level ground needed for production. Their shortage reports remain explicit.
`bronze_premises.inc` binds enabled smith/metalworking, pottery, woodworking
and food preparation occupations to the four available open work yards. The
initial compatibility mapping recognizes lowercase vocation names containing
`smith`/`smelt`/`metal`, `potter`/`pot_`, `carpent`/`woodcarv`/`bowyer`, or
`cook`/`baker`/`brewer`/`food`, respectively. A matching vocation must also have
an actual craft operation with a recipe present in the scenario, including
inside nested conditionals. Other occupations retain their existing policy.
This mapping labels the premises; recipes and inputs remain authored content.

Each active professional yard has a distinct owned service place ID, capacity,
entrance and working position. Service-based professional jobs use that place
as their actual workplace; jobs explicitly requiring a resource patch keep
their patch assignment but use their township's professional yard for shared-
stock crafting. Shared-stock crafting for these professions targets the yard;
recipes explicitly configured to craft at the workplace keep their original
cargo-based behavior. A missing yard produces no matching workplace or craft
target. No recipe, inventory or output is synthesized. Existing worker review,
incumbency, capacity, claim release and death handling apply to these places.
The shared store and central service place move to their actual entrances;
`move_to service` still means the common service area, not the last workshop.

`bronze_paths.inc` builds a deterministic network from the open center (or home
anchor if no center fits) to each building entrance and owned production place.
Search uses four-way wrapped adjacency, dry land above maximum tide, a maximum
height step of 18, and solid home/store footprints as obstacles. Dijkstra costs
are positive: 6 for new ground or 2 for an existing track, plus height change.
This favors shared track segments while retaining distance and slope costs.
Each search visits at most `MAP_AREA` cells. There are at most 128 paths of 256
cells each; unreachable endpoints and length/allocation limits are reported.

Store and production connections request earth roads. A road is two cells wide
only when its complete +Y verge is dry, clear of solid buildings and within
six height units of the centerline, with at most an 18-unit step along the verge.
Otherwise the connection remains a one-cell footpath. A path's `to_place`
identifies a production endpoint; building endpoints additionally carry
`to_building`. Multiple paths may share segments. No water crossings or bridges
are generated. Paths are network data until route preference in cycle 5 and
live rendering in cycle 6.

Dedicated premises use the existing serialized owned-service-place records.
Derived buildings and paths reconstruct from the restored terrain, places and
scenario. A sidecar roundtrip test checks exact place IDs, complete layout and
network reconstruction, and unchanged serialized bytes. Mutable building and
road persistence remains cycle 7 work.

The professional regression follows a smith using the native movement
integrator to the distinct station, verifies input consumption and tool output,
checks incumbent retention, and fills vacancies after death and loss of age
eligibility. Path tests independently check doors, solids, dry ground, slopes,
wrapped steps, shared segments, wide-road verges, unreachable destinations,
length limits, deterministic regeneration and world isolation. The older
recipe transaction fixture explicitly co-locates its counters to isolate its
inventory and teaching assertions; the smith fixture does not teleport arrivals.
Both focused suites support `SANITIZE=1`.

## Agent routes and shared rendering (township cycles 5–6)

The settlement derives constant-time cell indexes for footprints, solid walls,
entrances, tracks and track connections. These belong to the world and rebuild
with its layout after import. Agent searches use positive weighted travel costs:
2 on a track or 6 on other dry ground, plus native height change. Off-path travel
remains legal; long track detours can lose to a shorter cross-country route.
A deterministic heap search visits at most 16,384 cells per request. The existing
two searches per tick, 256-waypoint bound, retry policy and steering/needs limits
remain. Searches and steering reject solid destination cells, including a
cached next step that has become blocked. A being already inside a solid cell
may leave it; no actor is relocated to resolve old-save or initial occupancy.
Native movement, energy accounting, capability checks and observed arrival
remain authoritative. There is no road speed bonus.

Both workspace cameras now read the same settlement indexes and records.
Decorative township house arrays are removed. Live footprints produce round
thatched homes, ridged store roofs, open centers, professional yard fixtures,
entrance thresholds and worn tracks. Roof shading and simple directional ground
shadows distinguish structures; existing terrain depth handles occlusion.
Tracked roads use their actual width and footpaths follow recorded connections.
Ground under tracks retains its interpolated height. Trees are excluded from
building and track cells, while fixed sandy beaches, translucent tides and the
half-height native relief scale are preserved. Selecting a township uses a
32-cell camera setback and higher view to show the settlement instead of
placing the camera among houses. Actor views retain their closer camera.

The only shared C change is workspace snapshot/camera wiring inside the
existing `#ifdef BRONZE_SIM` region of `gui/shared.c`. All model, route and
rendering implementation stays under `bronze/`.

Regression coverage checks useful path preference, rejection of excessive
track detours, solid avoidance, obstructed replanning and unreachable sources.
Rendering tests check roofs, thresholds, track height, vegetation exclusion,
wrapped footprints, frame bounds, viewport sizes, panned centers and moving
actor positions. Actor/session save checks still compare full rebuilt layouts,
including the cell indexes. Sanitized actor and hybrid suites, the native
observable-equivalence contract and the Bronze Catalyst Release build pass.
The live app was inspected with Example Large, site selection and map zoom.

Visual qualification remains limited: this is procedural settlement rendering,
not the reference image's photographic material/detail quality. Interactive
pan/resize attempts were interrupted by changing UI state; their geometry and
frame bounds were checked automatically. Full visual polish and broader live
scenario qualification remain work for the final integrated acceptance cycle.


## Township inspection and schema 2

Cycle 7 adds exact footprint picking and copied township/building inspection
records, optional validated `town_house_capacity` and `town_work_yards` settings,
and schema 2 geometry persistence. Earlier notes about immutable regeneration
are historical: new saves preserve explicit records; only version 1 saves
regenerate geometry. See [save format](../SAVE-FORMAT.md) and the
[cycle 8 qualification report](../validation/townships/README.md) for migration,
validation, measured costs, image evidence and remaining live/visual checks.
