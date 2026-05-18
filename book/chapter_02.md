# Chapter 2 - A Shared Language For Life

The foundation of ApeSDK is [`toolkit/`](../toolkit/). This directory supplies
the shared language used by the rest of the engine: numeric types, vectors,
areas, strings, memory helpers, math helpers, file IO, object handling, and JSON
support.

Artificial life code often fails quietly when two modules disagree about what a
number means. ApeSDK reduces that risk by putting common assumptions in
[`toolkit/toolkit.h`](../toolkit/toolkit.h) and by reusing those structures
across simulation, entities, rendering, transfer, and scripting.

## Shared Types

Important types include:

- `n_byte`, `n_byte2`, and `n_byte4` for byte-sized storage.
- `n_int` and `n_uint` for native integer work.
- `n_double` for floating-point math.
- `n_vect2` and `n_vect3` for 2D and 3D vectors.
- `n_area2`, `n_line`, and `n_quad` for geometric regions.
- `n_spacetime` for a date, time, and location bundle.
- `n_rgba` and `n_rgba32` for color data.

The same `n_vect2` can pass through movement, terrain sampling, rendering, and
command output. That reuse is deliberate. It keeps conversions visible and lets
readers audit coordinate assumptions.

## Bounded Data Is A Model Choice

ApeSDK favors bounded structures in many places. That is not merely old C style.
For simulation, bounded memory creates explicit limits:

- a being can only carry so much state;
- social graphs have a maximum size;
- episodic memory has a maximum number of records;
- feature frequencies are normalized rather than allowed to grow forever;
- braincode has a fixed instruction memory.

These limits make failure modes inspectable. A social relationship disappears
because a bounded feature set replaced it, not because an unbounded container
silently expanded until behavior became impossible to reason about.

## Math And Determinism

The math layer is important because many higher-level claims are built from it:
randomness, interpolation, vector distance, angles, and basic geometry. When a
terrain gradient or social range changes, the source of the change often lives
in a small shared helper.

For deterministic experiments, treat shared helpers as part of the experimental
surface. A "small" math change can alter:

- food discovery;
- line of sight;
- water and terrain sampling;
- social range checks;
- renderer output;
- test fixture stability.

## Files, Objects, And JSON

`toolkit/file.c`, `toolkit/io.c`, and `toolkit/object.c` support the data layer.
The repository also contains JSON examples under [`toolkit/json/`](../toolkit/json/).
These are useful for understanding how structured data is represented outside
the running simulation.

When adding a new persisted concept, ask three questions:

1. Does it need a stable file representation?
2. Does it need object or JSON helpers?
3. Can older data fail clearly if the shape changes?

The transfer system in [`universe/transfer.c`](../universe/transfer.c) depends
on that kind of discipline. File compatibility is not a separate concern from
simulation science; it determines whether historical states can still be read
and compared.

## Assertions And Error Surfaces

`toolkit.h` defines assertion and error hooks such as `NA_ASSERT` and shared
string helpers. In debug builds, assertions can catch incorrect assumptions
close to the source. In release-style builds, disabled assertions make it even
more important that deterministic tests cover boundary behavior.

Good toolkit tests should cover:

- construction of common types;
- boundary values for math helpers;
- memory allocation and list behavior;
- object parsing and serialization;
- platform-sensitive assumptions.

The current deterministic runner includes [`toolkit/unit`](../toolkit/unit/).
Legacy diagnostics under [`toolkit/test`](../toolkit/test/) are still useful
for manual investigation, but they are not the current project-wide CI contract.

## Practical Extension Rule

When you introduce a new primitive, do not start with the behavior that uses it.
Start with the representation:

```text
define the type or helper
define valid ranges
add construction and boundary tests
use it in two modules
verify no stale direct assumptions remain
```

That order keeps the shared language honest. A simulation cannot be more
reproducible than its smallest shared representation.
