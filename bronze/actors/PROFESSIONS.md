# Bronze professions and skills

The graphical host copies the declared `.bronze` vocations into the world
registry in DSL order. The vocation index is the actor’s job and skill ID.
Scenario selection stages a roster for the next world reset. Existing actors
keep their current roster until that reset takes effect.

## Authoring

Use `occupation_<vocation>_weight` in the `world` block to enable a vocation.
An enabled vocation requires `occupation_<vocation>_workplace`. Supported
requirements are `field`, `forest`, `mine_copper`, `mine_tin`, `service`, and
`coast`. Coastal jobs remain unassigned until a matching coastal workplace
exists. Requirements identify place types. Vocation names come from the
scenario.

| Setting in the `world` block | Default | Range |
| --- | ---: | --- |
| `occupation_<vocation>_weight` | 0 | 0–10,000 |
| `occupation_min_age_days` | 0 | 0–1,000,000 |
| `occupation_site_capacity` | 32 | 1–256 actors per workplace |
| `occupation_retry_minutes` | 60 | 1–1,440 native minutes |
| `occupation_skill_start` | 0 | 0–skill maximum |
| `occupation_skill_max` | 100 | 1–1,000 |
| `occupation_skill_gain` | 1 | 0–skill maximum per completed work unit |

The native age of maturity is the minimum working age. The scenario age
threshold can raise it. The graphical host accepts up to 64 uniquely named
vocations with names up to 63 bytes. Settings for unknown vocations, invalid
workplace types, fractional values and out-of-range values fail transactionally.

## Assignment and practice

Adults with a home can fill reachable, matching workplaces owned by their
township. Valid incumbents retain their roles and reserve their slots before
vacancies are filled. The allocator visits remaining actors in native
population order and minimizes the resulting worker-to-weight ratio among
roles with space. Equal ratios use DSL order. For a given role it prefers
the shortest dry route from home, then place order. Children, unhoused
actors, actors without a matching reachable workplace and actors facing
full workplaces have separate diagnostic reasons.

Reviews run at world creation and at the configured interval on the native
clock. Vacancies can be filled on the next review. Skills and recent
assignment history follow actor identity through compaction and session
switches. Actors retain previously learned vocation skills if they change
jobs. History keeps up to 32 vocation transitions.

The [task scheduler](../tasks/README.md) can gather when an assigned worker
runs the operation on their own production patch. Successful gathering adds
skill up to the configured cap.

The Bronze panel shows the selected being’s actor ID, vocation, current skill
and scale, home ID and workplace ID. Unassigned actors show their reason and
home ID. The native civilian profession helpers in `entity/profession` emit
social memory events and use a fixed civilian type enum. Bronze uses its
existing registry and work gate for assignment and skill accounting. Those
episodic helpers remain available for later service and mentorship integration.

A positive `occupation_mentor_skill` sets the skill threshold for
independent practice. Below it, completed work grants skill only if an
eligible worker of the same vocation and workplace is present at the work
site with skill at or above the threshold. Zero preserves independent
practice at every skill level. The threshold cannot exceed the skill cap.
At world creation, the first assigned worker of each vocation at each
workplace starts at least at the mentor threshold. Later hires use the
normal starting skill and retain any previously learned skill. Below the
threshold, the lowest-ID eligible mentor present at completion supervises
the practice. Reaching qualification records one native training interaction.
Supervision stores no persistent mentor reference. Native sleep, survival
needs and other work interruptions defer practice through the work gate.

Maturation makes an actor eligible on the next occupation review. A missing
workplace releases the assignment and its work reservations at review, and
previously learned skills and personal cargo follow the actor. Temporary
native incapacity pauses work and supervision while the actor retains their
slot. Death releases the slot and outstanding work claims. Personal cargo
and private household stock are booked as losses, and communal stock stays
with its township.

## Starting occupations

At world creation, at least `ceil(population × 0.60)` registered agents receive
an occupation selected from the scenario's enabled roles. Existing employed
agents count toward that minimum. Remaining labels are distributed in stable
native population order using the declared occupation weights; ties use scenario
order. Disabled roles are excluded, and a scenario with no enabled roles keeps
occupations disabled. Reviews retain valid labels and top up the minimum.

`BronzeActor.occupation` stores this social role separately from `vocation` and
`workplace`, which continue to describe detailed employment. A child or an agent
without housing or an available workplace can display a selected occupation
without becoming a worker. Labels do not grant skill, inventory, production,
mentor qualification or task execution. Once an agent has a detailed job, its
occupation label follows that job. The inspector shows the selected role and
its current eligibility reason instead of presenting every non-worker as
unassigned. Saves preserve these labels in sidecar schema 3.
