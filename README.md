# ApeSDK

ApeSDK is an open source artificial life and simulation SDK written primarily in
C. It is the source base behind **Simulated Ape**, a long-running agent-based
simulation of autonomous ape-like beings living in a changing landscape with
weather, biology, movement, memory, language, social relationships, immune
behavior, and braincode-driven control.

The current engine version is **Simulated Ape 0.710**, defined in
[`sim/sim.h`](sim/sim.h). The project continues the older **Noble Ape** and
**Nervana** lineage while keeping the simulation core portable across command
line builds, native app wrappers, and test harnesses.

ApeSDK is intended for developers, artificial life researchers, simulation
enthusiasts, and people interested in compact C engines that combine
environmental simulation, social modeling, scripting, graphics, and portable
front ends. It is not a single demo program. It is a collection of engine
modules, documentation, tests, and toolchains around a shared simulation model.

## Current Focus

- **Portable artificial life core.**
  The core C modules remain the authority for terrain, weather, beings, drives,
  memory, social behavior, scripts, commands, rendering support, and transfer.

- **Deterministic testing.**
  [`test.sh`](test.sh) is the canonical deterministic test runner. It builds and
  runs the current unit-style checks for `toolkit`, `script`, `sim`, `render`,
  `entity`, and `universe`.

- **Current macOS app builds.**
  GitHub Actions currently builds the `sim-mac` and `planet` Xcode projects.
  Some older wrappers remain in the tree as useful reference material but need
  sibling source trees outside this checkout.

- **Reader documentation.**
  [`book/`](book/) contains a long-form guide to understanding the current
  ApeSDK source tree, including the Noble Ape lineage and the 0.710 testing and
  release workflow.

- **Future interface proposal.**
  [`june13/`](june13/) is a working proposal area for a possible **Simulated
  Universe** interface focused on richer interaction with the simulated apes while
  preserving the ApeSDK simulation as the source of truth.

## What ApeSDK Contains

- **Artificial life agents** with body state, drives, immune state, social
  graphs, episodic memory, speech, inventory, genetics, and braincode.
- **Landscape and terrain simulation** with map-space, ape-space, high-resolution
  terrain, tiling, water, tide, territory, food, and environmental state.
- **Weather and time simulation** including pressure, wind, lightning,
  cloud/light values, day/night phases, and weather-aware rendering.
- **Social simulation** with relationship tracking, attraction, familiarity,
  bounded feature sets, episodic events, and memory-derived behavior.
- **ApeScript and command handling** for controlled intervention, inspection,
  scripted experiments, and command-line simulation control.
- **Portable data handling** using ApeSDK file, object, JSON-style IO, and
  transfer layers for loading, saving, testing, and structured state movement.
- **Rendering and GUI support** for visualizing agents, terrain, weather,
  braincode, graph views, buffers, messages, and platform windows.

The code is deliberately small and modular. Most of the engine is plain C, which
makes it suitable for porting, embedding, deterministic testing, and study
without needing a large framework stack.

## Repository Layout

- [`toolkit/`](toolkit/) - low-level memory, math, vector, file, IO, object, and
  JSON-style support code.
- [`sim/`](sim/) - landscape, weather, time, audio, tile, console, and versioned
  simulation constants.
- [`entity/`](entity/) - ape bodies, brains, drives, immune system, food, speech,
  skeleton, episodic memory, and social behavior.
- [`universe/`](universe/) - simulation loop, command interface, file transfer,
  and higher-level world state.
- [`script/`](script/) - ApeScript parsing and interpretation.
- [`render/`](render/) - graph and OpenGL-oriented rendering support.
- [`gui/`](gui/) - shared GUI drawing, buffers, messages, tutorials, and
  application shell logic.
- [`toolchains/`](toolchains/) - application projects built around ApeSDK.
- [`book/`](book/) - long-form guide to the current ApeSDK architecture.
- [`june13/`](june13/) - proposal notes for a possible Simulated Universe interface.
- [`test/`](test/), `*/unit/`, and `*/test/` - command-line, legacy, and current
  deterministic test sources.
