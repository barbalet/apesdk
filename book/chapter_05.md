# Chapter 5 - Designing The Being: Genes, Energy, And Physical Rule Sets

The being layer turns the world from landscape into life. Its main anchors are
[`entity/entity.h`](../entity/entity.h), [`entity/body.c`](../entity/body.c),
[`entity/food.c`](../entity/food.c), [`entity/immune.c`](../entity/immune.c),
[`entity/skeleton.c`](../entity/skeleton.c), and cross-module constants in
[`universe/universe.h`](../universe/universe.h).

A being in ApeSDK is not a character script. It is a bundle of bounded state:
energy, body, genetics, drives, social graph, episodic memory, territory
familiarity, immune state, brain state, and braincode.

## Noble Ape Body Context

The [Noble Ape 2012 transcript](https://nobleape.com/) describes ape internals
in terms that still echo through ApeSDK: location, speed, energy, speaking
state, random state for distribution, date of birth, brain location, cognitive
state, inventory, family structure, genetics, metabolism, and later vascular or
toxicology additions. That history supports the central claim of this chapter:
the being is not a thin visual agent. It is an accumulation of interacting body,
history, and control surfaces.

The transcript also discusses development, maturity, lifespan, and the desire to
expand genetics toward richer regulatory effects. The current code keeps the
genetic model compact, but the historical direction is useful. Genetics in
ApeSDK should be read as a stable variation mechanism that can grow, not as a
finished biological claim.

## Archive Evidence

- The [Original Manuals](https://www.apesdk.com/doc/Barbalet_OriginalManuals.pdf)
  describe movement as "desire-driven motivation." That phrase is the early
  version of this chapter's body-first claim: beings do not merely move because
  a rule says so; movement is attached to internal pressure, energy, state, and
  future orientation.
- [The Mind of the Noble Ape in Three Simulations](https://www.apesdk.com/doc/Barbalet_OriginOfMind.pdf)
  argues that the environment needed enough depth for simulated minds to
  flourish. In this chapter, that supports the idea that bodies and worlds must
  be interpreted together.
- The [Simulated Ape Manual](https://www.apesdk.com/doc/man/) remains the
  user-facing companion for current behavior, while `entity/` is the source
  authority.
- The auto-captioned
  [downloaded ApeSDK explanation](https://www.youtube.com/watch?v=mvORipomK98&t=50s)
  identifies a visible "cognitive simulation" alongside the running beings and
  command interface. That supports this chapter's body-first claim: the ape on
  screen is the observable surface of several embodied internal systems.

## Energy And Survival

`entity/entity.h` defines the basic energy bands:

```c
BEING_DEAD
BEING_HUNGRY
BEING_FULL
```

Food energy values include grass, bush, fruit, seaweed, shellfish, nut, fish,
bird eggs, and lizard eggs. These values are not flavor text. They define the
economics of movement. If food is too generous, foraging disappears as pressure.
If food is too scarce, social behavior collapses under constant hunger.

The useful question is not "does a being eat?" It is "what other behaviors does
the energy system allow or suppress?"

## Body State

The being state enum includes asleep, awake, hungry, swimming, eating, moving,
speaking, shouting, grooming, suckling, show-force, attack, and no-food states.
Because states are bit values, a being can carry more than one condition at a
time. That allows conflicts such as moving while hungry or being awake while
under social pressure.

State bits are a compact phenotype vocabulary. They are also a test surface:
if a new action does not leave a clear state trace, it will be difficult to
debug or observe.

## Genetics

ApeSDK genetics are compact, regulator-driven values derived from four
chromosomes. Macros such as `GENE_VAL`, `GENE_VAL_REG`, `GENE_FRAME`,
`GENE_HAIR`, `GENE_PIGMENTATION`, `GENE_VISION_INITIAL`, and many others turn
encoded genetic material into traits.

The model is not trying to be a full biological genome. It is using inherited
variation as a stable source of difference between beings. That stability is
important because a fixed seed should create a repeatable distribution of
traits.

Genetics influence:

- appearance;
- body frame and growth;
- social preferences;
- braincode composition;
- affect fading;
- mate-seeking tendencies;
- immune behavior.

## Growth, Reproduction, And Care

Important life-history constants include:

- `AGE_OF_MATURITY`;
- `GESTATION_DAYS`;
- `SUCKLING_ENERGY`;
- `WEANING_DAYS`;
- `CARRYING_DAYS`;
- `CONCEPTION_INHIBITION_DAYS`.

These are long-horizon filters. Changing one value may not show up in the first
few cycles, but it can reshape population structure, kinship, social memory, and
mate competition across generations.

When tuning these constants, do not judge only the first successful birth. Track
population stability, age distribution, energy variance, and social graph depth.

## Inventory And Manipulation

Inventory values include children, wounds, grooming traces, branches, rocks,
shells, twigs, nuts, grass, scrapers, spears, fish, and eggs. `entity/body.c`
uses these concepts to create and store events such as giving, whacking, hurling,
chewing, dropping, pickup, and fishing.

Inventory is where physical context becomes social context. A branch is not only
an object; it may later become part of an episodic event and change how another
being is remembered.

## Immune State

The current tree includes [`entity/immune.c`](../entity/immune.c) and a focused
deterministic immune test under [`entity/test`](../entity/test/). Immune state is
therefore part of the active quality boundary, not just an unused design note.

Immune behavior matters because it adds another source of internal pressure. A
being can be hungry, tired, socially motivated, and biologically stressed at the
same time. The model becomes more interesting when those pressures interact
without becoming untestable.

## Practical Protocol

When changing being-level code:

1. Identify whether the change is body, energy, genetics, immune, inventory, or
   state-machine work.
2. Add or adjust the smallest deterministic test that can prove the mechanism.
3. Run the project test runner.
4. Only then inspect long-run emergent behavior.

For biology changes, keep a fixed-seed matrix:

- baseline;
- energy-biased;
- reproduction-biased;
- immune-biased;
- inventory/action-biased.

The aim is to see which subsystem truly caused the observed change.

## Bodies As Negotiated Limits

A being in ApeSDK is best understood as a negotiated limit rather than a free
agent. It has energy, growth, drives, genetics, inventory, memory, immune state,
and social relationships. Each of those layers limits what the others can do.

This is important because artificial life often becomes unconvincing when
agents are allowed to act as pure decision systems. Real organisms do not make
choices from nowhere. They act from bodies. Hunger narrows attention. Fatigue
changes movement. Age changes reproduction. Injury or immune stress changes
available energy. Inventory changes both opportunity and risk.

ApeSDK's body model is compact, but it carries the right kind of pressure. A
being that is hungry is not simply a being with a different label. It is a being
whose future actions are now more constrained. A being carrying an object is not
only carrying an item; it may be carrying future social meaning. A being with a
new episodic memory is not simply storing history; it is changing the context in
which body and drives will be evaluated later.

## Energy Is A Clock

Energy is usually read as fuel. It is also a clock. A full being has time to
explore. A hungry being has less time before survival pressure dominates. A
being at zero energy has crossed a terminal boundary. Energy therefore measures
how long other behaviors are allowed to matter.

This framing helps explain why food values need care. If one food type gives too
much energy, it can create long spans where hunger is irrelevant. If food values
are too low, the model becomes a starvation simulator. If rare foods are too
valuable, beings may overfit to one region or object. If all foods are similar,
the environment loses ecological texture.

Energy changes should be read through multiple outputs:

- time spent hungry;
- distance traveled while hungry;
- food event frequency;
- fatigue accumulation;
- reproduction success;
- mortality or collapse;
- social event frequency while hungry.

The last item is especially important. Hunger does not only affect eating. It
can suppress sex drive, redirect travel, and reduce social variety. A food
constant can therefore become a social constant by another route.

## Genetics And Variation Without Mystery

The genetic macros in ApeSDK create variation, but they do so in a readable way.
That is a useful middle ground. The engine does not require a high-fidelity
biological genome to create inherited differences. It needs stable variation
that can be tied back to source.

This makes genetics useful for experiments. If a group behaves differently, the
reader can ask whether it inherited a different distribution of body, sensory,
social, or braincode tendencies. Genetic variation is not hand-waved as
"personality." It has named extraction points.

At the same time, inherited values should not be interpreted too strongly. A
genetic macro creates a tendency, not destiny. Learned preference, episodic
memory, drive state, and environment can all alter the result. Good analysis
keeps nature and nurture in the same frame.

The `NATURE_NURTURE` macro is a concise expression of that idea. It turns an
inherited value and a learned preference into a combined value. That is the kind
of small code artifact that carries a large modeling philosophy: beings are not
blank slates, and they are not fixed machines.

## Reproduction As Population Structure

Reproduction constants are easy to treat as local biological parameters. In
practice, they shape the entire population. Maturity age determines how long a
being participates socially before reproducing. Gestation and conception
inhibition determine reproductive pacing. Weaning and carrying determine mother
and child coupling. Suckling energy determines a transfer from adult body state
to offspring survival.

A small change can alter:

- generation overlap;
- kinship density;
- competition between cohorts;
- maternal energy pressure;
- infant survival;
- social memory around care events;
- long-run population stability.

That means reproduction changes should be tested over longer horizons than a
single action change. A one-cycle unit test can prove a constant is applied. A
fixed-seed scenario can show whether the population effect is plausible.

## Inventory As Embodied Memory

Inventory is a bridge between physical and social modeling. An object can be
picked up, carried, dropped, given, used, or associated with an event. Once an
event is stored, the object is no longer only physical. It becomes part of
memory.

For example, a rock may be a carried object. If it is used in a conflict, it can
become part of an episodic event. That event can later alter how beings remember
one another. The object has become a historical mediator.

This is a powerful pattern because it lets the world create meaning without
requiring a large symbolic system. Objects matter when they participate in
actions. Actions matter when they become events. Events matter when they alter
future behavior.

When adding inventory concepts, ask:

- Can the object be observed?
- Can it be carried or stored?
- Does it affect energy, conflict, construction, or care?
- Does it create episodic memory?
- Does it need transfer/save/load support?
- Does it need command or rendering visibility?

If the object does none of those, it may be decoration rather than model.

## Immune Behavior And Hidden State

Immune state is an example of a subsystem that may not dominate the visual story
but can matter deeply to the being. Internal stress can alter survival,
activity, reproduction, and energy use. It can also create delayed effects that
are easy to miss if the observer watches only immediate actions.

The presence of a focused immune test is important. It says this subsystem has a
deterministic boundary. A reader can examine immune initialization and response
without needing a full world run.

As immune behavior grows, it should maintain that discipline. Hidden state is
dangerous when it cannot be inspected. If immune dynamics begin to affect
behavior, the command or logging layer should expose enough state to explain the
effect.

## Body Changes Need Many Kinds Of Evidence

A body change can be correct in isolation and still harmful to the model. For
example, increasing the energy value of fish may pass every local food test. But
if fish become so valuable that beings cluster permanently near water, the
change has altered geography, social density, mating, conflict, and memory.

This does not mean the change is wrong. It means the evidence must match the
blast radius.

Use three evidence levels:

1. Unit evidence: the value or state changes correctly.
2. Scenario evidence: a fixed-seed run changes in an expected direction.
3. System evidence: unrelated behaviors do not become implausibly dominated.

The stronger the body change, the more levels you need.

## Case Study: Energy Tuning That Accidentally Changes Culture

Imagine increasing `ENERGY_FISH` because fishing seems too unrewarding. On the
surface, this is a food balance change. But if fish become the strongest energy
source, beings near water may spend more time there. More time near water means
more swimming, more fatigue, more shoreline encounters, and more repeated
co-location with the same individuals.

After several cycles, the social graph may show stronger relationships among
shoreline beings. A reader might call this a cultural cluster. That may be a
reasonable interpretation, but the cause began with a food constant.

The right analysis follows the chain:

```text
ENERGY_FISH increased
fish events became more valuable
shoreline occupancy increased
encounter frequency increased
social graph familiarity increased
episodic memories clustered around shoreline beings
mate or grooming patterns changed
```

This is not a warning against changing food values. It is a warning to respect
their reach.

## Case Study: Maturity Timing And Social Depth

Changing `AGE_OF_MATURITY` alters more than reproduction. If beings mature later,
they spend more time as non-reproductive social participants. That can deepen
pre-reproductive social memory, alter kinship timing, and change competition
between cohorts. If beings mature earlier, population growth may accelerate but
social history before reproduction may be shallower.

The effect may only appear over multiple generations. A unit test can prove that
maturity gates sex drive, but a scenario run is needed to understand population
shape.

Useful metrics include:

- age at first mate goal;
- age at first birth;
- number of social entries before maturity;
- episodic memory depth before first reproduction;
- population growth slope;
- child survival;
- distribution of kin relationships.

This is the kind of biological change that deserves documentation because it
alters the reader's expectation of what a being's life means.

## Body State And Command Visibility

State bits are useful only if they can be observed. If a being can be hungry,
swimming, eating, moving, speaking, shouting, grooming, suckling, showing force,
attacking, or unable to find food, the observer needs some way to inspect those
states. Otherwise the model can be correct internally and opaque externally.

The command system and render layer do not need to display every bit at all
times, but the state should be reachable during debugging. A hidden state is
especially dangerous when it suppresses or enables behavior. If a being is not
seeking a mate because another state blocks it, the reader should be able to see
that.

When adding a new state bit, include:

- a description string if state descriptions are available;
- command or debug visibility;
- tests for setting and clearing;
- interactions with existing states;
- persistence decision if saved state must include it.

## Growth And Body Proportion

The body is not only energy and state. Genetic values for frame, height, growth,
appearance, and body segments create visible and behavioral variation. Some of
these values may primarily affect presentation today, but presentation can still
matter for selection, recognition, or future behavior.

The reader should not separate appearance too sharply from simulation. In a
social model, visible traits can become features. Features can become
preferences. Preferences can influence attraction or grooming. An appearance
field becomes social when another being can classify or respond to it.

This gives a path for future work: body traits should become meaningful through
observation and preference, not by arbitrary special casing.

## Persistence Of The Being

A being's state matters across cycles and across saves. Transfer support must
preserve the fields that define continuity: location, energy, date of birth,
random seed, state, brain state, height, mass, and social or episodic data where
appropriate.

If a being changes after reload, ask whether the body state was fully
transferred. A missing field can create subtle differences: a being may wake
differently, move differently, or remember differently after load. Save/load is
therefore a body test as much as a file test.

## Adding A New Body Mechanism

A new body mechanism should follow a full path:

```text
define state or field
define range and default
define update rule
define interactions with drives and memory
define observation path
define persistence path if needed
define deterministic tests
define documentation note
```

Skipping the observation path is the most common mistake. If the mechanism
matters, someone must be able to see it.

## Chapter Five Checklist

Before moving on, make sure you can:

- explain why energy is also a behavioral clock;
- distinguish body state from drive state;
- describe how genetics create stable variation;
- explain how reproduction constants affect population structure;
- trace an inventory item into an episodic event;
- explain why immune state needs deterministic tests;
- design a scenario that separates food, fatigue, and social effects.

The next chapter takes the embodied being and asks how it remembers others.

## Open Design Questions

The being model can become more detailed in many ways: injury, disease, tool
skill, growth stages, richer pregnancy, handedness, more nuanced sleep, or
expanded sensory ability. Each addition should earn its place by changing a
testable behavior.

The most important question is whether a new body concept has an update rule and
an observation path. A field that is stored but never affects behavior is only
metadata. A field that affects behavior but cannot be observed is hidden
causality. A field that can be observed but not tested is fragile.

The current model's strength is that its pieces are compact. Future body work
should preserve that compactness where possible.

## Practical Exercise

Trace one food event from environment to body to memory:

1. Identify where the food can exist.
2. Identify the energy value.
3. Find the action path that consumes or picks it up.
4. Find the state change.
5. Find any episodic event created.
6. Ask whether drive values should change afterward.

This exercise demonstrates the core ApeSDK pattern: physical world, embodied
change, remembered event.

## Body Vocabulary

Use careful language when discussing beings:

- energy is stored survival capacity;
- state is current condition;
- drive is internal pressure;
- genetics is inherited tendency;
- inventory is carried context;
- immune state is internal biological stress;
- memory is stored experience.

Keeping these terms separate prevents a common mistake: explaining everything as
"behavior" before identifying the layer that changed.
