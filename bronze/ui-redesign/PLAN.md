# BronzeSim workspace redesign

Deliver the live BronzeSim workspace shown in the
[workspace mockup](reference/bronzesim-workspace-concept.png) for the
`bronze-mac` Mac Catalyst target. The mockup is the visual acceptance
reference for the window chrome, overhead map, perspective terrain, and
inspector. [Shot 27](reference/shot-27.png) supports the overall composition,
[shot 26](reference/shot-26.png) illustrates file operations, and
[shots 04](reference/shot-04.png) and [29](reference/shot-29.png) inform
legibility at different sizes. All world imagery must come from the running
simulation. Match the mockup's composition, detail, and material treatment
using the active scenario's geography, sites, and actors.

This plan contains 30 cycles. Complete each cycle's acceptance check before
starting work that depends on it. Record the scenario, platform, and any
deferred cases. Cycle 24 qualifies the existing preview as a baseline.
Cycles 25–29 deliver and qualify mockup fidelity, and cycle 30 makes that
workspace the Bronze default.

## Bronze-only scope

- Compile the new workspace and its UI support only into `bronze-mac`.
  Keep production Swift under [bronze/ui](../ui), with this directory for
  design references and planning.
- In the [shared app shell](../../ios/ApeSimApp.swift), keep integration
  points inside `#if BRONZE_SIM`. If a shared C hook is necessary, keep it
  small and inside `#ifdef BRONZE_SIM`. Historical ApeSDK targets are
  regression checks only.
- Reuse the current session, input routing, scenario controller, and file
  operations. Use the existing render surfaces as the integration point
  for the Bronze map and terrain renderers. Each workspace view observes one world.
  The existing engine owns the clock, positions, and simulation state.
- Preserve scenario behavior, selection identity, save compatibility, and
  the existing BronzeSim menus and separate window layout during rollout.
  See the [Bronze implementation plan](../PLAN.md) and
  [save format](../SAVE-FORMAT.md).

## Foundation

See [foundation notes](FOUNDATION.md) for the baseline, command inventory,
layout sketches, and instructions for the opt-in preview.

| Cycle | Deliverable | Acceptance check |
| --- | --- | --- |
| 01 | Capture a Bronze baseline for View, Terrain, Control, scenario switching, and save and restore. Record a representative laptop layout and current rendering cadence. | Capture a reproducible session with the scenario, selected actor, window sizes, and expected behavior noted. |
| 02 | Inventory commands, inspector fields, keyboard shortcuts, and file actions. Map each to the new workspace or a Bronze menu. | Every existing user-facing action has an assigned location, including pause, selection, reset, import, and diagnostics. |
| 03 | Sketch wide and compact workspace layouts from the reference stills. Define which panel has priority and when the inspector can collapse. | Document one usable layout for each width and an explicit way to reveal every panel. |
| 04 | Add a Bronze-only workspace entry point behind a development opt-in. Route both layouts to the same existing session. | Open and close the workspace from a Bronze Debug build without creating a second world or clock. |
| 05 | Define read-only UI snapshots for scenario, selection, and simulation status. Copy the values the UI needs while the source data is valid. | Switching scenarios and deleting or losing a selected actor clears or refreshes stale fields safely. |

## Live workspace

See [live workspace notes](LIVE-WORKSPACE.md) for the implementation of cycles 05–08 and the interactive check.

| Cycle | Deliverable | Acceptance check |
| --- | --- | --- |
| 06 | Host the live overhead map in the workspace using the existing map renderer and overlays. | Compare the same paused world in both layouts. Site markers align and map clicks select the expected actor. |
| 07 | Host the live perspective terrain view beside the map using the existing terrain renderer. | Pan and resize while paused and running. The scene and overlays stay registered. |
| 08 | Connect panel attachment, focus, and input routing. Route gestures and keys to the active surface. | Selection and navigation work after changing focus, hiding a panel, and reopening a window. |
| 09 | Synchronize selection across the map, terrain, and inspector using stable Bronze actor identity. | Select one actor from each supported surface and see the same identity in the other panels. |
| 10 | Add resizable panels, a compact layout, and an inspector toggle. | Narrow and widen the window without clipping essential controls or shifting hit targets. |
| 11 | Place the existing run controls and actor navigation in a Bronze workspace toolbar. | Keyboard and pointer actions invoke each command once. Opening additional views does not accelerate time. |

