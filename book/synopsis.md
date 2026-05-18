# ApeSDK - A Reader's Map

This book is a guided introduction to ApeSDK as it exists in the current
repository. It is not a replacement for the source. It is a companion for
reading the source without losing the larger design: a portable artificial life
simulation built from explicit modules for landscape, weather, bodies, social
memory, language, scripting, rendering, and command-line inspection.

The current engine version is `Simulated Ape 0.710`, defined in
[`sim/sim.h`](../sim/sim.h). The release checklist lives in
[`RELEASE.md`](../RELEASE.md), and the deterministic project test runner is
[`test.sh`](../test.sh).

Historical context comes from the [Noble Ape site](https://nobleape.com/), whose
2012 talk transcript describes Noble Ape as the predecessor name for ApeSDK and
records many of the same design threads that remain visible in the current
source: landscape, weather, drives, social graphs, braincode, ApeScript, and
open-source research collaboration.

## How To Read This Book

Each chapter has three jobs:

- name the source files that matter;
- explain the modeling assumption behind those files;
- give a practical way to test or extend the topic without confusing artifacts
  for behavior.

The chapters are ordered from the lowest shared assumptions to the highest-level
workflows. Read them in order the first time. Later, use them as a map back into
specific directories.

## Contents

1. [Why We Build a Synthetic Planet](chapter_01.md)
2. [A Shared Language for Life](chapter_02.md)
3. [Time, Coordinates, and the Clockwork of a World](chapter_03.md)
4. [Weather, Topography, and Resource Geography](chapter_04.md)
5. [Designing the Being: Genes, Energy, and Physical Rule Sets](chapter_05.md)
6. [The Social Brain Before the Brain](chapter_06.md)
7. [Drives, Emotion, and Decision Pressure](chapter_07.md)
8. [A Neural Assembly Language: Braincode](chapter_08.md)
9. [Learning by Event: Episodic Memory and Meaning](chapter_09.md)
10. [Language as Computation, Not Narrative](chapter_10.md)
11. [Seeing, Measuring, and Talking to the Model](chapter_11.md)
12. [Building, Extending, and Stress-Testing a Living System](chapter_12.md)

## Current Repository Anchors

The important directories are:

- [`toolkit/`](../toolkit/) - shared C types, math, memory, IO, file/object, and
  JSON helpers.
- [`sim/`](../sim/) - landscape, spacetime, weather, terrain tiles, audio, and
  console support.
- [`entity/`](../entity/) - being state, bodies, drives, food, braincode,
  episodic memory, speech, immune behavior, and social behavior.
- [`universe/`](../universe/) - simulation loop, command interface, save/load
  transfer, and cross-module constants.
- [`script/`](../script/) - ApeScript parser and interpreter.
- [`render/`](../render/) and [`gui/`](../gui/) - low-level graphing, buffers,
  drawing, messages, and shared UI support.
- [`toolchains/`](../toolchains/) - current macOS app projects for `sim-mac`
  and `planet`, plus legacy wrappers whose full builds may require sibling
  source trees outside this checkout.

The important workflows are:

- Build the command-line simulation with [`build.sh`](../build.sh).
- Run deterministic C tests with [`test.sh`](../test.sh).
- Use GitHub Actions as the current CI boundary: Linux runs deterministic C
  tests, while macOS builds the `sim-mac` and `planet` Xcode projects.
- Follow [`RELEASE.md`](../RELEASE.md) for the macOS release process and
  versioned artifacts.

## A Note On Determinism

Simulation code can produce fascinating output while still being useless as a
test. This book treats deterministic behavior as an engineering requirement.
When a section proposes a protocol, it assumes fixed seeds, repeatable inputs,
and a named version. The current project-level runner intentionally includes
only deterministic tests.
