# Chapter 6 - The Social Brain Before The Brain

Social behavior in ApeSDK begins before explicit cognition. The main source
files are [`entity/social.c`](../entity/social.c),
[`entity/being.c`](../entity/being.c), [`entity/episodic.c`](../entity/episodic.c),
and structure definitions in [`universe/universe.h`](../universe/universe.h).

The central idea is bounded social cognition. A being does not maintain an
infinite relationship database. It keeps a limited social graph, bounded feature
sets, familiarity, attraction, relationship type, and memory-derived context.

## Noble Ape Social Context

The [Noble Ape talk](https://nobleape.com/) gives a rich historical backdrop for
this chapter. It describes social graph memories, friend/enemy style scalar
relationships, attraction, familiarity, and the idea that meeting and eating
places can reinforce territorial understanding. That maps closely to the
current ApeSDK distinction between social graph entries, episodic memories, and
territory familiarity.

The transcript also emphasizes that not all history can be retained. Social
memory is compressed, updated, and partly represented through code-like internal
models. That is exactly the point of bounded social cognition: what a being
knows socially is not a perfect transcript of the past, but a structured summary
that can guide future action.

## Archive Evidence

- [The Mind of the Noble Ape in Three Simulations](https://www.apesdk.com/doc/Barbalet_OriginOfMind.pdf)
  says the productive use of the mind models is "in concert: not in contrast or
  competition." That quotation is the best archive support for this chapter's
  insistence that social graph, memory, body, environment, and drives must be
  read together.
- The [ApeSDK Philosophic page](https://www.apesdk.com/doc/man/philosophic.html)
  links society to a chain from external space through vision, internal space,
  identity, and language. This supports the chapter's view of social behavior
  as layered rather than isolated.
- The auto-captioned
  [Python version walkthrough](https://www.youtube.com/watch?v=LQ1r0MiJU-w&t=245s)
  describes live output for "social interactions" among related apes. That
  gives a current video reference for this chapter's claim that social state is
  something the engine exposes for inspection, not only something it renders.

## The Social Graph

`simulated_isocial` is the core social graph entry. It records:

- spacetime of the encounter;
- names identifying the other being or entity;
- attraction;
- friend/foe value;
- belief;
- familiarity;
- relationship type;
- entity type;
- a bounded classification feature set;
- optional braincode associated with the entry.

The key limit is `SOCIAL_SIZE`, with `SOCIAL_SIZE_BEINGS` reserved for specific
beings. This keeps social memory finite. A finite graph means replacement,
normalization, and forgetting are not bugs; they are part of the model.

## Feature Sets

With `FEATURE_SET` enabled, each social entry can carry a
`simulated_featureset`. A feature has a value, frequency, and type. The project
also defines:

- `MAX_FEATURESET_SIZE`;
- `MAX_FEATURE_FREQUENCY`;
- `MAX_FEATURESET_OBSERVATIONS`.

Feature frequency is a compact form of learned salience. A being that repeatedly
encounters a trait can weight it more strongly, but bounded frequency prevents
one early signal from dominating forever.

## Identity Matters

Social code depends on names and relationships. If identity is wrong, memory is
wrong. Functions that compare, select, or retrieve beings by name are therefore
part of the social model, not just convenience helpers.

When reading social behavior, separate:

- the being being observed;
- the being remembered;
- the being acting;
- the being affected by an action.

A single event can have different meanings for each role.

## Territory And Place

`simulated_iplace` records place name and familiarity. Territory state is small,
but it lets the engine represent more than interpersonal memory. A being can
also have a history with a place.

This matters because social behavior is rarely only social. Familiar places can
alter movement, contact frequency, resource access, and memory formation.

## Social Graph Experiments

A safe social experiment changes one pressure at a time:

```text
baseline fixed seed
same seed with feature updates reduced
same seed with graph size changed
same seed with attraction updates changed
same seed with territory familiarity disabled or ignored
```

Then compare:

- attraction distribution;
- relationship counts;
- feature replacement frequency;
- mate goal selection;
- grooming and chat events;
- conflict events;
- path length and co-location.

If a social change also changes movement dramatically, the cause may be crowding
or goals rather than attraction itself.

## Practical Reading Order

Read social code in this order:

1. Structure definitions in `universe/universe.h`.
2. Name and state helpers in `entity/being.c`.
3. Feature and social graph updates in `entity/social.c`.
4. Episodic event storage in `entity/episodic.c`.
5. Mate and social pressure use in `entity/drives.c`.

The social brain is not a single module. It is a relay between identity, memory,
features, goals, and drives.

## Social Knowledge Is Compressed Knowledge

The social graph should not be imagined as a perfect relationship database. It
is compressed knowledge. A being does not know another being with unlimited
depth. It stores selected names, familiarity, attraction, relationship, belief,
location, classification features, and sometimes braincode-like associations.

Compression is a modeling strength. It creates forgetting, simplification, and
bias. Those are not failures of intelligence; they are ingredients of realistic
bounded behavior. A being with limited social memory must decide what matters
from repeated encounters. That decision may be imperfect, and imperfection can
produce interesting social dynamics.

This is also why social memory should be tested under pressure. A sparse world
may never fill the social graph. A dense world may force replacement constantly.
The same code can produce stable familiarity in one scenario and volatile
relationships in another.

## Familiarity, Attraction, And Relationship Are Different

Social entries contain several fields that can be confused if read too quickly.
Familiarity is not attraction. Attraction is not relationship. Relationship is
not belief. Location is not identity.

Familiarity says something like "how often or strongly this entry has been
encountered." Attraction can influence mate selection. Relationship can encode
kinship or social category. Belief can encode current interpretation. Location
can make the entry spatially useful.

A being may be familiar without being attractive. A being may be attractive but
not currently nearby. A family relationship may exist even if recent familiarity
is low. Keeping those concepts separate gives ApeSDK a richer social surface
without requiring a huge model.

When analyzing behavior, ask which field actually caused the action. If a being
sets a mate goal, was it driven by attraction, recent mating memory, sex drive,
or lack of other options? The social graph is part of the answer but rarely the
whole answer.

## Feature Sets As Stereotypes And Signals

Feature sets are compact classifications. In human language, one might call them
stereotypes, memories of traits, or recurring signals. In code, they are bounded
features with values and frequencies.

This design lets a being summarize another entity without storing every
encounter. Frequency becomes salience. Value becomes content. Replacement
becomes forgetting. Normalization prevents a single cue from growing without
limit.

The word "stereotype" should be used carefully here. In the model, it means a
compressed feature representation, not a social endorsement. It is a mechanism
for bounded cognition: the being uses reduced information because full
information is unavailable.

Feature experiments should ask:

- Which features are created most often?
- Which features are replaced most often?
- Do high-frequency features dominate attraction or belief?
- Does normalization preserve diversity?
- Does feature pressure change under crowding?

Those questions turn an abstract social concept into measurable state.

## Social Memory And Episodic Memory

Social graph entries and episodic memories are related but distinct. The social
graph is a structured relationship summary. Episodic memory stores event-like
history. A being may have a social entry for another being and also have several
episodic memories involving that being.

This separation is useful because it lets stable relationship and recent event
history diverge. A being may generally be familiar and attractive but recently
associated with a negative event. Another may be unfamiliar but involved in a
strong positive event. The decision system can then combine long-term structure
with recent experience.

When debugging social behavior, inspect both. Looking only at the social graph
may miss the event that changed the next action. Looking only at episodic memory
may miss the long-term relationship that made the event important.

## The Role Of Names

Names in ApeSDK are not cosmetic. They are identity handles. The social graph,
episodic memory, command output, and selection functions all depend on stable
identity. A name mismatch can make memory appear to refer to the wrong being or
make a relationship impossible to retrieve.

This matters for testing. A social test should not only assert that an entry was
created. It should assert that the entry can be found again by the intended
identity path. If the model stores one name format and command code queries
another, social behavior will become difficult to explain.

The reader should also notice how names make the model more humane to inspect.
Numbers are efficient, but names let a person follow a being across logs,
commands, and memories. A synthetic world becomes more legible when its agents
can be addressed.

## Crowding And Social Opportunity

The social model does not operate in a vacuum. The drive system scans local
beings and crowding state, while terrain and movement determine who becomes
nearby. This means social structure is partly environmental.

A dense region can produce many encounters, high familiarity, replacement
pressure, and conflict. A sparse region can produce isolation, low feature
counts, and different mate selection patterns. Neither outcome comes solely from
the social graph. The graph records a history that geography helped create.

Good social experiments therefore include spatial metrics. If a social change
appears, ask whether co-location changed first. If co-location changed, ask
whether terrain, food, water, or fatigue caused it.

## What To Add Carefully

Social systems are tempting places to add new concepts: friendship, dominance,
trust, deception, teaching, alliance, reputation. Some of those may fit ApeSDK,
but each needs a clear storage and observation plan.

Before adding a new social concept, decide:

- Is it a relationship field?
- Is it an episodic event?
- Is it a feature?
- Is it a drive modifier?
- Is it a command output?
- Is it a rendered overlay?
- Does it persist through transfer?

If the concept cannot be placed, it may be too vague. ApeSDK works best when
social ideas become small inspectable fields and transitions.

## Case Study: Attraction Without Encounter

Suppose a being has a high attraction value toward another being in its social
graph but never reaches that being. The social entry alone does not guarantee
behavior. The drive system must open mate-seeking. The goal system must point to
the target. Terrain must allow movement. The target must be locatable enough for
the goal to matter. Fatigue and hunger must not suppress the path.

This case is useful because it prevents over-reading the social graph. The graph
stores potential. Behavior is the result of potential meeting body and world
conditions.

To debug attraction without encounter, inspect:

- sex drive and maturity;
- current goal;
- remembered mate events;
- target name in social graph;
- last known target location;
- terrain and water between beings;
- hunger and fatigue;
- competing social or survival events.

Only then decide whether attraction failed or another layer blocked it.

## Case Study: False Social Change From Density

Imagine a code change appears to make beings more aggressive. Conflict events
rise. Before changing conflict logic, check density. Did a resource change pull
beings into the same region? Did a movement change reduce travel distance? Did a
map-scale change make ranges effectively larger? Did a tide change funnel beings
through a narrow crossing?

If encounter rate rose first, the conflict increase may be a density effect. The
social code may be doing exactly what it did before, just with more
opportunities.

This is why social experiments should include denominators. Count conflicts per
encounter, not only total conflicts. Count grooming per nearby-being minute, not
only total grooming. Rate metrics help distinguish social tendency from social
opportunity.

## Social Graph Persistence

Social memory has to survive save/load if long-running histories matter. A
loaded being should not lose relationship context unless the file format or
experiment intentionally resets it. Transfer code therefore participates in the
social model.

When testing social persistence, save after a known social event, reload, and
inspect:

- relationship field;
- attraction;
- familiarity;
- location and spacetime;
- feature set counts;
- identity fields;
- downstream goal behavior.

If the social graph reloads but behavior changes, the missing state may be
elsewhere: drive, episodic memory, random seed, or location.

## Feature Replacement As Social Bias

Feature replacement can create social bias. If low-frequency features are
replaced, common traits become stable and rare traits disappear. If replacement
prioritizes recency, recent impressions dominate. If intensity matters, strong
events can persist.

Each policy has behavioral consequences. A being that overweights frequency may
stereotype common features. A being that overweights recency may be volatile. A
being that overweights intensity may be shaped by rare dramatic events.

ApeSDK's bounded feature structures make these questions testable. You can run
the same scenario under different replacement strategies and compare social
stability, attraction diversity, and relationship churn.

## Designing Social Commands

Social behavior needs good inspection. A useful social command should expose
enough to explain decisions without overwhelming the reader. Fields might
include:

- target name;
- relationship;
- attraction;
- familiarity;
- last known location;
- feature count;
- strongest feature;
- latest related episodic event.

The best command output is layered. A short view shows the summary. A detailed
view shows the evidence behind it. This mirrors the model itself: compact
summary plus deeper event history.

## Adding A Social Concept Carefully

Suppose you want to add trust. The first question is not how to calculate it.
The first question is where it belongs.

Trust might be:

- a social graph field;
- an aggregate of episodic affect;
- a learned preference;
- a feature;
- a temporary drive modifier;
- a command-only derived value.

Each choice has different consequences. A stored field persists and can drift.
A derived value remains explainable from events but may be expensive or
volatile. A feature can be replaced. A drive modifier affects immediate action.

Pick the representation that matches the claim. If trust is supposed to be
long-term relationship state, store it. If it is supposed to be inferred from
recent memory, derive it.

## Chapter Six Checklist

Before moving on, make sure you can:

- distinguish social graph summary from episodic history;
- explain why bounded social memory is useful;
- identify attraction, familiarity, relationship, and feature state;
- explain why identity names matter;
- include density metrics in social experiments;
- design a save/load check for social graph state;
- decide where a new social concept belongs.

The next chapter asks how internal drives decide when social possibilities
become action pressure.

## Open Design Questions

The social layer could grow in many directions. Trust, dominance, reputation,
coalition, teaching, deception, mourning, territorial ownership, and parental
preference are all plausible artificial life concepts. The challenge is not
imagining them. The challenge is representing them without losing clarity.

A new social concept should be reducible to fields, events, or derived values
that can be inspected. "Dominance" might be a relationship field, an aggregate
of conflict outcomes, a feature frequency, or a command-level summary. Each
choice says something different. A stored dominance field can persist even after
events fade. A derived dominance score remains tied to evidence but may shift
quickly.

The social layer should also avoid becoming a place where every unexplained
behavior is patched. If beings do not meet, the cause may be terrain. If mating
does not occur, the cause may be hunger. If conflict rises, the cause may be
crowding. Social additions should solve social problems, not mask lower-layer
ones.

## Practical Exercise

Pick two beings and build a social dossier from existing outputs or source
state:

```text
Being A:
Being B:
relationship field:
familiarity:
attraction:
last known location:
shared episodic events:
feature count:
current goals:
recent co-location:
```

Then watch one later interaction and ask whether the dossier predicted it. If it
did not, identify which missing layer mattered. Was it hunger? Fatigue? Terrain?
Another being? A missing memory? This exercise teaches social humility.

## Social Metrics That Age Well

When comparing social behavior across versions, prefer metrics that are robust:

- encounters per being per time interval;
- conflicts per encounter;
- grooming or chat per encounter;
- social graph churn;
- average familiarity;
- attraction distribution;
- repeated pair frequency;
- memory-social graph agreement.

Raw totals can be misleading when population size, density, or run length
changes. Rate and distribution metrics make historical comparison safer.

## Chapter Vocabulary

Use these terms carefully:

- encounter means beings came into relevant range;
- event means something was stored or acted on;
- relationship means a social graph category;
- familiarity means repeated or weighted contact;
- attraction means mate-related desirability;
- feature means compressed observed trait;
- memory means episodic event history.

This vocabulary keeps the social brain readable.

## Future Exercise: Social Diffing

A useful future tool would diff one being's social graph between two times. It
could show which entries were added, which were replaced, which attraction
values changed, which features became more frequent, and which relationships
became stale. Even without the tool, the idea is useful.

When reviewing a social change, think in diffs:

```text
before: who is known, how strongly, and why
after: who changed, what field changed, and what event explains it
```

This prevents vague conclusions like "the social graph changed." It turns the
change into evidence.

## Social Behavior Needs Negative Results

Not every social experiment should produce more social behavior. Sometimes the
important finding is that a change did not affect relationships. If a terrain
change alters movement but social graph churn stays stable, that says the social
layer is robust to that environmental variation. If a memory change alters
episodic records but attraction does not move, that says attraction is not
coupled to that memory path.

Negative results are useful when the setup is controlled. They show which
modules are independent enough to trust.
