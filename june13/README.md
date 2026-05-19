# Simulated Universe Interface Proposal

This directory is a working proposal space for a new ApeSDK interface, currently
called **Simulated Universe** for discussion. The purpose is to imagine a modern
environment around the existing ApeSDK simulation that gives the user a more
direct, legible, and emotionally engaging way to observe and interact with the
simulated apes while preserving the scientific and deterministic qualities of
the engine.

The notes below are intentionally written as editable dot points. Some ideas may
be too ambitious, some may not fit the spirit of ApeSDK, and some may need to be
split into later stages. The aim is to create a starting proposal that can be
cut down, clarified, or redirected.

## Core Direction

- **Treat the existing ApeSDK as the simulation authority.**
  Simulated Universe should not become a separate model with different rules unless
  that is explicitly chosen later. The first principle should be that `sim`,
  `entity`, `universe`, `script`, `render`, and `toolkit` continue to define the
  world state. The new interface should reveal, explain, and interact with that
  state, not quietly replace it.

- **Create a new visual language for the world.**
  The interface should feel like a new way of seeing the simulated environment,
  not only a skin over the current presentation. Terrain, weather, water,
  social links, movement, memory, and individual ape state should have a richer
  visual identity. The goal is not photorealism for its own sake. The goal is a
  world that is easier to understand and more compelling to explore.

- **Move from passive viewing to active observation.**
  The user should be able to click, inspect, follow, compare, annotate, and
  influence the simulation through clear tools. Current ApeSDK already has deep
  internal state. Simulated Universe should make more of that state available in
  ways that a human can use without reading C source every time.

- **Preserve scientific credibility.**
  A richer interface must not turn the simulation into vague storytelling.
  Every visual, panel, or interaction should have a path back to source state:
  energy, drive, memory, location, event, weather, relationship, braincode,
  script, or transfer data. If the interface suggests an interpretation, the
  user should be able to inspect the evidence.

## Experience Goals

- **The first screen should be the living world.**
  The user should open the application and immediately see terrain, weather, and
  apes moving through the environment. Avoid a marketing-style landing page or a
  detached configuration screen as the first experience. Configuration can
  exist, but the world should be primary.

- **The interface should invite following an individual ape.**
  One of the strongest ways to make the simulation understandable is to let the
  user select an ape and follow its life. The selected ape could have a compact
  profile showing name, age, energy, drives, current state, recent events,
  social graph highlights, and current goal. This should feel like observation,
  not game character control.

- **The interface should support both overview and intimacy.**
  A user should be able to zoom out and see population movement, resource
  geography, weather, and social structure. The same user should be able to zoom
  in and inspect one being's body state, memories, relationships, and recent
  decisions. The value is in moving between scales.

- **The simulation should feel continuous even when inspected.**
  Panels, overlays, and tools should not make the world feel like it has stopped
  being alive. If the user pauses, that pause should be explicit. If the world
  continues, the inspection surfaces should update without becoming noisy or
  unreadable.

## Visual Environment

- **Use terrain as a readable ecological field.**
  Terrain should communicate height, water, shorelines, resource likelihood,
  sunlight, and perhaps territory familiarity. The current environmental model
  has many variables that can be visualized more clearly. A modern interface
  could offer different map modes rather than one overloaded view.

- **Represent weather as more than a color tint.**
  Weather could be shown through cloud movement, rain patterns, wind direction,
  lightning, dawn/dusk lighting, and water-level change. These effects should be
  visually pleasing, but also inspectable. For example, a weather panel could
  show the current weather value, tide level, time of day, and recent changes.

- **Make resources visible without making the world look artificial.**
  Resource overlays could be toggled on and off. In normal view, resources might
  be suggested through terrain texture, vegetation, water edges, or small
  markers. In analysis view, resource density could become a clearer heatmap.
  This keeps the world beautiful for ordinary viewing and precise for study.

