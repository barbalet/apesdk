# Chapter 6 - The Social Brain Before The Brain

Social behavior in ApeSDK begins before explicit cognition. The main source
files are [`entity/social.c`](../entity/social.c),
[`entity/being.c`](../entity/being.c), [`entity/episodic.c`](../entity/episodic.c),
and structure definitions in [`universe/universe.h`](../universe/universe.h).

The central idea is bounded social cognition. A being does not maintain an
infinite relationship database. It keeps a limited social graph, bounded feature
sets, familiarity, attraction, relationship type, and memory-derived context.

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
