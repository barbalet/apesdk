# Chapter 7 - Drives, Emotion, And Decision Pressure

The current drive model lives in [`entity/drives.c`](../entity/drives.c), with
drive constants and preferences in [`universe/universe.h`](../universe/universe.h).
It has four biological drives:

- `DRIVE_HUNGER`;
- `DRIVE_SOCIAL`;
- `DRIVE_FATIGUE`;
- `DRIVE_SEX`.

Drives are not emotions in the literary sense. They are bounded internal
pressures that make some actions more likely and suppress others.

## Noble Ape Drive Context

The [Noble Ape reference](https://nobleape.com/) names the same four motivation
classes that remain central here: hunger, fatigue, sex, and social pressure. It
also notes that fatigue and hunger are dominant, while social motivation can
draw beings toward familiar others and familiar places. That historical framing
is helpful because it shows the drive system was never meant as a decorative
mood layer. It was meant as a state-transition mechanism.

The transcript's discussion also reinforces an important idea in this chapter:
drives do not act alone. They combine with new information, social memory,
episodic memory, location, and language-like representations. In current
ApeSDK terms, drive values are one part of a larger decision context.

## Archive Evidence

- The [Original Manuals](https://www.apesdk.com/doc/Barbalet_OriginalManuals.pdf)
  treat natural desires as time-bound needs: food, sleep, safety, and
  reproduction. This is the historical basis for reading the current four-drive
  model as scheduling pressure rather than mood decoration.
- The [Nature Inspired Informatics chapter](https://www.apesdk.com/doc/Barbalet_NatureInspired.pdf)
  names "fear and desire" as two primary cognitive elements. In current ApeSDK
  terms, this supports the chapter's contrast between immediate suppression,
  longer-horizon goals, and drive-mediated movement.
- The auto-captioned
  [ApeSDK Talk](https://www.youtube.com/watch?v=qdqe0mCgeUE&t=1379s) names the
  four-drive list as sex, hunger, social, and fatigue. That oral-history
  reference usefully ties the current drive fields to the older biological and
  robotics lineage discussed in this chapter.

## Hunger

`drives_hunger()` checks energy against `BEING_HUNGRY`. If the being is hungry,
the hunger drive rises and sex drive falls. If energy is sufficient, hunger
falls.

This creates triage. A hungry being is less available for mating behavior,
because survival pressure consumes the decision budget.

The testable claim is simple:

```text
lower energy -> higher hunger drive -> lower sex pressure -> fewer mate goals
```

If a change breaks that chain, either the drive logic changed or another module
is overpowering it.

## Sociability

`drives_sociability()` scans nearby beings and sends a count through
`being_crowding_cycle()`. This turns local density into internal state.

Local density can have two interpretations:

- social opportunity;
- crowding pressure.

That ambiguity is valuable because it lets the same environment produce
different outcomes depending on thresholds, learned preference, and current
state.

## Sex Drive And Mate Goals

`drives_sex()` is gated by age, wake state, pregnancy, episodic memory, and the
social graph. Mature awake beings increase sex drive. Above
`THRESHOLD_SEEK_MATE`, a being may seek a remembered mate or the most attractive
entry in its social graph.

The function also stores `EVENT_SEEK_MATE` when a goal is created. This is the
important bridge: a drive state becomes an intention, and the intention becomes
episodic data.

## Fatigue

`drives_fatigue()` checks movement speed. Movement above
`FATIGUE_SPEED_THRESHOLD` increases fatigue. Swimming increases it again. Rising
fatigue decreases sex drive. Resting lowers fatigue.

Fatigue gives cost to travel and swimming. Without it, movement-heavy strategies
can become unrealistically cheap.

## Preferences And Learned Pressure

The drive system does not work alone. Preferences in `universe/universe.h`
include mate height, pigmentation, hair, frame, grooming, anecdote mutation,
chat, and social preference. Some values are genetic, some learned, and some
combined through macros such as `NATURE_NURTURE`.

This is one of the central design ideas of ApeSDK:

```text
body state + inherited tendency + learned preference + social memory -> action pressure
```

No single term should be treated as the whole explanation.

## Drive Testing Protocol

For any drive change, record all four drives over time for selected beings:

- hunger vs energy;
- social drive vs local density;
- sex drive vs age, wake state, pregnancy, and mate goals;
- fatigue vs movement speed and swimming state.

Then run fixed seeds with only one perturbation:

1. low food;
2. high crowding;
3. altered maturity or pregnancy condition;
4. high movement or swimming conditions.

The output should be directional and explainable before it is considered useful.

## Questions For The Reader

- Which drives suppress each other directly?
- Which drives only appear to suppress each other through goals or state?
- Which thresholds are inherited from constants rather than learned?
- When does an intention become memory?
- Can the behavior be reproduced with deterministic tests, or only observed in a
  long run?

## Drives As A Scheduler

Drives can be read as a scheduler for biological urgency. They do not directly
write a full action plan. Instead, they change which plans are likely to be
available, sustained, or suppressed. Hunger says "food matters now." Fatigue
says "movement has a cost." Sex drive says "mate-seeking may become relevant."
Sociability says "nearby beings matter."

This scheduler framing is useful because it avoids over-personalizing the code.
A being is not hungry because it has a narrative desire. It is hungry because
energy crossed a threshold and a drive counter changed. That counter then
participates in later decisions.

The result can still look lifelike, but the explanation stays grounded. This is
the heart of artificial life: simple internal pressures interacting long enough
to produce behavior that deserves interpretation.

## Drive Interactions Are The Interesting Part

A single drive is easy to understand. Interactions between drives are where the
model becomes expressive. Hunger suppresses sex. Fatigue suppresses sex. Social
density can alter internal social pressure. Pregnancy changes sex drive.
Wakefulness gates mate-seeking. Age gates whether sex drive is meaningful.

The important point is that drives are not parallel sliders that simply add
together. Some drives open doors. Some close them. Some only matter when a body
state or social memory is present.

For example, a mature awake being with high sex drive may still fail to seek a
mate if hunger or fatigue has suppressed the drive, if no social graph entry is
attractive enough, if remembered mate information is absent, or if another goal
is already active. The visible absence of mating is therefore not a simple
absence of sex drive.

Good drive interpretation asks what blocked the path.

## Thresholds And Meaning

Thresholds such as `BEING_HUNGRY`, `THRESHOLD_SEEK_MATE`, and
`FATIGUE_SPEED_THRESHOLD` turn continuous or bounded values into qualitative
changes. Below a threshold, one kind of behavior is available. Above it, another
kind appears.

Thresholds are powerful because they create phase changes. A being can look
stable for a long time and then switch behavior when one counter crosses a line.
That kind of change feels lifelike because organisms often appear to shift modes
rather than change smoothly.

Thresholds are also dangerous. If a threshold is too low, behavior can trigger
constantly. If too high, it may never trigger. If two thresholds interact, one
may silently dominate the other. This is why threshold changes need fixed-seed
tests and plots, not only visual inspection.

## Hunger As Attention Collapse

Hunger is the clearest example of attention collapse. When energy is below
`BEING_HUNGRY`, hunger rises and sex drive falls. This makes biological sense:
survival takes priority over reproduction.

But hunger also has indirect effects. A hungry being may travel more, enter
riskier terrain, encounter different beings, or miss social opportunities. It
may store different episodic memories because it is in different places. It may
become fatigued from foraging, which further suppresses other drives.

This chain shows why energy and hunger belong in social analysis. A social
pattern may change because food changed first.

## Fatigue As A Memory Filter

Fatigue does not only regulate movement. It can filter experience. A tired being
may move less, meet fewer others, seek mates less often, and create fewer social
events. Over time, that changes memory formation.

Swimming is especially interesting because it increases fatigue more strongly.
Water-heavy geography can therefore create a body-state filter: beings near
water may pay different movement costs and build different social histories.

If a terrain change increases swimming, look at fatigue before interpreting
social outcomes. A decline in mating may be caused by movement cost rather than
mate preference.

## Sociability As Context Rather Than Personality

Sociability is easy to mistake for personality. In the current drive code,
nearby beings and crowding cycles are central. That means sociability is partly
contextual. A being in a dense region experiences different social pressure than
the same being in isolation.

This matters for interpretation. If one being appears "more social," ask whether
it had more opportunities. Did it live near resources? Did it occupy a crossing?
Did terrain funnel others toward it? Did its movement pattern keep it close to a
group? Social drive and social history cannot be separated from local density.

## Building A Drive Dashboard

A useful future tool would display the four drives over time for a selected
being alongside energy, speed, wake state, pregnancy, goal, and nearby-being
count. Even without a new tool, the conceptual dashboard helps when reading logs
or command output.

For each sampled being, capture:

```text
time
energy
hunger drive
social drive
fatigue drive
sex drive
awake/asleep
speed
swimming state
current goal
nearby being count
latest episodic event
```

With that table, many behaviors become explainable. Without it, observers are
tempted to invent intention where there may only be a threshold crossing.

## How To Extend Drives

Adding a new drive should be rare. Every drive adds a new axis of explanation
and a new source of hidden interaction. Before adding one, ask whether the same
effect is better modeled as:

- an energy change;
- a body state;
- a preference;
- an episodic affect;
- a social graph field;
- a terrain pressure;
- a script-controlled experiment.

If a new drive is justified, define its range, increment/decrement rules,
suppression relationships, command visibility, tests, and transfer implications.
The drive should not be invisible. A hidden drive creates hidden causality.

## Case Study: Low Food And Disappearing Courtship

Suppose courtship-like behavior drops after a terrain or food change. It is
tempting to inspect mate-selection logic first. The drive layer suggests a
different first question: did hunger rise?

If energy falls below `BEING_HUNGRY`, hunger drive rises and sex drive falls.
The model may still have mature beings, attractive social graph entries, and
valid mate memories, but the drive scheduler has reduced the pressure to act on
them.

The diagnostic sequence is:

```text
compare energy before and after change
compare hunger drive
compare sex drive
compare EVENT_SEEK_MATE count
compare actual mate events
compare travel and fatigue
```

If seek-mate events drop with sex drive, the effect is internal pressure. If
seek-mate events remain but mating drops, the blockage is later: terrain,
location, fatigue, target availability, or social context.

## Case Study: High Movement And Social Silence

Imagine beings move more after a pathing change, but grooming and mating fall.
The cause may be fatigue. Higher movement speed raises fatigue. Swimming raises
it further. Fatigue suppresses sex drive. Movement can also reduce time spent
near the same beings, lowering social opportunity.

This case demonstrates why "more active" does not always mean "more social."
Activity has cost. A being can be busy enough that social behavior declines.

Metrics should include:

- average speed;
- fatigue drive;
- swimming state;
- nearby-being count;
- grooming/chat events;
- sex drive;
- mate goals;
- path length.

The social silence may be embodied, not psychological.

## Drive Saturation And Loss Of Sensitivity

Drives have maximum values. Saturation can hide differences. If many beings
spend most of a run at maximum hunger, the model cannot distinguish moderately
hungry from desperately hungry. If fatigue saturates, movement-cost differences
may disappear. If sex drive saturates, mate-seeking may depend more on gates and
goals than drive magnitude.

Saturation is not automatically wrong. It can represent an extreme state. But if
a drive is saturated too often, it loses explanatory power.

Drive analysis should include distributions, not only averages. A mean hunger
value may look acceptable while half the population is saturated and half is
not. Histograms or sampled trajectories reveal whether drives are operating in a
useful range.

## Drives And Goals

Drives are internal pressure. Goals are directed intention. The transition from
drive to goal is one of the most important behavioral moments. In mate-seeking,
sex drive above a threshold can create a mate goal based on episodic memory or
social graph attraction.

A goal then persists until cleared or completed. That persistence means current
behavior can reflect earlier drive state. A being may continue pursuing a goal
after the original pressure has changed, until logic clears it.

When debugging goal behavior, record:

- drive value when goal was created;
- event stored when goal was created;
- current drive value;
- goal age;
- conditions that clear the goal;
- whether another drive should override it.

This prevents confusion between "the being wants this now" and "the being is
still following a goal created earlier."

## Drives In The Command Interface

Drive values should be easy to inspect during debugging. If a user can see
energy, body state, current goal, and four drives together, many behaviors
become readable. If those values are scattered, interpretation becomes harder.

Good command output for a selected being might present:

```text
Energy: value and hungry/full label
Drives: hunger/social/fatigue/sex
State: awake, moving, swimming, eating, etc.
Goal: none, mate, location, or other
Recent event: last episodic record
Social context: nearby count or selected social target
```

This gives a compact behavioral dashboard without needing a new visual overlay.

## Adding Drive Tests

Drive tests should isolate one drive at a time before testing interactions:

- hunger increases below energy threshold;
- hunger decreases above threshold;
- fatigue increases above speed threshold;
- swimming adds fatigue;
- sex drive increases only when mature and awake;
- pregnancy decreases sex drive;
- mate goal clears below threshold;
- sociability responds to nearby count.

Interaction tests can then prove suppression paths. Keep them deterministic and
small. Long-run drive behavior is useful, but it should rest on simple unit
truths.

## Chapter Seven Checklist

Before moving on, make sure you can:

- explain each of the four drives;
- identify direct suppression paths;
- distinguish drive pressure from goals;
- recognize saturation as a diagnostic issue;
- connect terrain and food to drive changes;
- design a selected-being drive trace;
- decide whether a new pressure belongs as a drive or another kind of state.

The next chapter shows how compact control programs can act on the pressures
that drives help create.

## Open Design Questions

The drive model could be expanded with thirst, fear, pain, curiosity, parental
care, territoriality, or play. Each candidate should be treated skeptically. A
new drive is not just another number. It changes scheduling, tests, observation,
and explanation.

Some concepts may be better as events or preferences. Fear might be an affect
consequence of memory rather than a permanent drive. Parental care might be a
state and relationship effect rather than a drive. Curiosity might be a
movement or exploration policy. Pain might be body state plus drive suppression.

The guiding question is: does this pressure need its own persistent bounded
counter, or can it be represented through existing state?

## Practical Exercise

Choose one selected being and sample it every fixed interval for a short run.
Create a table:

```text
time | energy | hunger | social | fatigue | sex | state | goal | event
```

Then write a short explanation of one action using only table values. If the
action cannot be explained, identify what state is missing from the table. This
turns drive interpretation into evidence rather than guesswork.

## Drive Metrics For Version Comparison

When comparing releases or branches, useful drive metrics include:

- percent of time hungry;
- percent of time fatigue is high;
- sex drive distribution among mature awake beings;
- social drive distribution by density;
- mate goals created per mature being;
- hunger suppression events;
- fatigue suppression events;
- drive saturation time.

These metrics can reveal behavioral drift before visible behavior becomes
obviously wrong.

## Drive Vocabulary

Use precise terms:

- drive is internal pressure;
- threshold is a value where behavior changes category;
- suppression is one drive reducing another;
- goal is directed intention;
- state is current body condition;
- event is stored history.

When those words blur, drive analysis becomes storytelling.

## Future Exercise: Drive Perturbation Ladder

A drive perturbation ladder changes one condition at a time:

```text
baseline
slightly lower energy
strongly lower energy
high movement without swimming
high movement with swimming
high density with normal food
high density with low food
```

The point is not to create a dramatic run. The point is to observe how drives
move through related conditions. A good ladder shows smooth directional changes
until a threshold produces a phase shift.

This kind of ladder is also a good way to find bad constants. If a small
perturbation saturates a drive immediately, the constant may be too sensitive.
If a large perturbation does nothing, the drive may be disconnected.

## Negative Space In Drive Analysis

Pay attention to behaviors that do not happen. No mate goal, no grooming, no
movement, or no food event can all be informative. Absence often points to a
closed gate. The task is to find which gate:

- body state;
- age;
- energy;
- fatigue;
- social opportunity;
- memory;
- terrain;
- current goal.

Drive analysis is often the study of why a plausible action did not occur.

## Reader Lab: Explaining One Missing Action

Pick one action you expected but did not see, such as mating, grooming, eating,
or movement. Then explain the absence with evidence. Do not allow "the being did
not want to" as the first answer. Translate want into state.

For each missing action, check:

```text
was the being physically able?
was the relevant drive high enough?
was another drive suppressing it?
was a goal already active?
was the target visible or reachable?
was the being awake?
was memory helping or blocking?
```

This lab is valuable because absence is often where drive logic is most visible.
Actions that happen have many possible explanations. Actions that fail to happen
often reveal the closed gate.
