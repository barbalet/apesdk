# Chapter 9 - Learning By Event: Episodic Memory And Meaning

Episodic memory is where events become behavioral history. The main files are
[`entity/episodic.c`](../entity/episodic.c), event and affect definitions in
[`entity/entity.h`](../entity/entity.h), and related social structures in
[`universe/universe.h`](../universe/universe.h).

The model does not give beings an unlimited narrative memory. It gives them a
bounded array of event records and rules for storage, replacement, affect, and
intention.

## Event Vocabulary

The event enum includes eating, mating, hitting and being hit, swimming,
grooming, chatting, shouting, birth, carrying, suckling, seeking a mate,
whacking, hurling, hugging, prodding, dragging, brandishing, dropping, pickup,
giving, chewing, fishing, smiling, glowering, patting, pointing, tickling, and
role-specific variants of many of these.

The role-specific variants are important. `EVENT_WHACKED` and
`EVENT_WHACKED_BY` are not the same memory. The actor and recipient can attach
different meanings to the same physical moment.

## Affect

`COMPOSITE_AFFECT` combines enjoyment, interest, surprise, anger, disgust,
dissmell, distress, fear, and shame into a single affect value. Defined affect
values include mating, birth, carrying, suckling, chat, grooming, seek-mate,
squabble victor, squabble vanquished, whacked, hurl, hugged, prodded, receive,
fish, smiled, and glower.

This is not a full emotional theory. It is a compact way to make events
directional. A memory can pull a being toward a future action or away from it.

## Storage And Replacement

`simulated_iepisodic` stores:

- spacetime;
- first and family names;
- event;
- food;
- affect;
- argument.

`EPISODIC_SIZE` limits the number of memories. Replacement logic in
`entity/episodic.c` decides where new memories go. This is a real modeling
choice. With a small memory, intense or recent events may crowd out older
history. With a large memory, past events may dominate longer than intended.

## Intention

The code distinguishes between remembered events and intentions. `EVENT_INTENTION`
and goal-related storage let the engine represent not only "what happened" but
"what the being is oriented toward."

That bridge is visible in mate-seeking: a drive threshold can set a goal, the
goal can store `EVENT_SEEK_MATE`, and later social or episodic code can use that
history.

## Anecdote And Mutation

The episodic layer includes anecdote behavior and learned preferences for
anecdote event mutation and affect mutation. That gives the memory system a way
to vary how stories are retained or transformed without rewriting the entire
being model.

For experiments, treat anecdote mutation as a powerful source of interpretation
drift. If memory-derived behavior changes unexpectedly, ask whether the event,
affect, or social graph changed.

## Memory Testing Protocol

When adding or changing an event:

1. Add the enum value and affect semantics.
2. Store it from the smallest relevant action path.
3. Test self, actor, and recipient roles.
4. Test replacement when episodic memory is full.
5. Test any goal or preference effects.
6. Verify command output or logs can expose the event.

Useful metrics include:

- event counts by type;
- memory replacement count;
- actor/recipient asymmetry;
- affect distribution;
- time since event;
- goal transitions after memory formation.

## Questions For The Reader

- Which events are self-events, and which are interaction events?
- Which events should produce opposite meanings for actor and recipient?
- What should happen when memory is full?
- Which memories influence drives, preferences, or goals?
- Which memory effects can be proven with deterministic tests?