- **Give apes readable bodies and states.**
  The ape representation does not need to be complex animation at first, but it
  should show enough state to matter. Movement, rest, eating, swimming,
  speaking, grooming, carrying, attacking, or seeking could have distinct poses,
  icons, or subtle motion language. A richer body display would make internal
  state visible before the user opens a panel.

- **Use overlays as instruments, not decorations.**
  Social links, territory, memory events, paths, weather, and braincode activity
  should be overlays the user selects intentionally. Each overlay should answer
  a question. The interface should avoid permanently covering the world in
  visual noise.

## Interaction With Simulated Apes

- **Selection should be central.**
  Clicking an ape should select it and open or update a side panel. The panel
  should identify the ape, show core state, and allow the user to follow, pin,
  compare, or inspect deeper details. The selected ape should be highlighted in
  the world without making all other apes irrelevant.

- **Following should create a life-thread view.**
  A "follow" mode could keep the camera near one ape and build a chronological
  feed of important events: ate fruit, entered water, met another ape, groomed,
  shouted, sought mate, gave object, changed territory, became hungry, stored a
  memory. This could be one of the main ways non-programmers understand ApeSDK.

- **Comparison should let the user inspect two or more apes.**
  Comparing apes could show energy, drives, social connection, shared memories,
  relationship values, genetic differences, and recent locations. This would be
  useful for understanding mate choice, conflict, kinship, and social drift.

- **User influence should be explicit and logged.**
  If the interface allows the user to intervene, the intervention should be
  marked as such. Examples might include changing weather, adding a resource,
  moving the camera only, selecting an ape, loading an ApeScript, or injecting a
  controlled event. Any intervention that changes simulation state should appear
  in a run log so later behavior is not mistaken for autonomous emergence.

- **Avoid direct puppeteering by default.**
  The interface should not begin as "control an ape like a player character."
  ApeSDK's value is autonomous simulated beings. Direct control could exist as a
  special experiment mode, but the default should be observation, inquiry, and
  structured intervention.

## Panels And Tools

- **Ape profile panel.**
  A selected ape panel could include name, age, sex, energy, body state, drives,
  location, current goal, inventory, recent event, family identity, and basic
  genetics. It should start compact and allow expansion.

- **Memory panel.**
  A memory view could list episodic events with time, event type, affect,
  involved beings, location, and argument. This would make the "what, when,
  where" structure of memory visible. It could also show whether a memory is
  self-focused, social, food-related, conflict-related, or intention-related.

- **Social graph panel.**
  A social view could show known beings, attraction, familiarity, relationship,
  friend/foe value, features, and last known location. The user should be able
  to select a social entry and see related events. This would connect summary
  social knowledge to episodic evidence.

- **Drive timeline panel.**
  A small timeline could plot hunger, social, fatigue, and sex drive over time
  for the selected ape. This would help explain why an ape did or did not act.
  It would be especially useful when a plausible behavior is absent.

- **Environment inspector.**
  Clicking terrain could show map-space coordinate, height, water, tide, weather
  values, resource classification, and nearby apes. This would make the
  landscape an inspectable model rather than a background image.

- **Run log and intervention log.**
  The interface should maintain a visible log of important events and user
  interventions. This could become part of saved run metadata and later support
  reproducible experiments.

- **ApeScript panel.**
  A scripting area could allow loading or selecting ApeScript files, showing
  parser results, errors, variables touched, and execution status. It should be
  designed as a safe experiment launcher rather than a hidden console.

## Modes Of Use

- **Explore mode.**
  The default mode. The world runs, the user selects and follows apes, toggles
  overlays, and learns what is happening.

- **Inspect mode.**
  A more analytical mode where time can be paused or stepped. Panels can expand,
  memory and social graph data can be compared, and the user can inspect exact
  values.

- **Experiment mode.**
  A controlled mode for interventions, scripts, fixed seeds, scenario setup,
  and logging. This mode should clearly mark the run as user-influenced.

