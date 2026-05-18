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
