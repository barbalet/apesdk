# Chapter 11 - Seeing, Measuring, And Talking To The Model

Observation in ApeSDK is spread across [`render/`](../render/),
[`gui/`](../gui/), command handling in [`universe/command.c`](../universe/command.c),
console support in [`sim/console.c`](../sim/console.c), and data transfer in
[`universe/transfer.c`](../universe/transfer.c).

This chapter is about evidence. A simulation can look persuasive while still
being misread. ApeSDK gives several ways to see and ask about state, but the
reader must separate visualization from proof.

## Rendering And GUI

`render/graph.c` and `render/graph.h` provide low-level graphing primitives.
`gui/buffer.c`, `gui/draw.c`, `gui/shared.c`, `gui/message.c`, and related
headers provide buffers, drawing, app-shell support, and UI communication.

The renderer is useful for:

- terrain shape;
- social links;
- selected being state;
- weather and overlays;
- brain or status displays;
- visual debugging.

The renderer is not sufficient evidence by itself. If an overlay suggests a
pattern, verify it with command output, logs, transfer data, or deterministic
metrics.

## Commands

The command table in `universe/universe.h` exposes a wide inspection surface.
Commands include reset/open/save/script, run/step/interval, social graph,
braincode, speech, episodic memory, probes, stats, appearance, genome, list,
watch, memory, simulation, and debug-oriented views.

This command layer is one of the best ways to learn the model because it names
what the engine considers observable.

For example:

- `social` or `graph` inspects relationships.
- `episodic` inspects memory.
- `braincode` inspects the compact control program.
- `probes` inspects brain probes.
- `stats` or `status` inspects selected being state.
- `genome` inspects inherited state.
- `memory` inspects simulation memory.

## Speech And Audio

The project includes [`entity/speak.c`](../entity/speak.c) and
[`sim/audio.c`](../sim/audio.c). Speech and audio are not merely presentation
extras. They provide another way for behavior to become inspectable and for
language-like output to be tied back to entity state.

When changing speech-related code, verify both representation and output. A
speech artifact should be traceable to a selected being, state, and version.

## Transfer And Save/Load

`universe/transfer.c` packs and unpacks land, topography, weather, beings,
social entries, episodic memories, and territory state. This is observability
over time. If a state can be saved and reloaded, it can be inspected across
runs, releases, and platforms.

Save/load code must remain conservative. A beautiful visualization is less
valuable if the underlying state cannot be preserved reliably.

## Avoiding Observation Artifacts

A good observation workflow is:

```text
form a visual hypothesis
capture command or log evidence
rerun with the same seed
rerun with visualization disabled where possible
compare state, not just screenshots
```

Potential artifacts include:

- render cadence hiding state changes;
- overlays suggesting causal relationships they do not prove;
- selected-being commands changing what the observer notices;
- save/load differences being mistaken for behavior differences;
- platform UI differences being mistaken for engine differences.

## Practical Observation Bundle

For a serious run, archive:

- ApeSDK version;
- commit or release;
- seed or saved state;
- command transcript;
- selected metrics;
- relevant screenshots or videos;
- script file if used;
- test runner result if code changed.

Observation is strongest when visual and numeric evidence point to the same
explanation.
