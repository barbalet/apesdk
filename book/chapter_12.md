# Chapter 12 - Building, Extending, And Stress-Testing A Living System

The final layer is engineering practice. ApeSDK is valuable only if it can be
built, tested, changed, and released without turning every interesting result
into a mystery.

The current workflow is documented across [`README.md`](../README.md),
[`BUILD.md`](../BUILD.md), [`test.sh`](../test.sh),
[`.github/workflows/ci.yml`](../.github/workflows/ci.yml), and
[`RELEASE.md`](../RELEASE.md).

## Noble Ape Stewardship Context

The [Noble Ape 2012 talk](https://nobleape.com/) describes the project as open
source and as a meeting point for students, engineers, artificial-life
researchers, and people bringing biological or cognitive literature to a working
codebase. That is a useful frame for maintaining ApeSDK today. The goal is not
only to keep code compiling. It is to keep the project approachable enough that
outside interest can become useful contribution.

The transcript also separates scientific usefulness from merely organic or
interesting simulation behavior. That distinction is exactly what this chapter's
testing, release, and documentation discipline is for. ApeSDK remains valuable
when experiments, artifacts, and claims can be reproduced.

## Archive Evidence

- [The Mind of the Noble Ape in Three Simulations](https://www.apesdk.com/doc/Barbalet_OriginOfMind.pdf)
  says the software was available in "source and executable form free of
  charge." That line supports this chapter's maintenance argument: ApeSDK's
  value depends on inspectable source, repeatable artifacts, and public
  scrutiny.
- The [Ape Brain article](https://www.apesdk.com/doc/Barbalet_ApeBrain.pdf)
  describes artificial life as "an applied philosophy." That phrase explains why
  maintenance, tests, documentation, and release discipline are not separate
  from the project's intellectual purpose.
- The [IGDA IPR SIG White Paper](https://www.apesdk.com/doc/IGDA_IPRights_WhitePaper.pdf)
  and early interviews are historical context for stewardship and public-facing
  project identity, not current build instructions.
- The auto-captioned
  [command-line build video](https://www.youtube.com/watch?v=cYnLhDEwTWs&t=45s)
  walks through `build.sh`, the
  [Swift and C update](https://www.youtube.com/watch?v=3c-FTrRt-cw&t=1s)
  says the Mac version now uses "Swift and C," and the
  [long-term development update](https://www.youtube.com/watch?v=jD2AiFiu1FE&t=0s)
  describes a "long-term development time frame." Those references support this
  chapter's emphasis on build clarity, platform boundaries, and release
  expectations.

## Building

The command-line build is [`build.sh`](../build.sh). It compiles the C modules
from `toolkit`, `script`, `render`, `sim`, `entity`, and `universe`, then links
the long-term command-line executable.

The repository also contains Windows build scripts and Xcode-based macOS
toolchains. The active macOS CI build matrix currently includes:

- `toolchains/sim-mac/sim-mac.xcodeproj` with scheme `sim-mac`;
- `toolchains/war-mac/war-mac.xcodeproj` with scheme `war-mac`;
- `toolchains/urban-mac/urban-mac.xcodeproj` with scheme `urban-mac`;
- `toolchains/planet/planet.xcodeproj` with scheme `planet`.

The `war-mac` and `urban-mac` wrappers now use local bridge implementations for
their CI builds, so they can compile from a clean checkout without requiring
sibling source trees.

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

## Release 0.711 And Later

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

## Maintenance As Model Stewardship

Maintaining ApeSDK is not only software maintenance. It is model stewardship.
Every change asks whether the simulation remains understandable. A faster
terrain routine, a new command, a rewritten parser check, or a different build
target can all affect what users believe about the model.

This is why engineering process belongs in the final chapter. The project is not
finished when it compiles. It is finished when a change can be built, tested,
explained, observed, and released without misleading future readers.

Model stewardship means caring about:

- deterministic tests;
- accurate documentation;
- clear release notes;
- portable build assumptions;
- stable file formats;
- honest toolchain claims;
- reproducible examples;
- removal of flaky checks from required paths.

These practices are not overhead. They are what make the scientific parts of
the project credible.

## The Difference Between Exploratory And Canonical Tests

ApeSDK can support many kinds of tests, but not all tests should be canonical.
Exploratory tests are allowed to be broad, visual, long-running, and sometimes
messy. They help a maintainer discover behavior. Canonical tests must be
repeatable. They guard the project.

The current `test.sh` runner makes that distinction. It runs deterministic C
tests that should pass every commit. Older smoke tests and manual diagnostics
can still be useful, but they are not automatically suitable for CI.

This distinction prevents a common failure mode: a test suite that fails so
often for environmental reasons that maintainers stop trusting it. A flaky
required test trains people to ignore red builds. That is worse than leaving the
test out until it can be stabilized.

The rule is simple:

```text
exploratory tests may discover instability
canonical tests must not be instability
```

## What Determinism Means Here

Determinism does not mean the model has no randomness. It means the same inputs
produce the same outputs under the tested conditions. A fixed seed can still
generate rich behavior. A deterministic test can still exercise random-looking
systems.

For ApeSDK, deterministic tests should control:

- seed;
- initial state;
- module boundaries;
- expected outputs;
- file inputs;
- time limits;
- platform assumptions.

If a test depends on wall-clock timing, GUI availability, network state, file
ordering, unbounded randomness, or a long-run emergent outcome without a stable
assertion, it needs more work before it becomes canonical.

## Build Scripts And Platform Honesty

Build scripts are documentation in executable form. `build.sh` says which C
modules form the command-line target. Xcode projects say which app wrappers are
active. CI says which targets are expected to build automatically.

Documentation should match those facts. If `sim-mac`, `war-mac`, `urban-mac`,
and `planet` build in CI, say that. Optimistic documentation creates wasted time.
Honest documentation creates trust.

Platform honesty is especially important in old, portable codebases. A wrapper
may be valuable even if it is not fully buildable in the current checkout. The
right wording preserves both facts: it exists, and it has dependencies.

## Release As A Controlled Ritual

The release checklist is a ritual in the best engineering sense. It turns a
large set of details into a repeatable sequence. Update version constants. Build
artifacts. Package DMGs. Create a source zip. Verify checksums. Publish the
GitHub release. Download and smoke-test the result.

Each step protects a different failure mode:

- version constants prevent identity drift;
- clean builds catch local-only dependencies;
- checksums protect artifact integrity;
- source archives preserve historical source;
- smoke tests catch packaging mistakes;
- release notes communicate what changed.

Skipping a step may not fail immediately. That is why checklists exist. They
protect against rare but expensive mistakes.

## Documentation As Part Of The Build

This book now belongs to the project documentation surface. It should be updated
when the project changes in ways that alter a reader's mental model.

Examples:

- a new persistent field belongs in the save/load discussion;
- a new event belongs in the memory chapter;
- a new drive belongs in the drives chapter;
- a new toolchain belongs in the build chapter;
- a new deterministic test family belongs in the testing discussion;
- a new scripting feature belongs in the ApeScript chapter;
- a new renderer capability belongs in the observability chapter.

Documentation drift is a real bug in a research codebase. If the book says one
thing and the code does another, new readers will learn the wrong system.

## A Change Impact Template

For meaningful changes, write a short impact note before implementation:

```text
Change:
Primary module:
Secondary modules touched:
State added or changed:
Persistence impact:
Command/render impact:
Deterministic test plan:
Release note needed:
Book/docs update needed:
```

This template catches hidden coupling early. It also helps future maintainers
understand why a change was made.

## Avoiding Accidental Architecture

Long-lived projects often accumulate accidental architecture: a quick helper
becomes a public API, a test fixture becomes a dependency, a platform workaround
becomes the only way to run something. ApeSDK is old enough that this risk is
real.

The cure is periodic naming. If a pattern is important, name it and document it.
If it is accidental, simplify it before it becomes load-bearing. If it is legacy
but still useful, label its current status honestly.

This is especially important around toolchains and tests. A file existing in the
tree is not the same as a fully supported path. A test binary existing in the
tree is not the same as a canonical test. A project wrapper existing in the tree
is not the same as a CI guarantee.

## The Maintainer's Final Question

Before finishing any change, ask:

```text
Could another person reproduce my result from the repository, the docs, and the
release artifacts?
```

If the answer is no, improve the tests, docs, commands, or release notes until
the answer becomes yes. That is how a living simulation remains alive without
becoming unknowable.

## Case Study: Promoting A Manual Test

Suppose a manual smoke test catches a real bug. The temptation is to add it
directly to `test.sh`. That may be right, but only after the test becomes
deterministic.

Promotion requires asking:

- Does it build from a clean checkout?
- Does it require a GUI?
- Does it rely on wall-clock timing?
- Does it write uncontrolled files?
- Does it use a fixed seed?
- Does it assert stable facts?
- Does it clean up after itself?
- Does it fail with a useful message?

If the answers are good, the test can join the canonical runner. If not, keep it
as a manual diagnostic and document when to use it.

This process respects both kinds of tests. Manual tests are allowed to be
exploratory. Canonical tests must be boringly reliable.

## Case Study: A Release With The Wrong Version

Imagine building a macOS artifact after changing code but forgetting to update
`SHORT_VERSION_NAME`, `VERSION_NUMBER`, or Xcode marketing version. The app may
run perfectly, but the release identity is wrong. Users cannot tell which model
they are running. Saved files and support reports become harder to interpret.

This is why version updates belong early in the release checklist. A release is
not only a compiled product. It is a named state of the model.

The release process should always verify:

- source version constants;
- app marketing version;
- artifact names;
- checksums;
- source archive;
- GitHub release tag;
- smoke-tested downloaded artifacts.

The boring steps are the ones that protect future archaeology.

## Dependency Boundaries

The current repository includes wrappers whose full builds may require sibling
source trees. That is not a failure; it is a dependency boundary. The failure
would be documenting those wrappers as if the checkout alone fully supports
them.

Dependency boundaries should be explicit:

- what is in this repository;
- what is expected outside it;
- what CI verifies;
- what a contributor can build immediately;
- what requires extra setup.

Clear boundaries reduce frustration and make onboarding easier.

## How Documentation Should Evolve

This book should not freeze the project. It should evolve with it. But it should
evolve at the same level of care as code.

When changing documentation:

- preserve source accuracy;
- avoid overclaiming support;
- name current version-specific facts;
- update links when files move;
- distinguish active CI from legacy tooling;
- keep examples deterministic where possible;
- remove draft-process language from reader-facing chapters.

Documentation should help a new reader become effective faster. If a paragraph
does not do that, revise it.

## The Sustainable Loop

A sustainable ApeSDK change follows this loop:

```text
read current source
make the smallest coherent change
add or update deterministic tests
run canonical tests
inspect behavior if needed
update docs or book
prepare release notes when user-facing
```

This loop is conservative, but it is not timid. It allows bold model work by
making each step recoverable and explainable.

## Chapter Twelve Checklist

Before closing the book, make sure you can:

- build the command-line target;
- run `./test.sh`;
- explain why flaky tests are excluded from canonical CI;
- identify active macOS CI toolchains;
- follow the release checklist;
- document dependency boundaries honestly;
- write a change impact note;
- update this book when the model changes.

If you can do those things, you are no longer only reading ApeSDK. You are ready
to care for it.

## Open Design Questions

The project can improve its engineering surface in several ways: more
deterministic tests, clearer fixture data, stronger release automation, better
toolchain documentation, richer command transcripts, and more explicit
compatibility notes. Each improvement should preserve the project's practical
spirit. Automation is useful when it makes releases safer. It is harmful when it
hides steps maintainers need to understand.

One important question is how broad CI should become. More CI coverage is good
only when the checks are reliable. A flaky GUI or long-run scenario check should
not block every commit until it is made deterministic. Another question is how
to handle legacy projects. The best approach is usually honest status labels,
not deletion and not overclaiming.

## Practical Exercise

Take a hypothetical change, such as adding a new event or resource, and fill out
the impact template from this chapter. Then identify which chapters of this book
would need updates.

For a new event, the answer might include:

- chapter 5 if it involves body or inventory;
- chapter 6 if it affects relationships;
- chapter 9 for memory semantics;
- chapter 11 for command visibility;
- chapter 12 for tests and release notes.

This exercise shows that documentation is not separate from architecture. A
well-placed feature has a well-placed explanation.

## Maintenance Vocabulary

Use these terms consistently:

- canonical test means required deterministic check;
- exploratory test means useful but not required;
- release artifact means distributed build or source archive;
- toolchain means build wrapper or project;
- dependency boundary means required code outside the current checkout;
- documentation drift means docs no longer match source;
- stewardship means keeping the model understandable over time.

With this vocabulary, future work can be discussed clearly and maintained with
confidence.

## Reader Lab: From Change To Release Note

Choose one small hypothetical change, such as adjusting a food energy value.
Write the release-note sentence before making the change:

```text
Changed X so that Y; expected effect is Z; deterministic coverage is T.
```

If the sentence is hard to write, the change may not be clear enough. This is a
simple but powerful discipline. Release notes force intention into plain
language.

After writing the sentence, identify the matching test and documentation update.
For a food energy change, the test may be a body or drive test, and the
documentation may touch chapters 4, 5, and 7. For a script parser change, the
test belongs in script unit tests and the documentation belongs in chapter 10.

## Closing Principle

ApeSDK is a living project because it keeps its mechanisms reachable. The code
does not ask readers to accept behavior as magic. It gives them constants,
structures, commands, tests, scripts, renderers, and release artifacts. The work
of maintenance is to keep those paths open.

When future contributors add to the world, they should leave the next reader
with the same gift: a way to understand what changed and why.

## Final Reader Questions

At the end of any substantial change, ask:

- Did the deterministic tests pass?
- Did the change touch persistence, commands, scripts, or rendering?
- Did documentation change where reader expectations changed?
- Is the release process affected?
- Are toolchain claims still accurate?
- Could a future maintainer reproduce the result?

These questions are simple enough to feel repetitive. That is their value. A
long-lived artificial life project needs habits that survive excitement,
fatigue, and changing contributors. The habits keep the world understandable.
