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

Patch cells define gathering eligibility and support eight stock units per cell.
Fields and woodland renew up to five units per observed day change, capped at
capacity. Ore stocks deplete without renewal.
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
Prototype gathering reduces patch stock and credits the owner’s store when
an assigned worker is on their own patch. Tool crafting awaits task dispatch.
HUD figures are derived totals across the stores. The existing hourly
prototype transfers goods directly to storage. Actor cargo and transport
remain later cycles.

| Optional `world` setting | Default | Range |
| --- | ---: | --- |
| `town_capacity` | 128 | 1–256 residents |
| `town_initial_grain` | 0 | 0–1,000,000 grain per township |
| `patch_initial_per_cell` | 0 | 0–8 stock units per resource cell |

First Slice and Vertical Slice explicitly start resource patches at capacity.
Township stores start empty in both bundled scenarios.
