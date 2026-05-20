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

## Noble Ape Historical Thread

The [Noble Ape 2012 talk](https://nobleape.com/) is useful because it places
the current ApeSDK in its older intellectual setting. The transcript frames
Noble Ape as a simulation begun in 1996 from interests in physics, philosophy,
mind, consciousness, social interaction, and artificial societies. That origin
still matters. ApeSDK is not merely a modernization of source files; it is the
current form of a long-running attempt to make mind-like and society-like
behavior inspectable in code.

The same reference also makes clear why this project should be read as a
framework rather than a single monolithic model. The discussion repeatedly
returns to replaceable components, interacting simulations, and the ability to
remove or reintroduce subsystems for testing. That maps cleanly onto the current
directory structure. `toolkit`, `sim`, `entity`, `universe`, `script`, `render`,
and `gui` are not just folders. They are the modern boundaries of the same
modular philosophy.

## Archive Evidence

- The [Original Manuals](https://www.apesdk.com/doc/Barbalet_OriginalManuals.pdf)
  give the origin story in deliberately stark form: "On 12 June 1996, there was
  nothing." In this chapter, that line supports the idea that ApeSDK should be
  read historically, as a project that grew from a written and simulated
  research notebook rather than from a finished product brief.
- The [Mind of the Noble Ape in Three Simulations](https://www.apesdk.com/doc/Barbalet_OriginOfMind.pdf)
  describes the mind model as "three distinct yet interoperating simulated
  models." That phrase anchors this chapter's claim that ApeSDK is not a single
  AI module, but a layered system in which biological, cognitive, social, and
  narrative mechanisms meet.
- The [IEEE article](https://www.apesdk.com/doc/Barbalet_IEEE.pdf) says the
  early work centered on "the simulation core and the simulation graphics." That
  explains why this chapter treats the current source tree as both model and
  presentation boundary.
- The auto-captioned
  [ApeSDK Talk](https://www.youtube.com/watch?v=qdqe0mCgeUE&t=210s) gives the
  oral-history anchor "June 13th 1996" and describes the present project as a
  hobby that moved through several names before ApeSDK. That supports this
  chapter's emphasis on continuity rather than a single clean product origin.

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

## Reading The Architecture As A Promise

The easiest mistake to make with ApeSDK is to treat it like a normal application
with a simulation inside it. That makes the app window feel primary and the
source feel like implementation detail. For this project, the order is the other
way around. The source is the durable artifact. The windows, command-line
interfaces, release bundles, and rendered views are ways of meeting the model.

That distinction matters because the model is old enough to have lived through
several presentation layers. A project that began before many current UI
frameworks existed cannot depend on one user interface as its source of truth.
The central promise has to be lower: a portable C model, careful shared types,
and a group of toolchains that can wrap the model without owning it.

When you read a directory, ask what promise it makes:

- `toolkit` promises that common representations have one home.
- `sim` promises that the world can advance without knowing about a specific
  app shell.
- `entity` promises that beings have internal state rich enough to explain
  behavior.
- `universe` promises that those pieces are advanced and queried in a coherent
  order.
- `script` promises that experiments can be written without editing the engine
  for every question.
- `render` and `gui` promise that a human can inspect state, but not that the
  screen is the only truth.

This is why a book about ApeSDK should not begin with "what button does what."
It should begin with "what assumptions are stable enough that a button can ask
about them?"

## Why A Synthetic Planet Is Different From A Demo

A demo tries to make something happen quickly. A synthetic planet tries to make
many things happen for knowable reasons. Those two goals often fight each other.
A demo can hard-code a pleasing social interaction. A synthetic planet has to
explain how hunger, age, visibility, memory, terrain, drive pressure, and
available partners combined to make an interaction possible.

In ApeSDK, the phrase "planet" is therefore not only about scale. It is about
coupling. A planet is a place where small rules do not stay isolated:

- light changes energy opportunities;
- water and tide change movement routes;
- movement changes fatigue;
- fatigue changes sex drive;
- sex drive changes mate goals;
- mate goals create episodic memory;
- episodic memory changes later social decisions;
- social decisions change where beings spend time;
- where beings spend time changes what they experience next.

This chain is what makes the system worth reading. No single term has to be
complicated to create complexity. The complexity comes from explicit pieces
feeding one another over time.

When someone new opens the project, they may look first for "the AI." There is
no single file that deserves that title. The intelligence-like behavior is
distributed across body constraints, social memory, drives, braincode, scripts,
terrain, and event history. That is a healthier design for an artificial life
system because it makes behavior less theatrical and more explainable.

## Versioning As Scientific Hygiene

Most software projects use version numbers to communicate release state to
users. ApeSDK needs that, but it also needs version numbers to protect
interpretation. When a saved state or released artifact says `0.710`, it anchors
that artifact to a particular set of constants, file formats, tests, and build
assumptions.

Suppose a long run from one version shows a stable social pattern and a later
run does not. Without version metadata, the difference is just mystery. With
version metadata, the question becomes tractable. Did a drive threshold change?
Did a memory replacement rule change? Did weather initialization change? Did the
transfer format change? Did CI begin testing a different boundary?

This is why the release process belongs in a book about the model. Releases are
not just packaging. They are the point where a mutable laboratory becomes a
named historical claim. Anyone using a released build should be able to find the
source version, the release checklist, the tests expected to pass, and the
platforms the release intended to support.

The same idea applies at smaller scale. If you are running your own experiment,
write down:

- version number;
- commit hash if available;
- seed or input file;
- command or UI path used to run the experiment;
- scripts loaded;
- tests run before the experiment;
- any local patches.

That sounds bureaucratic until the first time a surprising behavior appears.
Then those notes become the difference between discovery and confusion.

## Initialization Is Part Of The Model

Artificial life systems are especially sensitive to initialization. A creature's
first position, genome, energy, braincode, and local terrain can shape hundreds
of later cycles. A world seed is not just a random number; it is the root of a
lineage.

The initialization path in ApeSDK contains several kinds of setup:

- memory allocation and buffer preparation;
- land genetics and terrain generation;
- weather or tile initialization;
- group creation;
- selected being setup;
- transfer loading when a saved state is used;
- script or command state.

The distinction between a fresh simulation and a loaded simulation is important.
A fresh simulation begins with generated beings and generated land state. A
loaded simulation carries history. Those are different experimental classes. It
is not fair to compare them as if they only differ by elapsed time.

A reader should therefore treat setup functions with the same seriousness as
behavior functions. Initialization determines what counts as "before." If
"before" is not controlled, "after" is not meaningful.

## Evidence Chains

This book uses the phrase "evidence chain" because it is useful for reading
ApeSDK. A claim about behavior should move through a chain like this:

```text
source assumption
state transition
observable state
repeatable test or run
interpretation
```

For example, "hunger suppresses mating" is not only a sentence. It has a source
assumption in `drives_hunger()`, where hunger can reduce sex drive. It has a
state transition in the being's drive array. It has observable state through
commands or tests. It has repeatability if fixed seeds and deterministic unit
tests exercise the path. Only then should it become interpretation.

Many misunderstandings come from skipping the middle of that chain. A person
sees fewer mating events in a run and concludes that hunger did it. Maybe. But
the same output could come from age distribution, pregnancy, fatigue, social
graph attraction, terrain separation, or memory. A strong ApeSDK reading traces
the full path.

## What A New Contributor Should Do First

A good first week with ApeSDK is not spent trying to change the most visible
thing. It is spent building a mental map:

1. Run the deterministic tests.
2. Build the command-line target.
3. Read the version constants.
4. Follow one simple value, such as energy, from definition to drive to command
   output.
5. Follow one event, such as grooming or eating, from action to episodic memory.
6. Inspect one social graph entry.
7. Read the release checklist.

This path gives the reader a sense of the whole system without requiring them to
understand every subsystem at once. It also trains the right instinct: follow
state. In an artificial life codebase, state is the story.

## Designing Future Work From The First Chapter

The chapter's final lesson is restraint. ApeSDK can invite grand ideas: richer
language, larger worlds, more detailed bodies, more elaborate cognition, better
visualization, new platforms. All of those may be worthwhile. But the project
has lasted because ideas enter through explicit surfaces.

Before adding a feature, decide which kind of feature it is:

- representation;
- world field;
- being state;
- event;
- drive;
- memory;
- control language;
- command;
- renderer;
- platform wrapper;
- release process.

Then make the smallest change that proves the concept. A synthetic planet grows
best when each new piece can be named, tested, observed, and removed if it turns
out to be wrong.

## A Worked Example: Following A Single Claim

Take a simple claim: "A being crossed water because it was pursuing food." This
sounds plain, but in ApeSDK it crosses several layers. To make the claim
credible, a reader needs to follow a path through the system.

First, identify the being and its state. Was it awake? Was it hungry? What was
its energy? Was it already moving? Was it carrying something? Was it in a social
goal state that could explain movement independently of food?

Second, identify the world state. What was the land height at the being's
location? What was the water level? Was tide changing? Was the food source a
terrain-derived resource, an inventory item, or an event side effect? Which map
scale was used to decide that?

Third, identify the decision pressure. Did hunger rise before movement? Did
fatigue rise after swimming? Did social drive or mate-seeking compete with the
food path? Did an episodic memory influence the route?

Fourth, identify observation. Was this seen visually, reported by a command, or
inferred from saved state? If it was visual, what numeric or command output
confirms it?

By the time those questions are answered, the sentence becomes stronger:

```text
At version 0.710, fixed seed S, being A was hungry at time T, moved from
ape-space position P through water-level condition W toward a resource field R;
fatigue increased after swimming and no mate/social goal was active.
```

That is the difference between watching behavior and understanding behavior.

## The Reader As A Scientist, Not A Tourist

This book is meant to make the reader active. Do not only read the chapters and
then trust them. Use them as prompts for source reading. When a chapter names a
constant, search for it. When it names a structure, inspect where it is written
and read. When it names a command, ask what state the command exposes.

ApeSDK rewards this style because the code is full of small named mechanisms.
There are constants for time, memory size, social range, energy, weather, map
scale, braincode, and drives. Those names are invitations. They let a reader
turn a behavior back into code.

The best readers will keep a notebook with three columns:

- observation;
- source path;
- test or command that could confirm it.

For example:

```text
Observation: fewer mate events during low-food runs
Source path: drives_hunger, drives_sex, food energy constants
Confirmation: drive logs, EVENT_SEEK_MATE count, energy trace
```

This habit prevents one of the classic simulation mistakes: mistaking a good
story for a good explanation.

## How The Chapters Fit Together

The sequence of this book is intentional. The toolkit chapter comes before time
and terrain because every later system relies on shared representation. Time and
coordinates come before weather and resources because environment needs a clock
and a map. The being chapter comes before social memory because social behavior
requires bodies. Drives come before braincode because internal pressure explains
why control matters. Episodic memory comes before scripting because experiments
often manipulate memory-like behavior. Observability comes before building and
release because a model must be seen clearly before it can be maintained
confidently.

This order also reflects a debugging order. When a result is surprising, inspect
lower layers first. A social oddity may come from a body constant. A body oddity
may come from terrain. A terrain oddity may come from coordinate conversion. A
coordinate oddity may come from a primitive assumption.

## Chapter One Checklist

Before moving on, make sure you can answer:

- Where is the current version defined?
- Which directory owns shared types?
- Which directory owns land and weather?
- Which directory owns being-level state?
- Which directory binds the simulation loop and command layer?
- Which test runner defines the current deterministic boundary?
- Which release document defines the public artifact process?

If those answers are clear, the rest of the project has a map.
