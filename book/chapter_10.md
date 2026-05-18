# Chapter 10 - Language As Computation, Not Narrative

ApeScript is the intervention language in ApeSDK. Its source lives in
[`script/script.h`](../script/script.h), [`script/parse.c`](../script/parse.c),
and [`script/interpret.c`](../script/interpret.c). Example scripts live in
[`script_examples/`](../script_examples/), and deterministic unit tests live in
[`script/unit`](../script/unit/).

ApeScript is useful because it lets a user express controlled changes without
immediately editing the C engine. That makes it a bridge between experiment and
implementation.

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
