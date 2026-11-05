# Live workspace · cycles 05–08

Open **Window → Open Bronze Workspace Preview** or press **⇧⌘U** in
`bronze-mac` Debug. The workspace hosts the native overhead and perspective
renderers from the same session as View, Terrain, and Control.

- **05:** The inspector reads a value snapshot with scenario name, selected
  actor identity, copied actor summary, and pause state. Refresh after engine
  cycles and on reset, import, restore, and menu commands. The C summary
  supplies an explicit empty selection when the selected actor is lost.
- **06:** The overhead panel uses the original map and overlays. Input uses
  the panel's rendered rectangle, including letterboxing.
- **07:** Terrain uses the original perspective renderer. Pointer input
  is consumed just after drawing the originating panel so its projection
  dimensions are current.
- **08:** Clicking a surface focuses it for arrow navigation. Tab moves
  between focusable controls and panels. The pointer is owned by the
  originating surface and released on end, cancellation, or detach.
  While workspace surfaces are attached, the existing timer advances the
  control cycle once per callback. Rendering either workspace surface or
  an additional Control window does not advance it again.

To check interactively, pause a world in the original layout. Open the preview,
compare the overlays, and click a being inside the map and then in its letterbox.
Focus Terrain and exercise pointer and arrow navigation. Resize across the
layout breakpoint, hide and show the inspector, and close and reopen the
preview. Compare selection after resetting or switching to another scenario.
Repeat with the simulation running and with the separate Control window closed.

## Verification

- Built Bronze Mac Catalyst Debug and passed the bundled resource and
  scenario controller checks.
- Built the historical Mac Catalyst target with `BRONZE_SIM` undefined.
  The deterministic contract passed in native, native with thunder hidden,
  and Bronze modes.
- Opened the workspace in the running Debug app and observed both live
  surfaces and Bronze overlays. Paused from the existing command, changed
  selection from actor #1 to actor #2 with Next Ape, hid the inspector, and
  closed and reopened the preview. The reopened inspector showed the same
  paused actor #2 and task values.
- The remaining interactive checks are map hit testing at the letterbox
  boundary, terrain pointer and arrow navigation, and overlay alignment
  while resizing through the layout breakpoint.

## Cycles 09–12

- **09:** The map, terrain, and inspector show the same stable actor ID.
  Map selection and actor navigation use the native session. Terrain
  pointer input retains its existing camera behavior.
- **10:** Drag the divider between the map and terrain column to change
  their share of the available space. At compact widths the panels stack
  and the divider moves vertically. The header retains the Inspector toggle.
- **11:** Pause or Resume, Previous Ape, and Next Ape invoke the same
  session commands as the existing Control menu. Menu shortcuts remain
  available while the workspace has focus.
- **12:** The inspector reads copied identity, vocation, workplace, and task
  fields from the selected actor. The classic inspector continues to use its
  existing summary. Both displays use the same task status rules.

Bronze Debug and bundled scenario checks, historical Catalyst Debug, the
deterministic compatibility contract, and the actor and work harnesses
passed. The actor harness checked that a cleared selection returns empty
inspector data. In the live Debug app, Pause changed to Resume and one Next
Ape click advanced actor #1 to #2 in both panels and the inspector. The
inspector displayed the farmer vocation, workplace #7, and the completed
recover task. Interactive divider dragging and resizing across the compact
breakpoint remain to be checked.

## Cycles 13–16

- **13:** The inspector shows cargo and items, native energy, hunger and
  fatigue, remaining work, and the last trade or service outcome. Empty
  inventory and absent outcomes have explicit labels.
- **14:** Open Control Window brings up the existing native Control surface
  for the same session and selected being, including its memory view.
- **15:** The overhead map has a legend for all five site symbols. It moves
  into a menu when space is limited. Actor site shows the workplace site
  or, for an actor without a workplace, the home township when available.
- **16:** The workspace scenario menu shows the active scenario and asks
  before resetting the world. It uses the existing transactional loader.
  A rejected load shows diagnostics in the workspace without committing
  a reset.

The Debug app showed the expanded inspector, all five legend entries,
the scenario menu, and the Control button. The actor harness checked native
energy, empty inventory, township context, and the exchange partner clearing.
The packaged controller tests exercised all three scenarios and a rejected
load with workspace-owned diagnostics. Interactive verification of the
scenario confirmation, Control activation, and compact legend remains open.