## Inspector and world commands

| Cycle | Deliverable | Acceptance check |
| --- | --- | --- |
| 12 | Replace the flat actor summary with sections for identity, vocation, workplace, and current task. | Compare the displayed values with the existing inspector for the same paused actor. |
| 13 | Add cargo, needs, remaining work, and last outcome using existing data. Keep unavailable values explicit. | Inspect actors in different states and verify that empty fields never display data from the previous selection. |
| 14 | Bring existing native Control information into a dedicated panel or expose the existing Control window from the workspace. | Energy, fatigue, and available memory details refer to the same being as the Bronze inspector. |
| 15 | Provide a legible map legend and visible selected-site context where the engine exposes it. | Distinguish township, copper, tin, field, and woodland symbols, including at compact widths. |
| 16 | Surface active scenario, scenario switching, and diagnostics. Make the world reset consequence clear before a switch. | Load all bundled scenarios. A rejected load keeps the current world and presents its diagnostic. |
| 17 | Connect Save Bronze World and Open Bronze World using the existing file flow. | Save and restore a paused world with active work. Scenario, selected actor, and task state agree after reopening. |
| 18 | Connect Import Native World and refine file error and cancellation states. | Import a supported native save, cancel a picker, and try a mismatched Bronze save. Each produces the expected world and UI state. |

## Preview qualification

| Cycle | Deliverable | Acceptance check |
| --- | --- | --- |
| 19 | Set typography, panel spacing, focus styles, and selection highlights using the stills as visual direction. | Labels, state, and controls remain legible over live map and terrain at laptop and desktop sizes. |
| 20 | Verify keyboard operation, accessibility labels, and reading order for the workspace. | Reach core commands and inspector sections without a pointer, and verify their names and state with VoiceOver. |
| 21 | Persist Bronze workspace panel sizes and visibility separately from world data. Keep the separate window layout available. | Relaunch into the chosen layout. Reset layout settings independently of the current world. |
| 22 | Profile renderer and inspector updates with both views visible, then with a panel hidden. | Compare against cycle 01. Confirm one simulation clock, bounded redraw work, and responsive selection and resizing. |
| 23 | Exercise lifecycle and restore cases across workspace and separate windows. | Cover resize, focus changes, closing and reopening panels, failed opens, and restoring a saved selection. |
| 24 | Qualify Bronze Debug and Release builds, bundled scenarios, and compatibility with the historical targets. | Run the relevant local checks, including `make -C bronze contract-test` and the bundle verifier. Build the historical target affected by any shared shell change. Use the existing CI jobs for hosted checks. |

Cycles 21–24 preview results on arm64 Mac Catalyst:

- **21** — Layout preferences store wide and compact splits and the Terrain
  and Inspector visibility flags separately from world files. Terrain stayed
  hidden after closing the workspace and after quitting and relaunching.
  Reset Workspace Layout restores both splits and both panels. The UI session
  timed out during reset verification, so reset and split relaunch still
  need a direct interactive check.
- **22** — With Bronze Age Full running and the separate windows still open,
  two three-second process samples found the drawing stack in 104 of 486
  main-thread samples with both workspace surfaces visible and 69 of 497
  with workspace Terrain hidden. These are stack sample counts. Cycle 01
  recorded a requested cadence without a measured baseline for CPU or frames.
  The Bronze Debug workspace uses one Control clock, requests drawing at
  about 30 Hz, and polls the inspector snapshot at up to 10 Hz during normal
  advancement. Direct input and file operations refresh the snapshot on demand.
  The native clock and render-rate checks passed.
