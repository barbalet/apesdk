# ApeSDK

ApeSDK is an open source artificial life and simulation SDK written primarily in C, with platform toolchains for macOS, iOS, Windows, Linux, and command-line builds. It is the source base behind **Simulated Ape**, a long-running agent-based simulation of autonomous ape-like beings living in a changing landscape with weather, biology, movement, memory, language, social relationships, and braincode-driven behavior.

The project is intended for developers, simulation researchers, artificial life enthusiasts, and people interested in compact C engines that combine environmental simulation, social modeling, scripting, graphics, and portable application front ends. ApeSDK is not a single demo program; it is a collection of engine modules and example toolchains that show how the same simulation core can be embedded in different products.

## What ApeSDK Contains

ApeSDK combines several simulation layers:

- **Artificial life agents** with body state, drives, immune behavior, social graphs, episodic memory, speech, and braincode.
- **Landscape and terrain simulation** with map-space, ape-space, tiling, territory, food, and environmental state.
- **Weather and time simulation** including pressure, wind, lightning, cloud/light values, day/night rendering, and weather-aware behavior.
- **Social simulation** ranging from reactive movement to relationship tracking, internal/external conversations, and shared braincode ideas.
- **ApeScript and command handling** for scripting, inspection, remote-style interaction, and simulation control.
- **Portable data handling** using the ApeSDK object and JSON-style IO layers for loading, saving, testing, and transfer.
- **Rendering and GUI support** for visualizing agents, terrain, weather, braincode, graph views, and platform windows.

The core code is deliberately small and modular. Most of the engine is plain C, which makes it suitable for porting, embedding, testing, and studying without needing a large framework stack.

## Included Toolchain Applications

The projects listed below are not separate external dependencies. They are included inside this repository under `toolchains/` and demonstrate how ApeSDK can be used to build full applications around the shared simulation code.

| Toolchain | Location | Purpose |
| --- | --- | --- |
| **Simulated Ape for macOS** | `toolchains/sim-mac` | Current Swift/macOS application wrapper for the ApeSDK simulation. The Xcode project builds `Simulated Ape.app` while keeping the developer scheme named `sim-mac`. |
| **Simulated War** | `toolchains/war-mac` | A pitched battle and warfare simulation using the same style of C/Swift bridge and ApeSDK drawing/application shell concepts. |
| **Simulated Urban** | `toolchains/urban-mac` | An urban simulation environment for placing organic ape agents into a city-style context. |
| **Simulated Planet** | `toolchains/planet` | A planetary-scale experiment that demonstrates weather, terrain, and ape movement concepts at a broader scale. |

These toolchains are useful examples for developers who want to see how the C simulation modules are connected to native app front ends, especially on macOS using Xcode and Swift.

## Repository Layout

- `toolkit/` - low-level memory, math, vector, file, IO, object, and JSON-style support code.
- `sim/` - landscape, weather, time, audio, tile, console, and versioned simulation constants.
- `entity/` - ape bodies, brains, drives, immune system, food, speech, episodic memory, and social behavior.
- `universe/` - simulation loop, command interface, file transfer, and higher-level world state.
- `script/` - ApeScript parsing and interpretation.
- `render/` - graph and OpenGL-oriented rendering support.
- `gui/` - shared GUI drawing, buffers, messages, and application shell logic.
- `toolchains/` - included application projects built with ApeSDK.
- `test/`, `*/unit/`, and `*/test/` - command-line and unit-style test sources.
- `BUILD.md` - build notes for command-line and test binaries.
- `RELEASE.md` - current macOS Simulated Ape release procedure.

## Building

For a simple command-line build on macOS or Linux:

```bash
./build.sh
```

That script builds the command-line simulation binary `simape` one directory above the repository root. Additional command-line test build scripts are described in `BUILD.md`.

For the current macOS application:

1. Open `toolchains/sim-mac/sim-mac.xcodeproj` in Xcode.
2. Build the `sim-mac` scheme.
3. The product name is `Simulated Ape.app`.

For release packaging of Simulated Ape DMGs and source archives, use `RELEASE.md`.

## Why Developers May Care

ApeSDK is a practical codebase for exploring:

- agent-based modeling in C,
- artificial life simulation,
- autonomous behavior systems,
- social and language simulation,
- environmental simulation with terrain and weather,
- cross-platform native application wrappers,
- compact JSON/object serialization,
- scriptable simulation control,
- legacy-to-modern codebase evolution,
- macOS Swift front ends backed by portable C engines.

It is also a useful reference for developers interested in how a simulation can survive across multiple decades, operating systems, compilers, UI frameworks, and product names while keeping a consistent core model.

## History

ApeSDK began in 1996 and has been under continuous open source development since then. Earlier versions of the simulation were known as **Noble Ape** and **Nervana** before the current **Simulated Ape** naming.

The project history includes use by:

- Apple from 2003 to 2009.
- Intel from 2005 to 2011.

## Contact And Feedback

Contact `barbalet at gmail dot com` for questions, feedback, or collaboration.

Join the [ApeSDK Discord server](https://discord.gg/sCXPWgv) for discussion and coordination.

File bugs, requests, and trackable feedback in [GitHub Issues](https://github.com/barbalet/apesdk/issues). GitHub issue tracking requires signing in with a GitHub account or a Google account so you can receive progress updates on the issues you file.
