# ApeSDK Mac Catalyst Plan

Last updated: 2026-07-06

## Goal

Create a `maccatalyst` home for the next Apple-platform simulation app:

- A full Mac build with the current `sim-mac` functionality, including multiple windows, menu commands, file open/save, input handling, and initial tutorial behavior.
- An iPadOS and iOS build that shows and drives the simulation.
- Shared simulation, rendering, and bridge C code wherever possible.
- Platform-specific Swift only where macOS, Mac Catalyst, iPadOS, or iOS genuinely need different windowing or input behavior.

## Source of Truth Rules

- Keep this `PLAN.md` current before or alongside meaningful structural changes.
- Leave `toolchains/sim-mac` in place until the `maccatalyst` Mac build reaches functional parity with it.
- Avoid duplicated source. Prefer source links, Xcode file references, shared build phases, or small platform-specific Swift wrappers.
- Do not physically relocate canonical Mac or iOS source unless the new project builds and the reason is recorded here.
- Stop and ask for feedback if a choice would permanently fork source ownership, remove an existing build path, or change user-facing behavior without a clear compatibility path.

## Current Status

- Done: Created `maccatalyst/`.
- Done: Added source links under `maccatalyst/source-links/`:
  - `sim-mac` -> `../../toolchains/sim-mac`
  - `ios` -> `../../ios`
- Done: Confirmed the existing `sim-mac` scheme exists.
- Done: Built the existing `sim-mac` Debug app successfully.
- Done: Launched the built app and confirmed `System Events` reports the `Simulated Ape` process is running.
- Done: Created `maccatalyst/maccatalyst.xcodeproj` from the existing Mac project metadata.
- Done: Repointed the `maccatalyst` Mac project to existing source instead of copying source:
  - Swift app shell, assets, entitlements, and bridging header via `maccatalyst/source-links/sim-mac`.
  - Shared C sources via paths from `maccatalyst/` back to the repository root.
- Done: Built the `maccatalyst` Mac project successfully in Debug.
- Not done: The first `maccatalyst` target and scheme still use the inherited `sim-mac` name.
- Not done: No iOS/iPadOS build has been validated from the new directory yet.
- Not done: No Mac Catalyst-specific target has been added yet.

## Baseline Evidence

Existing Mac baseline command:

```sh
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-sim-mac-derived build
```

Result on 2026-07-06: `BUILD SUCCEEDED`.

Built product:

```text
/private/tmp/apesdk-sim-mac-derived/Build/Products/Debug/Simulated Ape.app
```

Run check on 2026-07-06:

```sh
open /private/tmp/apesdk-sim-mac-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e 'tell application "System Events" to exists process "Simulated Ape"'
```

Result: `true`.