- **23** — Opened the preview alongside the separate windows, hid Terrain
  while the world continued to run, and closed and reopened the workspace.
  Command-S and Command-O round-tripped a Bronze Age Full save. The reopened
  inspector showed actor 1 and the workspace resumed in its paused state.
  A malformed file produced the Bronze World File alert. The actor harness
  passed its selection and resume checks. Interactive divider resize and
  focus routing between overlapping windows still need a repeatable pass.
- **24** — The Bronze contract suite, Bronze Catalyst Debug and Release
  builds and packaged scenario verifiers, and historical Catalyst Debug
  Swift tests all passed locally.

The workspace opens on a fresh Bronze Mac Catalyst launch. Reopen it with
**Window → Open Bronze Workspace** (⌘⇧U).

## Mockup implementation and rollout

| Cycle | Deliverable | Acceptance check |
| --- | --- | --- |
| 25 | Match the mockup's workspace composition and visual hierarchy. Build the slim title toolbar, dominant left map, stacked Terrain and Inspector on the right, in-map legend, and map footer. Match the charcoal surfaces, bronze accents, pin and actor glyphs, and inspector rows and meters. | Compare captures at the mockup's aspect ratio and at laptop width. Confirm the title, controls, panel proportions, legend, selection, and inspector read in the same order. Compact layout keeps every core action reachable. |
| 26 | Implement the mockup's live overhead cartography. Shade the simulation heightfield and water, depict township structures, field rows, woodlands, mine clearings and paths where supported by world data, and render pins and selection rings over the map. Derive decorative geometry reproducibly from the scenario and site coordinates. | Capture Bronze Age Full and a contrasting bundled scenario. Check landforms, water, and site silhouettes against the reference's level of detail. Pan and zoom, then select actors and sites and verify that the visuals and hit targets stay aligned. |
| 27 | Implement the Bronze perspective renderer for the mockup's naturalistic landscape. Render textured ground and water, vegetation, and Bronze Age structures with depth, shadows, and consistent lighting. Place actors and highlight rings in the same world coordinates as the overhead map. Use Bronze-owned rendering and assets and profile as detail increases. | Capture a township view with visible depth, vegetation, structures, and the selected actor. Switch scenarios and verify the landscape follows the live world. Pan, resize, and watch an actor move while the corresponding position tracks in both views. |
| 28 | Connect the mockup's remaining controls to live behavior. Add paused Step, playback speed and simulation date, map scale and zoom, and recenter on selection. Retain scenario selection, Open and Save, and access to diagnostics and inspector details. | Step advances exactly one simulation tick while paused. Speed changes affect the one world clock, the displayed date follows it, and the scale reflects the map zoom. Open and Save restore the scenario, selected actor, and work state from the new workspace. |
| 29 | Qualify the complete visual target and interaction. Capture the live workspace beside the reference at comparable sizes. Review map and terrain detail, panel composition, and the visible states for a selected actor. Check keyboard access, VoiceOver, saves, and redraw performance with both renderers active. | Record captures and any remaining visual differences. Resolve gaps in the reference's major visual and interaction features before rollout. Verify responsive pause, selection, and resizing with one clock, and rerun Bronze Debug and Release and historical ApeSDK regression checks. |
| 30 | Make the qualified workspace the Bronze default. Document its controls and how to open the separate windows. | A fresh Bronze launch opens the mockup-aligned workspace. Load a scenario, select and follow an actor, and save and reopen the world through the default interface. |

Cycles 25–28 were implemented in the Debug workspace preview.
The workspace adds the reference's map footer and overlaid legend, the
Terrain and Inspector stack, and the energy meter. Both renderers read the
live native heightfield and site registry. The terrain is procedural, with
roundhouse clusters, water, shading, and depth haze. Terrain trees use
depth-tested trunks and branches with clusters of foliage. Its visual
detail and composition still need side-by-side review against the mockup.

