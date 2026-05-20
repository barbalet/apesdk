# Chapter 9 - Learning By Event: Episodic Memory And Meaning

Episodic memory is where events become behavioral history. The main files are
[`entity/episodic.c`](../entity/episodic.c), event and affect definitions in
[`entity/entity.h`](../entity/entity.h), and related social structures in
[`universe/universe.h`](../universe/universe.h).

The model does not give beings an unlimited narrative memory. It gives them a
bounded array of event records and rules for storage, replacement, affect, and
intention.

## Noble Ape Memory Context

The [Noble Ape reference](https://nobleape.com/) explicitly distinguishes social
associations from episodic associations. In the transcript, social associations
are primarily about other apes, while episodic associations can include apes and
locations, including resource-related events. It also discusses the classic
"what, when, where" framing of episodic memory. That maps directly to the
current ApeSDK structures that store event type, names, affect, spacetime,
location-derived context, and additional arguments.

The historical discussion is also clear that memory is selective. Not every
past event is retained. Important events, scalar social changes, and
language-like internal representations interact over time. That supports this
chapter's emphasis on bounded memory, replacement, and evidence chains.

## Archive Evidence

- The [ApeSDK Philosophic page](https://www.apesdk.com/doc/man/philosophic.html)
  says "places and events can be considered as objects." That line supports the
  chapter's view that episodic memory is not only a list of social incidents; it
  is a way to bind beings, locations, objects, and actions into future evidence.
- The [Original Manuals](https://www.apesdk.com/doc/Barbalet_OriginalManuals.pdf)
  discuss past history as daily movement and interaction, with trivial or
  repetitive elements forgotten. That maps directly onto bounded episodic memory
  and replacement.
- [The Mind of the Noble Ape in Three Simulations](https://www.apesdk.com/doc/Barbalet_OriginOfMind.pdf)
  gives the higher-level reason: social constraints and narrative are part of
  the applied mind, not optional storytelling.
- The auto-captioned
  [overview video](https://www.youtube.com/watch?v=W9SmPYhtsGA&t=536s) places
  episodic material beside the social aspects of the simulation, while the
  [downloaded ApeSDK explanation](https://www.youtube.com/watch?v=mvORipomK98&t=87s)
  points to the "Narrative engine" as live output. Together they support this
  chapter's link between memory, social history, and generated narrative.

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

## Memory As A Compression Of Causality

Episodic memory in ApeSDK should be read as compressed causality. The engine
does not store an unlimited record of everything that happened. It stores
selected events with enough context to bias future behavior. That means each
memory record has to carry disproportionate meaning.

An event record answers several questions at once:

- what kind of event happened;
- who was involved;
- where and when it happened;
- what affect value was attached;
- whether food or another argument was involved.

That compact record can later influence intention, preference, social
interpretation, or command output. It is not a diary. It is a behavioral prior.

This is why event vocabulary matters so much. If the vocabulary is too small,
important differences collapse. If it is too large, memory becomes noisy and
hard to interpret. ApeSDK's event list sits in the middle: broad enough to
separate care, conflict, food, speech, mating, object use, and social gestures;
compact enough to remain readable.

## Role-Specific Meaning

Many events come in role pairs: acted and received, did and was-done-to. This
pattern is one of the most important parts of the memory model. The same moment
can mean different things depending on which side of it a being occupied.

For example, giving and receiving are not the same memory. Whacking and being
whacked are not the same memory. Grooming and being groomed are not the same
memory. The model needs both sides because social meaning is asymmetric.

Without role-specific events, a conflict could create the same memory in both
participants. That would erase the difference between aggression and victimhood,
between care and being cared for, between seeking and being sought. Social
behavior would flatten.

When adding a new interaction event, ask whether it needs one event or two. If
the actor and recipient should learn different things, the event vocabulary
should reflect that.

## Affect Is Direction, Not Poetry

The affect constants can look emotional, but their job is computational. They
turn events into directional pressure. Positive affect can make future approach
or preference more likely. Negative affect can make avoidance, fear, or reduced
trust more likely. Mixed affect can encode ambiguity.

The composite affect macro is intentionally compact. It does not model a full
psychological theory. It creates a single value from multiple named components,
which makes the source easier to discuss. A reader can see that birth includes
enjoyment, surprise, distress, fear, and shame-like terms, while grooming has a
different balance.

Those names are useful because they keep the model legible. But the
interpretation should stay grounded: affect is a value that influences future
behavior through code paths.

## Replacement Is Forgetting

Because episodic memory is bounded, replacement is a theory of forgetting. When
memory is full, the engine must choose what to overwrite. That choice shapes
personality-like behavior. A being that forgets old conflicts quickly may be
socially resilient. A being that retains strong negative events may avoid or
retaliate longer. A being that over-retains frequent minor events may miss rare
important ones.

Forgetting is often as important as remembering. An artificial life model with
perfect recall can become brittle. Old experiences dominate forever. A model
with no recall becomes reactive and shallow. Bounded episodic memory creates a
middle ground where history matters but does not become infinite.

When inspecting memory bugs, do not only ask whether an event was stored. Ask
what it displaced.

## Intention Events And Future Orientation

`EVENT_SEEK_MATE` is especially interesting because it records orientation
toward a future action. A being does not only remember what happened; it can
record that it began seeking. This turns memory into a bridge between past and
future.

Future-oriented memory is valuable because goals can fail. A being may seek a
mate and not find one. It may begin an approach and be interrupted by hunger,
fatigue, terrain, or another social event. The intention record can still
matter, because it says what the being was trying to do at that time.

This helps distinguish absence of outcome from absence of motivation. If mating
does not occur but seek-mate events are stored, the system is expressing the
drive but failing elsewhere. If seek-mate events are absent, the drive path may
not be opening.

## Memory And Speech

The episodic layer also connects naturally to language-like output. Speech in
ApeSDK should not be read only as audio or text. It is another way for internal
state and social history to become observable. If a being can speak or produce
speech-related output, memory and identity become part of that expression.

This connection suggests useful future work: command outputs or speech traces
could expose not just what a being says, but which recent memory or social entry
made that output likely. That would make language-like behavior easier to test.

## Memory Audits

A memory audit is a deterministic review of event storage and consequence. A
small audit can be more useful than a long run:

```text
initialize fixed state
choose two beings
force or trigger one event
inspect both episodic arrays
inspect social graph changes
advance a controlled number of cycles
check decay or replacement
check any goal or drive consequence
```

This kind of audit is how memory work should enter the canonical test suite.
The goal is not to prove that every long-run social behavior is fixed. The goal
is to prove that each memory mechanism does what it says.

## Designing New Events

A new event should have a reason to exist. Add one when an existing event cannot
express a meaningful behavioral distinction.

A good event proposal includes:

- event name;
- actor and recipient semantics;
- affect value;
- storage path;
- retrieval or influence path;
- command/log visibility;
- replacement behavior expectation;
- deterministic test.

Avoid adding events that are only synonyms. A larger vocabulary is only useful
when it improves interpretation or behavior.

## Reading Memory In A Long Run

Long-run memory analysis should not start with all beings. Start with one. Pick
a selected being and follow:

1. latest event;
2. oldest retained event;
3. strongest affect;
4. most repeated actor;
5. current goal;
6. social graph entry for the same actor;
7. next action after memory inspection.

This gives a narrative that is still grounded in data. Once one being is
understood, compare others.

## Case Study: A Conflict That Outlives The Moment

Imagine one being strikes another. The physical event is brief, but the memory
can last. The actor may store one event, the recipient another. Affect values
may differ. Later, the recipient may avoid, retaliate, fail to approach, or
prefer a different partner. The conflict has outlived the contact.

To analyze this, do not only count hits. Follow both memory arrays:

```text
before conflict: social graph and episodic state for both beings
event moment: actor event, recipient event, affect values
after conflict: drive state, goal state, social relationship
later cycles: avoidance, approach, or repeated conflict
```

The goal is to distinguish immediate physical response from remembered social
meaning. If behavior changes only in the next cycle, it may be state or drive.
If it changes much later around the same individuals, memory is a stronger
candidate.

## Case Study: Positive Events That Fail To Build Trust

Now imagine repeated grooming events do not produce stable association. Several
causes are possible. The affect value may be too weak. Memory replacement may
discard the events. Social graph familiarity may not update as expected. Hunger
or fatigue may keep interrupting future contact. The beings may simply not meet
often enough.

This case prevents a common error: assuming that positive events automatically
produce positive relationships. The model has multiple gates between event and
future behavior.

A useful audit includes:

- number of grooming events;
- whether each event stored in both beings;
- affect values;
- memory replacement between events;
- social graph attraction or familiarity;
- later co-location;
- competing negative events.

Only after those are known can the observer decide whether the memory model,
social graph, or environment needs adjustment.

## Memory And Population Narratives

Episodic memory also scales upward into population narratives. If many beings
remember the same individual through positive events, that individual may become
socially central. If many remember a region through food or conflict, that
region may become behaviorally charged. If memories decay quickly, population
patterns may remain fluid. If strong memories persist, history can harden into
structure.

This is one of the reasons artificial life becomes interesting over time. The
world is not only current terrain and current body state. It is accumulated
experience distributed across beings.

Population-level memory analysis can track:

- most remembered beings;
- most common event types;
- average affect by event;
- memory age distribution;
- repeated actor/recipient pairs;
- correlation between memory and social graph centrality.

These metrics turn "the group seems to remember" into concrete evidence.

## Event Vocabulary And Future Growth

The existing event vocabulary already covers many domains: food, reproduction,
conflict, care, object handling, communication, and social gestures. Future
events should preserve that breadth without becoming clutter.

Good additions might be justified by new mechanics:

- teaching;
- warning;
- sharing location;
- tending injury;
- tool construction;
- territorial display;
- illness contact;
- cooperative carrying.

Each of these would need actor/recipient semantics and affect. Some might also
need inventory, speech, immune, or territory links. A new event is rarely local.

## Command Output For Memory

Memory is hard to reason about unless it can be inspected. A strong episodic
command output should include:

- event name;
- actor and recipient names where relevant;
- affect;
- age or timestamp;
- location;
- argument or food item;
- whether it is an intention;
- maybe a derived positive/negative label.

The output should be readable enough that a user can compare two beings. If the
memory command is too raw, only maintainers will use it. If it is too summarized,
it may hide the evidence needed for debugging.

## Chapter Nine Checklist

Before moving on, make sure you can:

- distinguish event vocabulary from affect values;
- explain actor/recipient asymmetry;
- describe why bounded memory creates forgetting;
- inspect memory before and after an event;
- separate event storage from social graph update;
- design a deterministic memory audit;
- decide whether a new behavior needs a new event.

The next chapter shows how scripts can create controlled experiments around
these mechanisms.

## Open Design Questions

Memory can grow in several directions. The project could add richer affect
dimensions, more event roles, place-linked memories, stronger decay policies,
or memory-derived command summaries. Each direction should be evaluated against
clarity.

More affect dimensions may express subtlety, but they also increase tuning
burden. More event roles may preserve meaning, but they can make tests larger.
Place-linked memory may connect territory and social behavior, but it requires
careful coordinate handling. Stronger summaries can help users, but they must
not hide the raw event evidence.

The guiding question is: does this memory addition make future behavior more
explainable, or merely more elaborate?

## Practical Exercise

Choose one event type and write its complete evidence chain:

```text
trigger condition:
actor:
recipient:
event stored for actor:
event stored for recipient:
affect:
memory slot/replacement:
social graph effect:
later behavior likely affected:
command output needed:
test needed:
```

If any line is unclear, the event is not yet fully understood. This exercise is
especially useful before adding a new event or changing affect.

## Memory Vocabulary

Use these terms consistently:

- event is the encoded occurrence;
- affect is the directional value attached to it;
- actor is the being doing the action;
- recipient is the being receiving it;
- intention is a future-oriented memory;
- replacement is forgetting;
- decay is reduced influence over time;
- retrieval is memory affecting later behavior.

Clear vocabulary keeps memory from becoming vague storytelling.

## Reader Lab: Memory Timeline For One Being

Pick one being and build a memory timeline over a short fixed interval. The goal
is not to summarize its whole life. The goal is to learn how events accumulate.

Use a table like:

```text
time | event | actor | recipient | affect | location | later consequence
```

After filling the table, ask which events are likely to matter later. A recent
low-affect event may matter less than an older high-affect event. A repeated
minor event may matter through familiarity. A role-specific negative event may
matter only for one participant.

Then compare the timeline with the social graph. If the same names appear in
both, memory and social summary agree. If they differ, ask why. The being may
remember a strong event involving someone who is not socially central, or it may
have a strong social entry without a recent episodic event.

## Memory As The Bridge To Narrative

Memory is the layer most likely to tempt human storytelling. That is not a bad
thing. Narrative helps people understand agents over time. The discipline is to
keep narrative attached to event records.

A good narrative sentence looks like this:

```text
After EVENT_X with affect Y involving being B, being A later changed goal Z.
```

A weaker sentence says:

```text
Being A dislikes being B.
```

The second sentence may be a useful shorthand, but the first sentence is the
evidence. ApeSDK is strongest when readers can move between both levels.
