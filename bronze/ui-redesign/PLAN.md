# BronzeSim workspace redesign

Deliver a BronzeSim workspace for the `bronze-mac` Mac Catalyst target. Use
[shot 27](reference/shot-27.png) as the main reference for the overhead map,
perspective terrain, and inspector on one display. Use
[shot 26](reference/shot-26.png) for the relationship between file operations
and the world. [Shot 04](reference/shot-04.png) and
[shot 29](reference/shot-29.png) inform composition and legibility at desktop
and laptop sizes. The stills are design references. The shipping views must
show live BronzeSim state.

This plan contains 25 proposed cycles. Complete each cycle's acceptance check
before starting work that depends on it. Record results against the cycle,
including the scenario, platform, and any deferred cases.

## Bronze-only scope

- Compile the new workspace and its UI support only into `bronze-mac`.
  Keep production Swift under [bronze/ui](../ui), with this directory for
  design references and planning.
- In the [shared app shell](../../ios/ApeSimApp.swift), keep integration
  points inside `#if BRONZE_SIM`. If a shared C hook is necessary, keep it
  small and inside `#ifdef BRONZE_SIM`. Historical ApeSDK targets are
  regression checks only.
- Reuse the current session, render surfaces, input routing, scenario
  controller, and file operations. Each workspace view observes one world.
  The existing engine owns the clock, positions, and simulation state.
- Preserve scenario behavior, selection identity, save compatibility, and
  the existing BronzeSim menus and separate window layout during rollout.
  See the [Bronze implementation plan](../PLAN.md) and
  [save format](../SAVE-FORMAT.md).

## Foundation

| Cycle | Deliverable | Acceptance check |
| --- | --- | --- |
| 01 | Capture a Bronze baseline for View, Terrain, Control, scenario switching, and save and restore. Record a representative laptop layout and current rendering cadence. | Capture a reproducible session with the scenario, selected actor, window sizes, and expected behavior noted. |
| 02 | Inventory commands, inspector fields, keyboard shortcuts, and file actions. Map each to the new workspace or a Bronze menu. | Every existing user-facing action has an assigned location, including pause, selection, reset, import, and diagnostics. |
| 03 | Sketch wide and compact workspace layouts from the reference stills. Define which panel has priority and when the inspector can collapse. | Document one usable layout for each width and an explicit way to reveal every panel. |
| 04 | Add a Bronze-only workspace entry point behind a development opt-in. Route both layouts to the same existing session. | Open and close the workspace from a Bronze Debug build without creating a second world or clock. |
| 05 | Define read-only UI snapshots for scenario, selection, and simulation status. Copy the values the UI needs while the source data is valid. | Switching scenarios and deleting or losing a selected actor clears or refreshes stale fields safely. |

## Live workspace

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

## Qualification and rollout

| Cycle | Deliverable | Acceptance check |
| --- | --- | --- |
| 19 | Set typography, panel spacing, focus styles, and selection highlights using the stills as visual direction. | Labels, state, and controls remain legible over live map and terrain at laptop and desktop sizes. |
| 20 | Verify keyboard operation, accessibility labels, and reading order for the workspace. | Reach core commands and inspector sections without a pointer, and verify their names and state with VoiceOver. |
| 21 | Persist Bronze workspace panel sizes and visibility separately from world data. Keep the separate window layout available. | Relaunch into the chosen layout. Reset layout settings independently of the current world. |
| 22 | Profile renderer and inspector updates with both views visible, then with a panel hidden. | Compare against cycle 01. Confirm one simulation clock, bounded redraw work, and responsive selection and resizing. |
| 23 | Exercise lifecycle and restore cases across workspace and separate windows. | Cover resize, focus changes, closing and reopening panels, failed opens, and restoring a saved selection. |
| 24 | Qualify Bronze Debug and Release builds, bundled scenarios, and compatibility with the historical targets. | Run the relevant local checks, including `make -C bronze contract-test` and the bundle verifier. Build the historical target affected by any shared shell change. Use the existing CI jobs for hosted checks. |
| 25 | Make the workspace the Bronze default after qualification. Document the layout, core actions, and how to switch to separate windows. | A fresh Bronze launch opens a usable workspace, and a user can load a scenario, follow an actor, and save and reopen the world from it. |

## Definition of done

The Bronze workspace shows a live overhead map, perspective terrain, and
inspector for one world. Selection, commands, and file operations work from
the new layout at both laptop and desktop widths. The separate Bronze windows
remain available, existing saves still open, and shared-shell changes pass
the relevant historical target checks with `BRONZE_SIM` undefined.
