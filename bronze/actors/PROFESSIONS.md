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

Prototype gathering requires a valid assigned worker on their own production
patch and a grant from the existing native work gate. Successful gathering
adds skill up to the configured cap. A service assignment has a workplace
but awaits task dispatch. Automatic township tool crafting is suspended
until the task executor can attribute the action to a worker.

The Bronze panel shows the selected being’s actor ID, vocation, current skill
and scale, home ID and workplace ID. Unassigned actors show their reason and
home ID. The native civilian profession helpers in `entity/profession` emit
social memory events and use a fixed civilian type enum. Bronze uses its
existing registry and work gate for assignment and skill accounting. Those
episodic helpers remain available for later service and mentorship integration.
