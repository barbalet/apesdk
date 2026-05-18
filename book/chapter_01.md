# Chapter 1 - Why We Build a Synthetic Planet

ApeSDK is a long-running artificial life codebase. Its current public identity
is `Simulated Ape 0.710`, but the design reaches back to Noble Ape and earlier
work on portable simulations of ape-like beings. The project is not just an app.
It is a set of C modules and platform wrappers for building a world, populating
it with embodied beings, observing what happens, and repeating the run under
controlled conditions.

The best first reading is architectural. ApeSDK is a laboratory organized as
source directories:

- [`toolkit/`](../toolkit/) defines the shared data language.
- [`sim/`](../sim/) defines land, weather, time, terrain, audio, and console
  support.
- [`entity/`](../entity/) defines beings: bodies, drives, memory, braincode,
  social state, speech, food, immune state, and skeleton support.
- [`universe/`](../universe/) binds the simulation loop, commands, transfer
  format, and cross-module constants.
- [`script/`](../script/) provides ApeScript for controlled intervention.
- [`render/`](../render/) and [`gui/`](../gui/) expose the world to a human
  observer.
- [`toolchains/`](../toolchains/) contains platform projects, most importantly
  the current macOS `sim-mac` and `planet` projects.

This book treats the code as a hypothesis machine. Every constant, type,
serialization field, update order, and build target is a claim about what must
remain stable for behavior to be interpretable.

## The Current Version Boundary

The current version is declared in [`sim/sim.h`](../sim/sim.h):

```c
#define SHORT_VERSION_NAME "Simulated Ape 0.710 "
#define VERSION_NUMBER 710
```

That is not only release metadata. In simulation work, versioning is part of the
experiment. A saved file, a test run, a screenshot, or a released macOS artifact
should be traceable to the source version that produced it. If the same seed is
run against two different versions, differences may be legitimate model changes
rather than bugs.

A useful reading habit is to ask:

- What is the version?
- What seed or input created the state?
- Which module consumed the state?
- Which observable proves the claim?

## The Simulation As A Layered Claim

ApeSDK is most understandable as a layered claim about life-like behavior:

1. A world must have time, space, weather, terrain, water, and resources.
2. A being must have a body, energy, genetics, state, and constraints.
3. A social being must remember and classify relationships.
4. A decision system must turn internal pressure into action.
5. An observer must be able to inspect the result without changing it.
6. A maintainer must be able to rebuild, test, and release it repeatably.

The code follows this order. `toolkit` comes before everything because all
modules need the same numeric and memory conventions. `sim` gives the world its
clock and landscape. `entity` gives the world agents. `universe` is the bridge
where state is advanced, commands are interpreted, and transfer formats are
defined.

## One Simulation Beat

The real implementation is more detailed, but the conceptual flow looks like
this:

```text
initialize shared memory and simulation state
seed or load land state
create or load the group of beings

for each simulation cycle:
    advance land time, tide, weather, and tile state
    update each being's body, drives, memory, and social context
    execute braincode or scripted control where enabled
    process movement, food, speech, conflict, and reproduction effects
    expose selected state through commands, renderers, or logs
```

This ordering matters. If time moves after behavior, dawn and tide effects may
lag. If social memory updates before an event is stored, learning appears to
skip a cycle. If rendering mutates simulation state, screenshots can become
causal actors instead of observations.

## What Makes It Scientific

ApeSDK is not scientific because it produces complex-looking behavior. It is
scientific when behavior can be traced back to bounded assumptions:

- fixed-width or consciously platform-dependent types;
- named constants for time, map scale, energy, memory, and social limits;
- explicit event enums and affect values;
- testable parser and interpreter behavior;
- deterministic tests in the project-level runner;
- release artifacts tied to a version and source archive.

The current repository reinforces this through [`test.sh`](../test.sh), which
acts as the canonical deterministic C test runner. Older smoke tests and manual
diagnostics still exist, but the current CI boundary is narrower and more
reliable: deterministic unit-style tests for C modules, plus macOS Xcode builds
for the active app projects.

## Current Reading Path

For a first code pass, read in this order:

1. [`README.md`](../README.md) for the public project overview.
2. [`sim/sim.h`](../sim/sim.h) for version, map, time, weather, braincode, and
   file signatures.
3. [`toolkit/toolkit.h`](../toolkit/toolkit.h) for the common type system.
4. [`universe/universe.h`](../universe/universe.h) for cross-module constants,
   command surface, social sizes, preferences, drives, and structures.
5. [`universe/sim.c`](../universe/sim.c) and [`universe/loop.c`](../universe/loop.c)
   for lifecycle and loop behavior.
6. [`entity/entity.h`](../entity/entity.h) and the focused `entity/*.c` files
   for being-level mechanisms.
7. [`test.sh`](../test.sh) for what the project currently promises to run
   repeatably.
8. [`RELEASE.md`](../RELEASE.md) for how a version becomes a release.

## Questions To Carry Forward

- Which claims are encoded as constants, and which are only described in prose?
- Which modules can be tested without a GUI?
- Which behaviors depend on random seed lineage?
- Where does an event become memory?
- Where does a body state become a decision?
- Which outputs are observations, and which outputs feed back into the model?

The rest of the book answers those questions one layer at a time.