The overhead map starts at half-world width and zooms out to one full world
width. Drag, arrow keys, or a mouse wheel or trackpad pan the map. Clicking
an actor updates the shared selection, and clicking a site highlights its
pin. The scale uses native map cells. The footer has playback speed, date,
zoom, and recenter, and Step runs only while paused.

The actor harness verifies opaque frames from both renderers, repeatable
map output at a fixed simulation time, and exactly one native tick per
paused Step. Debug build and bundled scenario checks passed. During
interactive checks the Terrain panel was initially inverted, and a later
build showed black panels. The preview now presents opaque RGBA frames
through UIKit with an explicit vertical orientation. The Bronze Age Full
preview showed both live panels and an upright Terrain view after that fix.
Scroll gesture verification timed out. The updated branch shapes, compact
layout, and contrasting scenario still need visual checks.

The cycles 17–20 build and packaged scenario checks passed, as did the actor
tests and a historical Catalyst Debug build. Workspace Save and Open are covered above. Native import through the
workspace picker and VoiceOver reading order still need interactive checks.

Cycle 29 qualification progress:

- The workspace is available in Bronze Catalyst Debug and Release. Its
  compact toolbar groups file operations and gives Pause and Step accessible
  names. **Open Separate Simulation Windows** opens View, Terrain, and Control.
- Terrain lighting now follows the interpolated heightfield and softens the
  transition from shore to grass. The actor and selection ring share the
  trees' depth buffer so closer terrain and vegetation can occlude them.
- Bronze Debug and Release builds and packaged scenarios, the Bronze
  contract and actor checks, and historical Catalyst Swift tests passed.
  A two-second Release process sample showed native simulation and renderer
  activity. UI inspection timed out with the Release preview open, so the
  sample does not establish responsiveness.
- A fresh arm64 Release launch showed the live Bronze Age Full workspace.
  The wide capture matches the reference's map, Terrain, and Inspector
  arrangement. The terrain still has visible vertical bands, stylized tree
  crowns, and a simplified actor silhouette. This capture establishes
  the layout and live rendering, with terrain fidelity still open.
- Command-P paused the world and enabled Step. Command-S saved the paused
  world with actor 1 selected. After changing selection to actor 2, Command-O
  restored actor 1, Bronze Age Full, and the paused state. Compact interaction,
  VoiceOver, and a contrasting scenario capture remain open.

Cycle 30 launch and fallback passed in a fresh Release run. The workspace is
the primary scene in Bronze Debug and Release. **Window → Open Separate
Simulation Windows** opened View, Terrain, and Control alongside it. Both
Bronze builds and their packaged scenario checks passed after the launch
change, as did the historical Catalyst Swift tests.

The renderer follow-up interpolates terrain color and depth per visible row,
softens repeating water patterns, and adds more distinct shoreline, field,
woodland, and mine materials. Selecting a site on the map aims Terrain at
that site. Recenter returns both views to the selected actor. The Release
capture still shows simplified geometry, especially tree silhouettes, ground
detail, and the actor. Keep the reference's landscape fidelity open under
cycle 29.

## Definition of done

The default Bronze workspace matches the mockup's composition, controls,
map detail, and naturalistic terrain. Both views render one live world and
show the same selected actor as the inspector. Selection, commands, and file operations work from
the new layout at both laptop and desktop widths. The separate Bronze windows
remain available, existing saves still open, and shared-shell changes pass
the relevant historical target checks with `BRONZE_SIM` undefined.

## Next settlement development phase

Develop spatial townships, professional premises and agent-usable footpaths
before the Victorian extension. See the [township implementation plan](../TOWNSHIPS-PLAN.md).
This is additional work; existing decorative houses and tracks do not satisfy
its simulation or visual acceptance criteria. All integration remains Bronze-only.