- **Replay mode.**
  If feasible, saved runs or event logs could be replayed. Replay would be
  valuable for teaching, debugging, and comparing behavior after code changes.

## Technical Relationship To ApeSDK

- **Prefer a thin interface layer over a forked simulation.**
  The new interface should call into ApeSDK's existing simulation APIs where
  possible. If APIs are missing, add clear engine functions rather than reading
  internal structures chaotically from the UI layer.

- **Separate rendering state from simulation state.**
  Camera position, selected overlays, panel expansion, and UI preferences should
  not alter the simulation. Simulation-changing actions should go through named
  commands, scripts, or explicit intervention APIs.

- **Expose structured state for UI consumption.**
  Some current command outputs are human-readable text. Simulated Universe may need
  structured state snapshots for selected ape, social graph, memory, drives,
  environment sample, and run metadata. These should be designed carefully to
  avoid duplicating engine logic in the UI.

- **Keep deterministic testing in mind.**
  The interface can be visually rich, but core engine interactions should remain
  testable. If a UI button changes weather or loads a script, the underlying
  operation should be callable in tests without the UI.

## Possible Visual Directions

- **Map-rich 2D world.**
  A high-quality 2D visual environment may be the best first target. It can show
  terrain, water, weather, apes, trails, overlays, and panels without the cost
  of full 3D simulation or animation.

- **Layered cartographic style.**
  The world could look like a living scientific map: terrain texture, animated
  weather, moving beings, social overlays, and time-of-day lighting. This style
  would fit ApeSDK's scientific identity.

- **Close inspection portraits.**
  Selected apes could have simple portraits or silhouettes that show body state,
  inventory, age, and expression-like state. These should be derived from actual
  state, not random decoration.

- **Event glyphs.**
  Eating, grooming, conflict, speech, birth, swimming, and seeking could have
  small glyphs that appear briefly in the world and then enter the event log.
  This would connect visible action to episodic memory.

## MVP Proposal

- **First milestone: world view plus selection.**
  Display the simulation world, show apes, allow selecting one ape, and show a
  basic profile panel with live state.

- **Second milestone: overlays and inspection.**
  Add toggleable overlays for social graph, paths, resource density, weather,
  and territory. Add terrain click inspection.

- **Third milestone: memory and social panels.**
  Add readable episodic memory and social graph views for the selected ape.
  Connect social entries to related memories where possible.

- **Fourth milestone: run log and scripts.**
  Add event/intervention logging and an ApeScript loading panel with parser
  feedback.

- **Fifth milestone: experiment and replay support.**
  Add fixed-seed scenario setup, step controls, exportable run metadata, and
  replay or timeline review if feasible.

## Open Questions

- **Should Simulated Universe be a new macOS app, a cross-platform app, or a web
  interface?**
  A macOS app may fit the current toolchain history. A web interface may be
  easier to iterate visually and share. A cross-platform native app may be
  broader but more expensive.

- **How much should the user be able to intervene?**
  The safest default is observation plus explicit experiment mode. Direct
  manipulation of apes should be treated carefully because it can undermine
  autonomous behavior.

- **How realistic should the visuals be?**
  The proposal favors legibility over realism. Realistic art can be beautiful,
  but if it hides state or implies behaviors not present in the model, it may be
  misleading.

- **Should the interface teach the model?**
  It may be useful for panels to include short labels or tooltips, but the app
  should not become a tutorial wall. The world and inspection tools should teach
  through use.

- **How much historical Noble Ape material should be visible?**
  The interface could include a small history/about section, but the main
  experience should focus on the current simulation.

## Guiding Principle

- **Simulated Universe should make the apes more available to the user without
  making them less autonomous.**
  The user should feel closer to the simulated beings, better able to understand
  their environment and histories, and more capable of asking controlled
  questions. The interface should deepen the simulation rather than turning it
  into a conventional game or a decorative visualization.
