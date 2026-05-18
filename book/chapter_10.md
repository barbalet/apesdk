# Chapter 10 - Language As Computation, Not Narrative

ApeScript is the intervention language in ApeSDK. Its source lives in
[`script/script.h`](../script/script.h), [`script/parse.c`](../script/parse.c),
and [`script/interpret.c`](../script/interpret.c). Example scripts live in
[`script_examples/`](../script_examples/), and deterministic unit tests live in
[`script/unit`](../script/unit/).

ApeScript is useful because it lets a user express controlled changes without
immediately editing the C engine. That makes it a bridge between experiment and
implementation.

## Noble Ape Scripting Context

The [Noble Ape 2012 talk](https://nobleape.com/) describes ApeScript as an
earlier C-like scripting language related to the simulation's language and
braincode experiments. That is useful context for the current `script/`
directory. ApeScript is not an accidental utility bolted onto the side of the
project; it belongs to the long-running effort to make hypotheses executable
without forcing every experiment into compiled engine code.

The transcript also raises the question of whether language forms need more
complexity than compact bytecode. That question remains relevant. The current
book separates braincode and ApeScript because they serve different levels of
control, but historically they are part of the same design conversation.

## The Contract

`script.h` defines the primary script terms:

- function;
- run;
- while;
- if.

It also defines syntax tokens for braces, parentheses, operators, numbers, text,
semicolons, functions, run blocks, and strings. The parser and interpreter are
strict by design. A language used for experiments should fail clearly when the
experiment is malformed.

## Errors Are Part Of The Interface

The `AE_ENUM` error list is one of the most important parts of ApeScript. It
includes errors for:

- number expected or out of range;
- maximum numbers, variables, or script size;
- selected entity or coordinates out of range;
- too many braces;
- missing equals;
- missing semicolon;
- code outside functions;
- function definition problems;
- no main code;
- output variables incorrectly set as input.

That level of detail makes scripts debuggable. It also makes scripts testable:
a bad script should fail the same way every time.

## Script Examples

The current examples include:

- `most_basic.txt`;
- `test_operator.txt`;
- `is_visible.txt`;
- `brain_flash.txt`;
- `emulate_rand_brain.txt`;
- `infinite_loop.txt`.

These examples are part of the learning path. They show how a small text file
can probe parser behavior, visibility, operators, brain-related state, and
runtime protections.

## Script Versus C

Use ApeScript when the change is experimental orchestration:

- set or inspect variables;
- test a small control idea;
- compare a scenario against a fixed seed;
- create a reproducible example for a bug.

Use C when the change is a durable model feature:

- new body state;
- new event type;
- new transfer field;
- new braincode instruction;
- new deterministic unit fixture;
- performance-critical loop behavior.

Script is fast to iterate. C is where stable engine behavior belongs.

## Deterministic Script Workflow

A reliable script experiment should have:

```text
script file path
ApeSDK version
seed or saved input state
expected parser result
expected runtime result
metrics to compare
```

Then run the same script through the unit path or command path until failure is
repeatable. If a script failure changes from commit to commit without code
reason, the test is flaky and should not be part of the canonical suite.

## Practical Test Boundary

The current project-level runner includes `script/unit/script_tests.sh` and the
compiled `script_tests` binary. Treat that as the deterministic script contract.
Other older scripts and broad smoke checks can still be useful during manual
investigation, but they should not be mistaken for stable CI unless they are made
deterministic.

## Questions For The Reader

- Does the script fail at parse time or runtime?
- Does the error class identify the actual authoring mistake?
- Does the script need engine access that should instead be a C API?
- Can the scenario be reduced to a small fixed-seed fixture?
- Does a script change alter model behavior, or only experiment setup?

## Scripts As Executable Method Sections

ApeScript is best understood as an executable method section. In a paper, a
method section describes what was done. In ApeSDK, a script can make that
description runnable. That is powerful because it reduces the gap between an
idea and a repeatable intervention.

The language is intentionally constrained. This matters. A permissive scripting
system can let users do almost anything, but "almost anything" is difficult to
test. ApeScript's grammar, variable rules, main function requirement, and error
taxonomy keep scripts closer to controlled protocols.

The goal is not to make the language fashionable. The goal is to make it
trustworthy enough that a script file can travel with a run and explain what was
changed.

## Parser Strictness Protects Experiments

Strict parsing can frustrate new users, but it protects the model. A missing
semicolon, unmatched brace, invalid variable, or number out of range should fail
early. Silent acceptance is dangerous because it can create the illusion that an
experiment ran as intended.

This is why the error enum is so detailed. Each class of failure narrows the
search. "No main code" is different from "code outside function." "Output set as
input variable" is different from "coordinates out of range." The script layer
does not merely say "bad script." It tries to say what kind of bad script.

That distinction becomes important when scripts are used by more than one
person. A repeatable error message is a teaching tool. It helps users learn the
language and helps maintainers identify where documentation or validation needs
work.

## Main As A Protocol Boundary

The requirement that ApeScript have main code is more than a syntax rule. It
gives the experiment a boundary. Code before main can define functions or setup
concepts, but execution has a clear entry point. That mirrors good scientific
practice: the protocol should say where the intervention begins.

Without an entry boundary, scripts can become collections of side effects. With
one, a reader can ask:

- what is defined;
- what is executed;
- what order execution follows;
- what variables are inputs;
- what outputs or side effects are intended.

This makes scripts easier to review and easier to test.

## Variables As Permissions

Script variables should be read as permissions. An input variable is not the
same as an output variable. A special variable is not the same as a normal local
value. The access rules define what a script is allowed to observe or change.

This protects the engine from accidental corruption. If a script could mutate
any internal field at any time, it would be difficult to know whether a behavior
came from the model or from a script side effect. By constraining variables,
ApeScript keeps experiments more honest.

When adding script access to a new engine value, ask:

- should scripts read it;
- should scripts write it;
- should it be range checked;
- should it be per-being or global;
- should writes be logged;
- should it persist;
- should tests cover invalid access?

Every new script variable is a public experimental API.

## Examples As A Curriculum

The files in `script_examples` are small, but they can form a curriculum. A new
reader can start with `most_basic.txt` to see minimum structure, then move to
operators, visibility, brain-related examples, random brain emulation, and loop
protection.

Good examples should teach one thing at a time. They should not be clever. A
script example that demonstrates five features at once is harder to debug and
harder to use as a fixture.

Future examples could be organized by purpose:

- syntax basics;
- variable access;
- coordinate checks;
- visibility checks;
- braincode interaction;
- event-triggered behavior;
- failure examples with expected errors.

Failure examples are especially useful because they show the language's safety
rails.

## Script Tests And Flakiness

Scripts can be tempting sources of flaky tests. A script that depends on timing,
random long-run behavior, GUI state, or uncontrolled simulation state may fail
differently across runs. That kind of test should remain exploratory until it
can be reduced.

A deterministic script test should control:

- script text;
- initial state;
- seed;
- expected parser result;
- expected error or output;
- execution limit;
- variables inspected.

If the expected result is "something interesting happens eventually," it is not
a unit test. It may be a demo, scenario, or manual smoke check, but it should
not be part of the canonical deterministic runner.

## When A Script Should Become C

Scripts are excellent for experiments, but successful experiments sometimes need
to become engine features. The transition point arrives when a script is no
longer testing a question but expressing a stable model rule.

Move logic into C when:

- it must run every cycle for many beings;
- it needs direct access to internal structures;
- it affects save/load format;
- it needs deterministic unit tests below the script layer;
- it has become part of the model rather than a temporary intervention.

Do not move logic into C merely because the script looks inelegant. First ask
whether the script is doing too much. Sometimes the right answer is a smaller
script, not a larger engine.

## Script Provenance

Every serious run that uses ApeScript should record script provenance:

```text
script file name
script content hash
ApeSDK version
seed or saved input
parser result
runtime result
metrics collected
```

This protects historical comparison. If a script changes and the version does
not, old results become hard to interpret. A script is part of the experiment,
not a casual input.

## Reader Exercise

Take the smallest example script and intentionally break it in three ways:

1. remove a semicolon;
2. remove the main function;
3. write an out-of-range number.

Then compare the error classes. This exercise teaches how the parser thinks.
Only after understanding failure should a reader rely on scripts for successful
experiments.

## Case Study: A Script That Changes Nothing

One of the most useful script failures is a script that runs but changes
nothing. This can happen for several reasons. The script may set a value that is
immediately overwritten by engine logic. It may target the wrong entity. It may
execute before the relevant state exists. It may change an input that is not
actually consumed by the behavior being observed.

The debugging process should be systematic:

```text
confirm parser success
confirm main executed
confirm target entity
confirm variable write
confirm engine reads that variable later
confirm expected output or state transition
```

If the script wrote a value but behavior did not change, the script has taught
something important: the hypothesis targeted the wrong lever. That is a good
scientific result when it is understood.

## Case Study: A Script That Changes Too Much

The opposite failure is a script that changes too much. A broad intervention may
alter multiple variables, making the result impossible to attribute. For
example, a script that changes location, energy, and social state at once may
produce a visible behavior change, but the cause is unclear.

A better pattern is a script ladder:

1. baseline script that does nothing except run;
2. one-variable intervention;
3. same intervention with a small range of values;
4. second variable added only after the first is understood;
5. combined intervention with explicit expectation.

This turns scripts into controlled experiments instead of magic spells.

## Scripts And Commands Together

ApeScript becomes more powerful when paired with commands. The script changes
state; commands inspect the result. For example, a script might alter a value
related to visibility, then commands inspect selected being state, social graph,
episodic memory, or probes.

This pairing creates a tight loop:

```text
write script
run fixed seed
inspect command output
adjust script
run again
compare transcript
```

The transcript becomes part of the experiment. It shows not only that the script
ran, but what the engine reported after it ran.

## Script Versioning

Scripts need versioning discipline even if they are small text files. A one-line
script change can invalidate previous results. If a script is used in a serious
run, store it with the output or record a content hash.

Useful script metadata includes:

- file name;
- description;
- ApeSDK version expected;
- variables touched;
- expected parser result;
- expected behavioral direction;
- author or source;
- date last changed.

This may become a simple header comment convention in the future. The important
thing is that scripts not become anonymous experimental inputs.

## Testing Error Messages

Error messages should be tested because they are part of the user interface.
When an invalid script produces the wrong error, the parser may still reject the
script, but the user learns the wrong lesson.

Good negative tests include:

- missing semicolon;
- missing main;
- extra close brace;
- invalid variable assignment;
- out-of-range number;
- code after main;
- while or if without braces.

Each should fail deterministically with the expected error class. These tests
make the scripting layer friendlier and safer.

## Chapter Ten Checklist

Before moving on, make sure you can:

- explain why ApeScript is an intervention layer;
- distinguish parser failure from runtime failure;
- understand variable access as permission;
- use examples as small lessons;
- pair scripts with command output;
- keep flaky script scenarios out of canonical tests;
- decide when a successful script should become C.

The next chapter focuses on how all this state becomes observable to a human.

## Open Design Questions

ApeScript could grow more expressive, but expressiveness is not free. More
syntax means more parser cases, more error messages, more tests, and more ways
for scripts to affect the engine. The question should always be whether a new
language feature supports clearer experiments.

Possible future improvements include better script metadata, a standard library
of safe helper functions, richer examples, command integration, and clearer
script result reports. Each would help users if implemented conservatively.

The risky direction is making ApeScript a second full engine. If too much model
logic moves into scripts, the C source stops being the clear center of truth.
Scripts should intervene, orchestrate, and test. Durable model rules should
remain in the engine.

## Practical Exercise

Write a tiny script proposal without coding it:

```text
Goal:
Variables read:
Variables written:
Expected parser result:
Expected runtime state:
Command used to inspect result:
Deterministic test idea:
Reason this belongs in script instead of C:
```

If the final line is hard to answer, the idea may belong in engine code or in a
manual experiment rather than ApeScript.

## Script Vocabulary

Use these terms consistently:

- parser means syntax and structure handling;
- interpreter means execution;
- variable class means access permission;
- main is the execution entry;
- script provenance is identity of script content;
- scenario is the controlled setup the script participates in;
- deterministic script test is a fixed expected parser or runtime result.

This vocabulary makes script discussions precise.

## Reader Lab: Reducing A Scenario

Take a broad experimental idea and reduce it to the smallest scriptable
question. For example, "make beings more social" is too broad. A better
question is: "If a selected variable related to social pressure is adjusted
under a fixed seed, does the social graph change within a fixed interval?"

The reduction process is:

```text
remove unrelated variables
choose one seed
choose one selected being or fixed group
choose one script intervention
choose one command output
choose one expected direction
choose one failure condition
```

This process often reveals that the original idea was underspecified. That is a
success. Scripts should force ideas to become executable.

## Script Reports

A useful future script runner would produce a small report:

```text
script loaded:
parse result:
main entry:
variables touched:
execution count:
errors:
selected metrics before:
selected metrics after:
```

Such a report would make ApeScript more accessible and easier to include in
experiments. It would also help separate parser success from behavioral success.

Until such tooling exists, users can approximate it with careful command
transcripts and fixed run notes.

## Why Scripting Belongs After Memory

This chapter follows the memory chapter because many useful scripts manipulate
conditions whose effects appear through event history. A script can alter a
state, but the model becomes interesting when that alteration creates or changes
events, social entries, and later decisions. Scripts are therefore best read as
controlled disturbances in a remembered world.

## Final Reader Questions

Before relying on a script, ask a short set of final questions:

- Can another person run the same script from a clean checkout?
- Does the script declare or imply the ApeSDK version it expects?
- Does it fail clearly when written incorrectly?
- Does it change one concept or many?
- Is the result inspected through commands, tests, or logs?
- Is the script still an experiment, or has it become engine behavior?

These questions keep ApeScript in its proper role. It is a powerful way to ask
questions of the model. It should not become an untracked place where model
rules hide.

The best scripts are modest. They make one idea executable, one failure obvious,
and one comparison repeatable.