- [`BUILD.md`](BUILD.md) - build notes for command-line and test binaries.
- [`RELEASE.md`](RELEASE.md) - current macOS Simulated Ape release procedure.

## Toolchains

| Toolchain | Location | Current status |
| --- | --- | --- |
| **Simulated Ape for macOS** | [`toolchains/sim-mac`](toolchains/sim-mac/) | Active macOS Swift/Xcode wrapper. CI builds the `sim-mac` scheme, producing `Simulated Ape.app`. |
| **Simulated Planet** | [`toolchains/planet`](toolchains/planet/) | Active macOS Xcode project in CI. Demonstrates broader weather, terrain, and movement concepts. |
| **Simulated War** | [`toolchains/war-mac`](toolchains/war-mac/) | Legacy/reference wrapper. It depends on sibling source trees outside this checkout. |
| **Simulated Urban** | [`toolchains/urban-mac`](toolchains/urban-mac/) | Legacy/reference wrapper. It depends on sibling source trees outside this checkout. |

The toolchains show how the C simulation modules connect to native front ends,
especially on macOS through Swift, Xcode projects, and C bridging layers.

## Building

For a simple command-line build on macOS or Linux:

```bash
./build.sh
```

That script builds the command-line simulation binary `simape` one directory
above the repository root.

For the current macOS application:

1. Open [`toolchains/sim-mac/sim-mac.xcodeproj`](toolchains/sim-mac/sim-mac.xcodeproj)
   in Xcode.
2. Build the `sim-mac` scheme.
3. The product name is `Simulated Ape.app`.

For release packaging of Simulated Ape DMGs and source archives, follow
[`RELEASE.md`](RELEASE.md).

## Testing

Run the deterministic project test suite with:

```bash
./test.sh
```

The canonical runner currently covers:

- `toolkit/unit`
- `script/unit`
- `sim/unit`
- `render/unit`
- `entity/test` immune checks
- `entity/unit`
- `universe/unit`

Older smoke tests and manual diagnostics remain in the tree, but they are not
the required CI boundary unless they are made deterministic and added to
`test.sh`.

## Documentation

- Start with [`book/synopsis.md`](book/synopsis.md) for the long-form reader's
  map.
- Use [`book/README.md`](book/README.md) as the book entry point.
- Use [`BUILD.md`](BUILD.md) for build notes.
- Use [`RELEASE.md`](RELEASE.md) for the release checklist.
- Use [`june13/README.md`](june13/README.md) for the draft Simulated Universe
  interface proposal.

## Why Developers May Care

ApeSDK is a practical codebase for exploring:

- agent-based modeling in C;
- artificial life simulation;
- autonomous behavior systems;
- social and language simulation;
- episodic memory and social graph modeling;
- environmental simulation with terrain, tide, weather, and resources;
- scriptable simulation control through ApeScript;
- compact JSON/object serialization;
- deterministic test boundaries for simulation code;
- macOS Swift front ends backed by portable C engines;
- legacy-to-modern codebase evolution.

It is also a useful reference for developers interested in how a simulation can
survive across multiple decades, operating systems, compilers, UI frameworks,
and product names while keeping a consistent core model.

## History

ApeSDK began in 1996 and has been under continuous open source development since
then. Earlier versions of the simulation were known as **Noble Ape** and
**Nervana** before the current **Simulated Ape** naming.

The project history includes use by:

- Apple from 2003 to 2009.
- Intel from 2005 to 2011.

Historical context is available at [nobleape.com](https://nobleape.com/), which
includes a 2012 talk transcript describing many ideas still visible in the
current source: generated landscapes, weather, drives, social graphs, episodic
memory, braincode, ApeScript, and open-source research collaboration.

## Contact And Feedback

Contact `barbalet at gmail dot com` for questions, feedback, or collaboration.

Join the [ApeSDK Discord server](https://discord.gg/sCXPWgv) for discussion and
coordination.

File bugs, requests, and trackable feedback in
[GitHub Issues](https://github.com/barbalet/apesdk/issues). GitHub issue
tracking requires signing in with a GitHub account or a Google account so you can
receive progress updates on the issues you file.