New `maccatalyst` Mac baseline command:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-mac-derived build
```

Result on 2026-07-06: `BUILD SUCCEEDED`.

Built product:

```text
/private/tmp/apesdk-maccatalyst-mac-derived/Build/Products/Debug/Simulated Ape.app
```

## Canonical Source Map

- Current Mac app baseline:
  - `toolchains/sim-mac/sim-mac.xcodeproj`
  - `toolchains/sim-mac/sim-mac/SimMacApp.swift`
  - `toolchains/sim-mac/sim-mac/AppDelegate.swift`
  - `toolchains/sim-mac/sim-mac/ViewWrapper.swift`
  - `toolchains/sim-mac/sim-mac/CustomDrawingView.swift`
  - `toolchains/sim-mac/sim-mac/InitialTutorialController.swift`
  - `toolchains/sim-mac/render/sim-mac-Bridging-Header.h`
- Current compact iOS app source:
  - `ios/ApeSimApp.swift`
- New `maccatalyst` project metadata:
  - `maccatalyst/maccatalyst.xcodeproj`
  - Currently references existing Mac source through `maccatalyst/source-links/sim-mac`.
  - Currently references existing iOS source through `maccatalyst/source-links/ios`.
- Shared simulation/rendering/bridge code already used by the Mac target:
  - `gui/shared.c`
  - `gui/draw.c`
  - `gui/gui.h`
  - `sim/`
  - `universe/`
  - `entity/`
  - `toolkit/`
  - `script/`
  - `render/`
  - `shared.h`
- Existing Catalyst-oriented reference material, not adopted yet:
  - `june13/SimulatedUniverse.xcodeproj`
  - `june13/SimulatedUniverse-200-Cycle-Plan.md`
  - `june13/Docs/`

## sim-mac Functional Parity Checklist

The new Mac build is not a replacement for `toolchains/sim-mac` until these are working:

- Main View window rendering `NUM_VIEW`.
- Terrain window rendering `NUM_TERRAIN`.
- Control window rendering `NUM_CONTROL`.
- Continuous simulation loop through `shared_cycle`.
- Drawing through `shared_draw`.
- New Simulation menu command.
- Open simulation file.
- Open script file.
- Save As.
- About Simulated Ape.
- Pause/Resume.
- Previous Ape and Next Ape.
- Clear Errors.
- Territory toggle.
- Weather toggle.
- Brain toggle.
- Braincode toggle.
- Command Line command.
- Online Manual and Simulation Page commands.
- Keyboard input for letters and arrows.
- Mouse down/up/drag, right mouse, and option/control-modified mouse.
- Scroll wheel, magnify, and rotate gestures where supported.
- Initial tutorial popovers or a platform-appropriate equivalent.
- App sandbox/file entitlements appropriate to the target.
- Local debug build and launch from Xcode command line.

## iOS/iPadOS Minimum Checklist

The mobile build should at least provide:

- A visible simulation view.
- Initialization through `shared_init`.
- Drawing through `shared_draw_ios` or a documented shared replacement.
- Cycle updates through `shared_cycle_ios` or a documented shared replacement.
- Touch input routed to the shared mouse/input API.
- New Simulation, Next Ape, and Previous Ape commands.
- Idle timer behavior appropriate for a running simulation.
- iPad layout that can grow beyond the single compact iPhone overlay.

## Architecture Direction

1. Keep the C simulation and rendering code shared.
2. Treat Swift as a platform shell:
   - Native macOS can use AppKit/SwiftUI where needed for full window and menu parity.
   - iOS/iPadOS can use UIKit/SwiftUI wrappers.
   - Mac Catalyst can share the UIKit/SwiftUI path where it is sufficient.
3. Prefer one Apple-platform Xcode project or workspace under `maccatalyst/` that references canonical source files instead of copying them.
4. Introduce new Swift files under `maccatalyst/` only when they are truly shared wrappers or intentional platform forks.
5. Only retire or move `toolchains/sim-mac` after the parity checklist is complete and the user approves the transition.

## Work Phases

### Phase 0: Baseline and Source Map

Status: Done.

- Done: Build and run-check `toolchains/sim-mac`.
- Done: Create `maccatalyst/`.
- Done: Add source links to the current Mac and iOS source trees.
- Done: Record current source ownership and parity requirements in this plan.
- Done: Inspect `sim-mac.xcodeproj` source membership closely enough to reproduce it in the new project without copying source.

### Phase 1: New Project Skeleton

Status: In progress.

- Done: Create a `maccatalyst` Xcode project.
- Done: Add a Mac build target by repointing the existing Mac project metadata.
- Done: Reference existing shared C sources and headers.
- Done: Reference existing Mac assets where possible.
- Done: Build the Mac target from `maccatalyst/`.
- Next: Add an iOS/iPadOS target that references `maccatalyst/source-links/ios`.
- Next: Decide whether to rename the inherited `sim-mac` target/scheme before adding more targets.

### Phase 2: Mac Parity

Status: Not started.

- Recreate the three-window Mac experience.
- Recreate menu commands and keyboard shortcuts.
- Recreate file open/save behavior.
- Recreate initial tutorial behavior or document a platform-appropriate replacement.
- Validate against the parity checklist.

### Phase 3: iOS/iPadOS Usability

Status: Not started.

- Validate a mobile build from the new project.
- Keep the current compact iOS overlay working.
- Improve iPad layout for larger screens and optional multi-window scene behavior.
- Confirm touch, simulation cycling, and redraw behavior.

### Phase 4: Transition

Status: Not started.

- Compare the new Mac build against `toolchains/sim-mac`.
- Keep both builds until parity is proven.
- Ask the user before removing, archiving, or physically moving old source.

## Immediate Next Steps

1. Add a buildable iOS/iPadOS target to `maccatalyst/maccatalyst.xcodeproj` that references `ios/ApeSimApp.swift` through `source-links/ios`.
2. Validate that target on an iOS Simulator destination with signing disabled.
3. Decide whether the inherited Mac target/scheme should be renamed now or after the mobile target exists.

## Open Questions

- None blocking right now.
