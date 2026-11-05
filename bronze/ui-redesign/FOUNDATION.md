# Bronze workspace foundation

## Cycle 01 · Baseline

The [existing interactive qualification](../contract/RESULTS-CYCLE18.md#interactive-catalyst-checks)
records all three surfaces in the first slice, selection recentering, a switch
to the vertical slice, and combined save and restore. That capture restored
actor #1 with energy 3783 and fatigue 13 from its particular saved checkpoint.
Use the values from your own checkpoint when reproducing the comparison.

The existing Catalyst window defaults in points are View 512 × 512, Terrain
600 × 500, and Control 400 × 600. For a repeatable laptop comparison, use a
1440 × 900 logical desktop, arrange View and Terrain side by side at their
defaults, and leave Control open in front when inspecting it. Record actual
window dimensions if macOS restores previous sizes. The session model requests
redraws every 1/60 second for attached render views. That is the configured
request cadence, not a measured frame rate or simulation tick rate.

1. Launch the unsigned `bronze-mac` Debug build. Load First Slice from the
   Bronze Scenario menu and pause using Control → Pause.
2. Select an actor and record the scenario, actor ID, inspector fields, map
   markers, and native Control details. Move or resize Terrain and compare
   the markers with the projected terrain.
3. Save the Bronze world. Select a different actor, then reopen the save.
   Check the restored identity, energy, fatigue, and task while paused.
4. Switch to Vertical Slice and confirm the active scenario changes. Switch back to First Slice before opening
   the previous save to repeat the comparison.

## Cycle 02 · Command and field inventory

| Existing Bronze entry | Intended location |
| --- | --- |
| New Simulation (⌘N) | File menu |
| Save Bronze World (⌘S), Open Bronze World (⌘O) | Workspace toolbar and File menu |
| Import Native World | File menu |
| Load First Slice, Vertical Slice, or Bronze Age Full and reset | Workspace scenario control and Bronze Scenario menu |
| Active scenario, scenario details and diagnostics | Workspace status and Bronze Scenario menu |
| Pause or Resume (⌘P), Previous Ape (⌘[), Next Ape (⌘]) | Workspace toolbar and Control menu |
| Clear Errors (⌘E), Territory, Weather, Brain, Braincode | Control menu |
| Map click, terrain interaction, selection and camera navigation | Existing surface handlers hosted in the workspace |
| Actor identity, vocation, skills, home, workplace, task and progress, cargo, needs, mentoring, last exchange | Workspace inspector |
| Native being and memory details | Native Control surface or dedicated workspace panel |
| Township, copper, tin, field, woodland legend | Overhead map |
| Separate View, Terrain, and Control windows | Existing window layout |

The concept still includes a Step control. Treat it as design exploration until
the engine can expose a supported single-step operation.

## Cycle 03 · Layout sketch

Use [the workspace concept](reference/bronzesim-workspace-concept.png) as the
wide composition. Below 960 points in the preview's content area, stack the
panels in reading order. These are starting sizes for the later live layout.

| Width | Panel arrangement | Panel access |
| --- | --- | --- |
| 960 points or more | Map on the left, terrain above inspector on the right | Inspector toggle in the header |
| Below 960 points | Map, terrain, then inspector | Inspector toggle stays in the header, both views stay visible |

The map has priority when setting final split sizes. Keep panel titles visible
and keep the inspector reachable when the window is narrow.

## Cycle 04 · Opt-in preview

In the Bronze Catalyst Debug app choose **Window → Open Bronze Workspace Preview**
or press **⇧⌘U**.
Select another actor in the existing View window and compare its summary with
the preview. Resize across the width threshold, toggle Inspector, close the
preview, and open it again. The preview receives the original scene model and
renders a live inspector with map and terrain placeholders. It does not attach
a renderer, start a timer, or create a simulation session. Live surface hosting
starts with the map in cycle 06.


## Verification on 2026-11-04

- Bronze Mac Catalyst Debug built without a signing override. Packaged
  resource and scenario controller tests passed.
- Historical Mac Catalyst Debug built with `BRONZE_SIM` undefined.
- The running Bronze Debug app listed the preview command and the existing
  View, Terrain, and Control windows. The UI automation could not reliably
  activate the menu item. The open, resize, and close smoke check remains
  pending. The baseline above draws on prior interactive qualification and
  current layout and timer settings.
