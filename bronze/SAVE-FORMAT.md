# Combined Bronze save format proposal

Cycle 16 remains open. This document specifies the intended package boundary
and the checks required before enabling combined Save and Open.

## Implemented envelope

The standalone Catalyst-side codec in `save/bronze_save.c` handles two opaque
payloads. It uses an 80-byte header with an eight-byte versioned signature,
two little-endian 32-bit payload lengths, a SHA-256 digest of the native
payload, and a SHA-256 digest of the concatenated native and Bronze payloads.
The two payloads immediately follow the header. The codec uses CommonCrypto
and has no dependency on the simulation. It does not yet define or validate
the payload records. The design below describes the remaining package work.

## Package

Use a single regular file with a fixed magic, format version, generation ID,
and a section directory. Encode integers as fixed-width little-endian values,
text as length-prefixed UTF-8, and fractional quantities as finite IEEE 754
binary64. The directory gives each section a type, offset, length and SHA-256
digest. A manifest digest covers the directory, version and generation so both
payloads are checked as a unit. Digests detect corruption and accidental
mispairing.

| Section | Contents |
| --- | --- |
| Native snapshot | Native state plus any runtime fields needed for deterministic continuation that the historical export omits |
| Scenario | Scenario identifier, SHA-256 of exact source bytes, compiled schema version and effective policy |
| Identity | Stable actor and place IDs, next allocator IDs, and explicit bindings to records in this native snapshot |
| Actors | Assignment and skill history, cargo, household stock, ration state, task frames, routes, claims and random streams |
| Landscape | Township stores, recipe reservations, patch depletion, renewal schedule and loss and consumption ledgers |
| Clock | Native date and minute, Bronze origin and tick, pending deadlines and renewal markers |

Reference task instructions by compiled vocation, task and statement indexes.
Reference beings by the validated snapshot binding and all Bronze entities by
stable ID. Never serialize pointers, array addresses, padding or C enum widths.

## Write and restore

Capture both layers while the session is paused and the native work tick is
closed. Write the complete file to a temporary sibling, flush and close it,
then atomically replace the destination. A failed write leaves the previous
save available.

Validate the directory before allocating from it. Reject duplicate sections,
overlapping ranges, offset overflow, unknown versions, hash mismatches and
payloads outside supported scenario or runtime limits. Validate unique IDs,
next IDs above existing IDs, population bindings, compiled instruction
references, route cells, clock bounds and reservation totals before activating
either layer. Resolve the exact scenario by content hash. A failed restore
leaves the current paused session intact.

Install the staged native and Bronze worlds together. A successful combined
restore skips fresh-world Bronze initialization. Native-only Open remains a
separate operation and explicitly starts a fresh Bronze economy. A file with
combined-format magic that fails validation must report an error rather than
fall through to native-only Open.

## Acceptance checks

Compare uninterrupted and resumed runs with live claims, pending recipes,
in-progress routes, stored food, and a nonzero allocator history. Compare both
state and subsequent event traces. Exercise truncation, malformed lengths,
cross-generation section substitution, wrong scenario hashes, unsupported
versions and a failed replacement of an existing save. Verify historical
native file bytes and workflows with `BRONZE_SIM` disabled.

## Native continuation audit

The historical being export stores the current speed but only the first
entry in the ten-sample velocity history. Native distance calculations
sum all ten entries. It also stores brain-state constants and leaves the
brain block unimplemented. A deterministic combined snapshot must carry
these fields explicitly. The historical importer writes directly into the
active land and population as it parses. Combined restore therefore needs
a staging boundary before using the importer on package data.

The envelope codec can publish a validated package using a temporary
sibling file and atomic replacement. This operation is still independent
of the simulation's Save and Open commands.
