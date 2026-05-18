# Chapter 12 - Building, Extending, And Stress-Testing A Living System

The final layer is engineering practice. ApeSDK is valuable only if it can be
built, tested, changed, and released without turning every interesting result
into a mystery.

The current workflow is documented across [`README.md`](../README.md),
[`BUILD.md`](../BUILD.md), [`test.sh`](../test.sh),
[`.github/workflows/ci.yml`](../.github/workflows/ci.yml), and
[`RELEASE.md`](../RELEASE.md).

## Building

The command-line build is [`build.sh`](../build.sh). It compiles the C modules
from `toolkit`, `script`, `render`, `sim`, `entity`, and `universe`, then links
the long-term command-line executable.

The repository also contains Windows build scripts and Xcode-based macOS
toolchains. The active macOS CI build matrix currently includes:

- `toolchains/sim-mac/sim-mac.xcodeproj` with scheme `sim-mac`;
- `toolchains/planet/planet.xcodeproj` with scheme `planet`.

The `war-mac` and `urban-mac` wrappers are present, but they depend on sibling
source trees outside this checkout. Do not describe them as fully CI-covered by
this repository unless those dependencies are present and the workflow has been
changed.

## Deterministic Testing

The canonical deterministic runner is:

```sh
./test.sh
```

It currently builds and runs:

- `toolkit/unit`;
- `script/unit`;
- `sim/unit`;
- `render/unit`;
- `entity/test` for immune behavior;
- `entity/unit`;
- `universe/unit`.

The runner also cleans generated objects and coverage files on exit. Its purpose
is not to run every historical diagnostic. Its purpose is to run tests that are
stable enough to trust on every commit.

That distinction matters. A flaky test is worse than no test when it trains
maintainers to ignore failures. Exploratory smoke tests can live outside the
canonical runner until they are deterministic.

## CI

The current GitHub Actions workflow has two jobs:

- Linux C build and deterministic tests through `./test.sh`.
- macOS Swift/Xcode builds for `sim-mac` and `planet`.

This CI boundary is intentionally modest. It should grow only when new tests are
repeatable. If a test depends on timing, GUI availability, random output,
external files, or platform-specific state, stabilize it before making it a
required check.

## Release 0.710 And Later

The release process in [`RELEASE.md`](../RELEASE.md) is centered on the macOS
`Simulated Ape.app` build from `toolchains/sim-mac`.

Important release steps include:

- update `SHORT_VERSION_NAME` and `VERSION_NUMBER` in `sim/sim.h`;
- update the Xcode marketing version;
- build Apple Silicon and Intel artifacts;
- create DMGs;
- create a source zip;
- verify SHA-256 checksums;
- publish the GitHub release;
- smoke-test the downloaded artifacts.

Versioning is a scientific boundary as well as a distribution boundary. A
released build should make it clear which source and test state produced it.

## Extension Discipline

When adding a feature, identify the smallest honest scope:

- toolkit representation;
- simulation world field;
- being body or state;
- social graph;
- episodic event;
- drive or preference;
- braincode instruction;
- ApeScript surface;
- render or command visibility;
- transfer/save/load field;
- unit test.

Then update the matching chapter of this book if the concept is important enough
that a new reader needs it.

## Change Checklist

Before merging a meaningful engine change:

1. Run `./test.sh`.
2. Add deterministic tests for new behavior.
3. Avoid adding flaky long-run or GUI-dependent tests to the canonical runner.
4. Confirm command or render visibility if the feature must be inspected.
5. Confirm transfer behavior if state must persist.
6. Update README, release notes, docs, or this book where reader expectations
   change.
7. Keep toolchain claims accurate.

## Final Synthesis

ApeSDK survives because it separates imagination from evidence. The project can
model a synthetic planet, but it also gives maintainers ways to ask precise
questions:

- What changed?
- Where is it stored?
- When is it updated?
- Which test proves it?
- Which observation explains it?
- Which release contains it?

That discipline is what lets the code remain a living artificial life system
rather than a collection of interesting but unrepeatable demos.
