# Chapter 15 - Canterbury: Hosting a City on the Native Engine

## A host profile with a different policy owner

`CANTERBURY_SIM` lets an external city host use native ApeSDK bodies, actor
identities and time while supplying its own human routines and movement policy.
It builds on `BRONZE_SIM`; the header rejects Canterbury without Bronze.
The distinction from Chapter 14 is responsibility. Ordinary Bronze adds work
around native island behavior. Canterbury hands the minute to a registered
city callback and returns before that autonomous island loop runs.

The profile is an integration boundary, not a complete historical simulation.
It does not create households, choose a city's calendar, interpret religious
institutions or turn map lines into traversable streets. Those are host-owned
records and policies. The [city-host contract](../bronze/CANTERBURY.md) documents
the SDK seams; this chapter explains why their boundaries matter.

## Read capacity from the actual checkout

This SDK revision defines `LARGE_SIM=27000` and
`CANTERBURY_POPULATION=26999` in
[`universe/universe.h`](../universe/universe.h). Configuration accepts an initial
count from one through 26,999 and leaves a spare native slot. The enclosing
Nosferatu project uses that slot for its native Count, giving 27,000 bodies.
That is a parent-project decision; the SDK flag does not automatically create
a special actor.

Other consumers can pin later SDK revisions. In particular, the Canterbury
Details checkout reviewed for the companion book uses a later profile with
27,000 initial ordinary humans and capacity 30,000. Those limits are not present
in this checkout. A reader should inspect the pinned header and host admission
contract rather than copying a population figure from another project's guide.
Ordinary native and Bronze profiles here retain capacity 256.

The distinction affects binary layout. The simulated group embeds its native
body array, so changing capacity changes structure size and field offsets.
Every linked translation unit must use the same profile definitions. These
are presence-tested preprocessor switches: leaving a macro defined as zero
still enables guarded code. Native body storage is large, and checkpoint staging
can require additional full-world memory; capacity alone is not a memory budget.

## Configuring and owning the native interval

Before `sim_init`, the host calls
`sim_canterbury_configure(population, handler, context)`. A null handler,
out-of-range count or reconfiguration of a running host fails. The setup belongs
to the host's lifecycle, and `sim_close` clears the callback and context so a
new run must configure again. The implementation is in
[`universe/sim.c`](../universe/sim.c).

`sim_cycle` initializes the native population when required, advances native
land time, opens the Bronze work interval, invokes the callback with the actual
group, then closes Canterbury work accounting. The ordinary island weather,
biology, social scans and autonomous movement do not also execute afterward.
The host must supply the human and environmental behavior its scenario requires.
Initialization of native state should not be confused with continued autonomous
island updates.

The city-assigned awake state is exposed by the guarded being getter, avoiding
an island sleep calculation during inspection. Native energy and coordinates
remain shared body fields. A host can maintain fatigue, illness and route
intentions, but those records must not become competing energy or physical
position authorities. A route cursor describes intended progress; reading the
body establishes where the person actually is.

## Sharing the work budget

The city nominates stationary effort through
`bronze_work_canterbury_claim(body, city_reason)` during the open callback.
The bridge checks the native tick, prior sample, needs, location and whether
the actor has already consumed the interval. A successful claim uses the common
work accounting and native energy charge. Host survival, care and travel policy
can refuse work before an effect is committed.

The enclosing begin/close calls ensure that pending duration requests use the
same remainder and diagnostics as other Bronze work. They do not insert a
generic township or DSL dispatcher into the city's callback. The host owns
intention cancellation, city-specific place references and serial publication
of completed effects.

Consider a person assigned to a workshop. Assignment alone earns no production.
The host first establishes that the person reached the admitted entrance,
is available after needs and care, and has the required materials. It claims
the native interval before committing work. If the person walked that minute
or was already used as a caregiver, another claim must not create an extra
minute. This is the same accounting principle that makes the ordinary Bronze
work gate useful, adapted to city-owned scheduling.

## Making identity practical at city scale

Native initialization avoids an all-population uniqueness scan for display
names. Names may repeat; actor IDs and project civil records establish identity.
City-mode actor lookup uses a bounded 65,536-entry hash index and explicit
record-to-native-slot mapping. This avoids repeatedly searching thousands of
records to resolve every person's routine.

Deletion uses tombstones, record reuse advances IDs, compaction updates physical
bindings, and restore rebuilds the index. Reclamation bounds the effect of
long-running churn. Full rebind validates duplicate IDs before replacing
bindings, and rejected reset requests preserve the current registry. These
failure properties are part of the scale work, not optional diagnostics.

The Bronze sidecar size bound rises from 16 MiB to 256 MiB in this profile.
Duplicate detection during restore avoids an all-pairs scan, and capture rejects
inconsistent public ID records. The encoded schema itself is unchanged by that
bound increase. A larger admitted payload does not grant portability across
arbitrary host layouts or make the sidecar a complete city save.

## Saving a city and drawing its map

A host adds households, employment, routes, resources and other obligations
that ordinary native/Bronze saves do not know about. It must validate and publish
all those owners with their matching bodies and registry. Canterbury-specific
`land_canterbury_snapshot/install` provides pointer-free land capture and
installation at a serialized checkpoint boundary. The guarded registry slot-ID
accessor lets a loader inspect staged binding order without activating it.

These seams support an atomic host coordinator. They do not define the host's
schema, migration policy or complete corruption checks. A failed load must
discard staging while retaining the active world; successful publication must
install the matching land, native bodies, registry and city records together.
The enclosing project's persistence documentation defines that additional
contract.

Presentation follows a similar split. `draw_canterbury_map` accepts an indexed
plot target between 512 and 4096 pixels and invokes the existing hybrid draw
boundary. Plot-buffer and world contexts are distinct. The Canterbury path
returns before default island-agent compositing, preventing a second rendering
of camera-transformed city agents at island coordinates. Basemap assets,
palette, camera, picking and city overlays remain host responsibilities.

## Verifying an extension without changing the baseline

The SDK's existing native, actor, hybrid and Bronze contract harnesses remain
relevant. The surrounding host additionally needs population-limit, callback,
movement, work, lifecycle, compaction and complete-save checks. At this revision,
the enclosing project supplies profile checks comparing ordinary layouts and
preprocessed sources with Canterbury disabled. That is stronger evidence than
merely observing that a city executable links.

When adding another city feature, first identify its field owner and its native
minute boundary. Keep generic city content in the host. Add an SDK seam only
when native storage or integration requires it, and guard its
declarations, storage and call sites consistently. Then test successful behavior,
rejection without live-state damage, and the ordinary profile. This preserves
the original engine while allowing a demanding host to reuse its identity,
time, body and work machinery.
