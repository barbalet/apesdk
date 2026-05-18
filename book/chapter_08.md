# Chapter 8 - A Neural Assembly Language: Braincode

Braincode is ApeSDK's compact control language for beings. The constants are in
[`sim/sim.h`](../sim/sim.h), the implementation is in
[`entity/brain.c`](../entity/brain.c), and command inspection is exposed through
[`universe/command.c`](../universe/command.c).

Braincode is intentionally small. That is its strength. A bounded instruction
set is easier to inspect, mutate, test, and explain than an opaque controller.

## Noble Ape Language Context

The [Noble Ape 2012 talk](https://nobleape.com/) treats braincode as more than
an internal control mechanism. It describes internal and external language-like
representations, social representations of other apes, and code that can be run
against internal models. That history helps explain why this chapter calls
braincode a neural assembly language rather than only a bytecode.

In the current ApeSDK source, braincode remains compact and bounded. The Noble
Ape reference reminds the reader what that compactness was trying to support:
not just movement control, but a way for internal representation, social
association, and communication-like behavior to share a computational form.

## The Budget

Current constants include:

- `BRAINCODE_SIZE` - 128 bytes.
- `BRAINCODE_PROBES` - derived from the braincode size.
- `BRAINCODE_PSPACE_REGISTERS` - three registers.
- `BRAINCODE_MAX_FREQUENCY` - 16.
- `BRAINCODE_BYTES_PER_INSTRUCTION` - three bytes.
- `BRAINCODE_BLOCK_COPY` - 16 instructions copied by `MVB`.
- `BRAINCODE_MAX_ADDRESS` - twice the braincode size.

These are not arbitrary limits. They define how much control complexity a being
can carry.

## Instruction Families

The instruction enum includes:

- data operations: `DAT0`, `DAT1`;
- arithmetic and data movement: `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `MVB`,
  `MOV`, `JMP`, `CTR`, `SWP`, `INV`, `STP`, `LTP`;
- conditionals: `JMZ`, `JMN`, `DJN`, `AND`, `OR`, `SEQ`, `SNE`, `SLT`;
- sensors: `SEN`, `SEN2`, `SEN3`;
- actuators: `ACT`, `ACT2`, `ACT3`, `ANE`.

This gives a strict loop:

```text
read a sensor or internal value
transform it
branch or copy
write to an actuator or internal state
repeat inside the budget
```

## Brain Versus Braincode

`entity/brain.c` also contains a larger brain array update model. The comments
describe the basic formula for waking and sleeping mind updates over the brain
state. Braincode is the compact symbolic layer attached to that broader
machinery through probes, registers, and commands.

This distinction matters:

- the brain array is a field-like state update;
- braincode is an inspectable program;
- probes connect code and state;
- commands and logging make the result observable.

## Genetic Bias

`entity/entity.h` contains macros that bias initial braincode instruction types:

- `GENE_BRAINCODE_SENSORS`;
- `GENE_BRAINCODE_ACTUATORS`;
- `GENE_BRAINCODE_CONDITIONALS`;
- `GENE_BRAINCODE_OPERATORS`;
- `GENE_BRAINCODE_DATA`.

That means inherited variation can alter not just body traits but controller
style. One lineage may generate more sensor-heavy code while another generates
more actuator-heavy code.

## Inspection

The command surface includes `braincode` and `probes` commands. These are
essential because a controller that cannot be inspected becomes a source of
mystery rather than science.

When debugging a behavior, collect:

- instruction distribution;
- branch counts;
- sensor reads;
- actuator writes;
- probe values;
- goal transitions;
- selected being state before and after the cycle.

## Braincode Change Protocol

A braincode change should be treated like an instruction set change:

1. Verify parsing or generation still produces valid instructions.
2. Check address wrapping.
3. Check frequency bounds.
4. Run deterministic unit tests where available.
5. Inspect command output for one selected being.
6. Only then compare long-run behavior.

Small controller changes can produce large behavioral differences. That does not
make them bad. It means they need tight evidence.

## Why A Tiny Language Matters

Braincode is one of ApeSDK's most distinctive ideas because it chooses a small,
assembly-like control surface instead of a large opaque behavior engine. That
choice has consequences for how the whole project should be read.

A tiny language makes behavior inspectable. If a being acts differently, the
reader can ask which instruction family, address, probe, or frequency changed.
That does not make debugging trivial, but it keeps the evidence close to the
mechanism. The alternative would be a large hidden policy where behavior changes
are difficult to attribute.

The smallness also creates evolutionary pressure. A being cannot carry infinite
logic. Braincode has to be compact. It has to reuse addresses, branch, sense,
act, and mutate within a fixed budget. That budget becomes a cognitive
constraint, much like bounded episodic memory or bounded social graph size.

## Programs As Bodies Of Habit

It is helpful to think of braincode as habit-like control rather than
deliberative planning. A braincode program does not write essays about goals. It
executes small operations that connect sensed state to action. That makes it
closer to reflex, routine, or micro-policy.

This is a good fit for ApeSDK because many behaviors should arise below the
level of explicit scripting. Movement adjustment, probe response, and actuator
selection can happen through compact repeated programs. Higher-level
experiments can remain in ApeScript or C.

The separation helps keep layers clear:

- C defines the engine and durable model.
- Braincode defines compact being-level control.
- ApeScript defines experiment-level intervention.
- Commands expose and inspect what happened.

When those layers blur, debugging becomes harder.

## Branches Create Behavioral Phases

Conditionals such as `JMZ`, `JMN`, `DJN`, `SEQ`, `SNE`, and `SLT` create
discrete branch points. A tiny branch change can produce a phase shift: the same
being may loop, stall, copy a block, skip an actuator, or sample a different
sensor.

This is one reason braincode changes should be tested with traces. A final
behavior count may reveal that something changed, but an instruction trace can
show where it changed. Did the program branch earlier? Did it reach an actuator
less often? Did a counter decrement to zero at a different moment? Did address
wrapping make an instruction repeat?

Small programs can still have complex dynamics when branches and state interact.

## Probes As The Bridge Between State And Program

Probes give braincode access to the larger being and world state. They can inject
or detect signals at particular positions, addresses, and frequencies. This is a
critical bridge. Without probes, braincode would be isolated computation. With
probes, it becomes coupled to body, environment, and behavior.

Probe frequency deserves special attention. A fast probe can react to transient
state but may amplify jitter. A slow probe can smooth noise but miss brief
opportunities. The maximum frequency constant therefore shapes responsiveness.

When investigating a behavior, probe state can answer questions that high-level
logs cannot:

- what did the program have access to;
- when did it sample that access;
- what address did it influence;
- did an actuator fire after the probe changed;
- did a branch depend on the probed value?

## Genetic Generation Of Control Style

The braincode instruction balance can be biased by genetics. This is subtle and
important. It means control style is heritable in a limited way. One being may
begin with more sensor-heavy code, another with more actuator-heavy code, and
another with more conditional structure.

That inherited control variation can interact with learned and environmental
differences. A being with sensor-rich control might respond differently to the
same terrain than one with a different instruction distribution. Over long runs,
this can create lineage-level differences without requiring a full learning
algorithm.

The reader should be careful not to overclaim. A gene-biased instruction
distribution is not the same as a fully evolved mind. It is a mechanism for
initial variation and mutation to matter.

## Braincode And The Larger Brain State

The brain array update in `brain.c` gives the system another kind of internal
dynamics. Its comments describe relationships between current state, previous
state, constants, and waking/sleeping behavior. This is not identical to
braincode, but the two are conceptually connected through probes and state.

The larger brain state can be read as a field of activation. Braincode can be
read as compact symbolic operations. Together they offer two modes of cognition:
a continuous-ish internal field and an instruction-based control language.

That combination is unusual and worth preserving carefully. If future work
simplifies one side, it should explain what interpretive power is lost or gained.

## Testing A Braincode Change

Braincode tests should be small and mechanical before they become behavioral.
For example:

1. Does the mnemonic formatter name the instruction correctly?
2. Does address wrapping behave correctly?
3. Does a branch jump to the expected place?
4. Does block copy preserve bounds?
5. Does a probe frequency stay within limits?
6. Does a generated program contain valid instruction families?

Only after those tests pass should a long-run behavior be interpreted. Otherwise
the run may be measuring a broken virtual machine rather than a meaningful
controller.

## A Useful Trace Format

A future trace for one selected being might record:

```text
time
being name
instruction pointer
instruction mnemonic
operand bytes
register state
probe state
branch result
actuator result
goal before
goal after
body state before
body state after
```

Even a short trace like this can make braincode behavior much less mysterious.
It turns "the being acted strangely" into "the program branched at instruction
17 because probe 3 changed."

## When Not To Use Braincode

Braincode is powerful, but not every behavior belongs there. Use it for compact,
repeated, being-level control. Do not use it for broad scenario setup, release
logic, data import, or large experiment orchestration. Those belong in scripts,
commands, tests, or C.

Layer discipline keeps the project readable. Braincode should make beings more
interesting, not make the whole engine harder to reason about.

## Case Study: A Branch Swap That Looks Like Personality

Imagine a small mutation changes a conditional branch. The being now turns away
from a stimulus it previously approached. An observer might describe this as a
personality change. At the braincode layer, it may be a branch polarity change.

The right analysis asks:

- which instruction changed;
- which probe or value the branch tested;
- which address was reached before;
- which address is reached now;
- which actuator fires after the branch;
- whether the goal or body state changed downstream.

This does not make the behavior less interesting. It makes it more explainable.
A small symbolic program can produce a large behavioral shift precisely because
it gates action.

## Case Study: Probe Frequency And Missed Opportunity

Suppose a being fails to respond to a brief nearby event. The social or sensory
code may be fine. The probe may simply not sample at the right frequency. A
slower probe smooths behavior but misses short windows. A faster probe catches
more but may create jitter.

This is a classic control tradeoff. Responsiveness and stability often fight.
`BRAINCODE_MAX_FREQUENCY` defines the upper boundary, but individual probe
configuration determines how that boundary is used.

A test scenario for probe frequency should include an event with a known time
window. Run the same setup with different frequencies and compare detection,
branch behavior, and actuator output. The result will teach whether the behavior
depends on control timing rather than high-level decision logic.

## Braincode As A Debugging Surface

Braincode is not only a behavior mechanism. It is a debugging surface. Because
instructions have mnemonics and bounded addresses, they can be printed,
compared, counted, and traced.

Useful diagnostics include:

- instruction family counts;
- invalid instruction checks;
- branch destination histograms;
- actuator fire counts;
- sensor read counts;
- block-copy events;
- probe frequency distribution;
- per-being program digests.

A digest is especially useful in deterministic tests. If two runs should produce
the same braincode state, a digest can catch divergence without dumping every
byte.

## Mutation And Safety

Any system that mutates compact programs needs safety rules. Mutations should
not create out-of-bounds addresses, invalid instruction families, impossible
probe states, or non-terminating behavior that overwhelms the simulation. Bounds
and modulo address helpers are part of that safety.

Mutation is valuable because it creates variation. Safety is valuable because it
keeps variation inside interpretable limits. The balance is the design.

When changing mutation logic, test:

- instruction validity after mutation;
- address wrapping;
- distribution of instruction families;
- absence of runaway execution;
- deterministic behavior under fixed seeds.

## Braincode And Inheritance

Because braincode composition can be genetically biased, it can participate in
lineage. This opens a path for inherited behavioral tendencies without requiring
a massive learning architecture. A lineage may tend toward more sensing, more
acting, more conditional control, or different data movement patterns.

The interesting question is not whether one instruction family is "better." It
is whether a distribution fits an environment. Sensor-heavy programs may do well
in information-rich contexts. Actuator-heavy programs may be more decisive but
less responsive. Conditional-heavy programs may adapt but become fragile if
branches depend on noisy probes.

Future experiments could compare instruction family distributions across
environments while holding seeds and body constants controlled.

## Braincode Documentation For Users

If users are expected to inspect braincode, documentation should provide a small
mnemonic guide. Each instruction family should have:

- mnemonic;
- rough purpose;
- operand interpretation;
- common failure mode;
- example trace line.

This chapter gives the conceptual introduction, but a future reference appendix
could make braincode easier to learn. A compact language becomes more powerful
when users can read it fluently.

## Chapter Eight Checklist

Before moving on, make sure you can:

- name the instruction families;
- explain why `BRAINCODE_SIZE` is a cognitive budget;
- distinguish brain array state from braincode program state;
- explain what probes connect;
- trace a branch from sensor to actuator;
- design a deterministic braincode instruction test;
- decide whether a behavior belongs in braincode, ApeScript, or C.

The next chapter turns from compact control to remembered experience.

## Open Design Questions

Braincode could grow in several directions: more instructions, richer probes,
better tracing, mutation controls, debugging views, or reference programs. Each
extension should protect the core value of the system: compact inspectable
control.

Adding instructions increases expressiveness but makes programs harder to reason
about. Adding probes increases access to state but can blur layer boundaries.
Adding mutation options increases variation but can make deterministic testing
harder. Adding tracing improves understanding but may create performance costs.

The safest improvements are those that make existing behavior more observable
before making the language larger.

## Practical Exercise

Select one braincode program and count instruction families:

```text
data:
operators:
conditionals:
sensors:
actuators:
```

Then compare the counts with the being's recent behavior. Does a sensor-heavy
program appear more reactive? Does an actuator-heavy program attempt more direct
actions? Do conditional-heavy programs show more branch-sensitive behavior?

This exercise should be done cautiously. One program is not proof. But it builds
intuition for how compact code can create visible tendencies.

## Braincode Metrics

Useful metrics for future tests or reports include:

- instruction family distribution;
- valid instruction percentage;
- branch execution count;
- actuator execution count;
- probe read/write count;
- address wrap count;
- mutation count;
- program digest by being.

These metrics would make braincode easier to compare across versions and seeds.

## Braincode Vocabulary

Use these terms consistently:

- instruction is one symbolic operation;
- program is the bounded instruction memory;
- probe is a bridge to brain or world state;
- actuator is an output operation;
- sensor is an input operation;
- branch is control-flow choice;
- frequency is timing of probe or execution influence.

With this vocabulary, braincode becomes a readable subsystem rather than a
mysterious inner machine.

## Future Exercise: Program Lineage

Braincode becomes especially interesting when compared across related beings or
runs. A future program-lineage exercise would choose a parent-like source
program and compare descendant or mutated programs by digest and instruction
family distribution.

The questions would be:

- which bytes changed;
- which instruction families changed;
- did branch structure change;
- did probes change;
- did actuator use change;
- did behavior change in the expected direction?

This would connect genetic or mutation logic to observable controller change.
It would also make braincode evolution easier to discuss without relying only on
visual behavior.

## Negative Results In Braincode

Sometimes a braincode mutation changes bytes but not behavior. That can be a
valid result. The changed instruction may be unreachable. The probe may not fire.
The branch may lead to equivalent output. The drive or body state may block the
actuator. The environment may never present the condition needed to reveal the
change.

This is why braincode needs traces and not only final behavior counts. A
non-effect can still teach where the active control surface really is.

## Reader Lab: From Mnemonic To Movement

Choose one instruction mnemonic from a selected being's braincode and follow its
possible path to behavior. The exercise does not require proving the whole
program. It asks the reader to connect one symbolic operation to the larger
system.

Write down:

```text
mnemonic:
instruction family:
operands:
address:
nearby branch instructions:
probe or register involved:
possible actuator:
body or goal state that could block it:
command output that could reveal it:
```

This makes braincode concrete. Instead of seeing a byte array, the reader sees a
small machine embedded inside a body. The exercise also exposes where more tools
would help: traces, command summaries, and per-being program digests.

## Why Braincode Belongs In The Book

Braincode can look specialized, but it expresses a central ApeSDK philosophy.
Behavior should be rich without becoming inaccessible. A compact instruction
language lets the project explore adaptive control while preserving a path back
to source, state, and test.

If future versions alter braincode, this chapter should be updated immediately.
The controller vocabulary shapes how readers understand agency in the model.
