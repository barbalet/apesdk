# Chapter 8 - A Neural Assembly Language: Braincode

Braincode is ApeSDK's compact control language for beings. The constants are in
[`sim/sim.h`](../sim/sim.h), the implementation is in
[`entity/brain.c`](../entity/brain.c), and command inspection is exposed through
[`universe/command.c`](../universe/command.c).

Braincode is intentionally small. That is its strength. A bounded instruction
set is easier to inspect, mutate, test, and explain than an opaque controller.

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
