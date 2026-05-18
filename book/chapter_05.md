# Chapter 5 - Designing The Being: Genes, Energy, And Physical Rule Sets

The being layer turns the world from landscape into life. Its main anchors are
[`entity/entity.h`](../entity/entity.h), [`entity/body.c`](../entity/body.c),
[`entity/food.c`](../entity/food.c), [`entity/immune.c`](../entity/immune.c),
[`entity/skeleton.c`](../entity/skeleton.c), and cross-module constants in
[`universe/universe.h`](../universe/universe.h).

A being in ApeSDK is not a character script. It is a bundle of bounded state:
energy, body, genetics, drives, social graph, episodic memory, territory
familiarity, immune state, brain state, and braincode.

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
