# Combined Bronze saves

In Bronze Mac Catalyst, choose **Save Bronze World…** to export a combined
save. To resume, load the scenario used to create the save and choose
**Open Bronze World…**. A successful open starts paused. Choose **Resume**
when ready. **Import Native World…** imports the historical native format
and initializes a fresh Bronze economy.

## Package layout

The 80-byte envelope carries a versioned signature, two little-endian
32-bit payload lengths, a SHA-256 digest of the native payload, and a
SHA-256 digest of both payloads together. The native payload starts with
the historical transfer length and selected-being index, then contains
the historical transfer and the native continuation record. The Bronze
sidecar follows the native payload.

The native continuation record preserves movement history, brain bytes,
being and parent names, social classifications, immune state, the remains
ring, and weather continuation state. Version 2 saves still open with the
social and immune fields available in their historical native payload.
The Bronze sidecar preserves the actor and place allocators, bindings in
native population order, work and task state, routes, inventories,
reservations, patches, township stock, and clock state. Task references
resolve against the active compiled scenario. The app compares the
SHA-256 digest of the bundled scenario source before staging the sidecar.

## Restore and checks

Combined Open checks the envelope and stages the native land, population,
and Bronze registry in separate storage. The native record and Bronze
sidecar must agree on population size and tick, and the continuation
records must match the native population order. Installation occurs after
these checks and resumes the saved state without a Bronze reset.

The actor and session suite publishes a combined file to disk, reopens it,
and compares the resulting package after three native cycles with an
uninterrupted run. It also checks that a truncated file, a wrong scenario
digest, and an unsupported continuation version leave the active world in
place. The hybrid suite restores a nested recipe with reserved inputs,
completes it, and compares the resulting native and Bronze records. The
full scenario and long-duration qualification remain part of cycle 18.

## Township sidecar schema 2 (2026-11-05)

Sidecar version 2 introduced explicit township geometry. The combined package envelope and native
continuation formats are unchanged. After the historical landscape fields,
version 2 stores the township housing/work-yard policy, layout version,
building/path counts, building IDs and footprints, entrances and work positions,
professional types, compatible place IDs, capacities, path IDs/endpoints,
surfaces/widths/cells, and placement/access reports. Actor records follow.
Cell indexes for picking, solids, doors and tracks are reconstructed, not stored.

The reader accepts versions 1 and 2. Version 1 receives the compatible defaults
of eight slots per house and four work yards; its geometry rebuilds after the
native world has been installed. Its existing actor/place IDs and assignments
are retained. Legacy geometry was never serialized and cannot preserve manual
building/path edits. Version 2 preserves explicit geometry and identities without
regenerating the settlement. Older binaries do not support version 2 saves.

Staging bounds every count, coordinate and enum before dereferencing records.
It rejects unknown schema versions, overlapping/disconnected footprints,
missing/foreign owners and place references, duplicate IDs, invalid work
positions, solid entrances, malformed path endpoints and non-adjacent path
steps. Combined Open checks dry footprints, ground relief, dry paths, slopes
and road verges against the staged native terrain before replacing either
active world. A rejected save leaves the current world installed.

Regression tests alter building/path IDs before capture to demonstrate actual
persistence, compare complete records and serialized roundtrips, reject an
unsupported version and corrupted path owner, migrate a version 1 fixture,
and check independent shared-session inspector results. All four bundled
scenarios pass paired save/Open with byte-identical resumed packages.

## Starting occupation schema 3

Current writes use sidecar version 3. It extends schema 2 with one 32-bit
selected-occupation ID per actor, immediately after its identity/vocation/task/
instruction fields. The no-role sentinel is `UINT32_MAX`; other IDs must be
within the loaded roster. This field does not authorize work or modify skills.

Readers accept versions 1, 2 and 3. Versions 1/2 initialize the label from the
saved detailed vocation; subsequent profession reviews fill any remaining
starting-label quota from enabled roles. All schema 2 geometry and validation
remain unchanged. Schema 3 preserves labels through paired Save/Open without
requiring an authored scenario change. Older binaries cannot read version 3.
