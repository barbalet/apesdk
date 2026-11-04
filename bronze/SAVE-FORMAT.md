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
