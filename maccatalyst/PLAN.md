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
- During this migration, test with explicit derived build products under `/private/tmp` or another recorded build output. Do not use `/Applications/Simulated Ape.app` for process validation until the app is intentionally production/release compilable.
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
- Done: Added an `ApeSim-iOS` target and scheme to `maccatalyst/maccatalyst.xcodeproj`.
- Done: `ApeSim-iOS` references `ios/ApeSimApp.swift` through `maccatalyst/source-links/ios`.
- Done: `ApeSim-iOS` builds for a generic iOS Simulator destination with signing disabled.
- Done: Added shared iOS draw/cycle wrappers in `gui/shared.c`.
- Done: Updated `ios/ApeSimApp.swift` to initialize the processing window (`NUM_CONTROL`) before drawing terrain.
- Done: Smoke-tested the `maccatalyst` Mac app launch. `System Events` observed `Control`, `Terrain`, and `View` windows plus the expected top-level menus.
- Done: Installed and launched the `ApeSim-iOS` product on an iPad Air 13-inch (M4) Simulator running iOS 26.5.
- Done: Verified the iPad Simulator renders visible terrain from the shared simulation.
- Done: Added an adaptive mobile command panel: compact width remains bottom-trailing, while regular iPad width uses a larger trailing panel.
- Done: Fixed mobile touch coordinate mapping so touches are scaled to the actual render buffer rather than blindly using `UIScreen.scale`.
- Done with note: Verified Simulator UI touch delivery through Computer Use. `simctl io tap` remains unavailable, but a terrain drag produced UIKit touch dispatch events in the rebuilt `ApeSim-iOS` process.
- Done: Exercised the iPad overlay commands (`Next Ape`, `Previous Ape`, `New Simulation`) through the Simulator UI. `New Simulation` visibly regenerated the terrain.
- Done: Triage of manual-shutdown crash report supplied on 2026-07-06. The crash was from `/Applications/Simulated Ape.app` with bundle id `com.apesdk.Simulated-Ape` and a MetalKit `ApeMTKRenderer` stack, not the current `/private/tmp/.../Simulated Ape.app` `maccatalyst` build with bundle id `com.apesdk.sim-mac`.
- Done: Added shutdown hardening for shared close/draw paths:
  - `shared_close()` is now idempotent.
  - The source-linked `toolchains/sim-mac` AppKit/SwiftUI app sets a termination flag, calls `shared_close()`, and stops queued redraw from reinitializing or drawing during termination.
  - The older shared `mac/` wrapper now makes close idempotent, stops redraw scheduling while closing, and calls close from `applicationWillTerminate`.
- Done: Rebuilt `maccatalyst` Mac, original `toolchains/sim-mac`, and `toolchains/planet` after shutdown hardening.
- Done: Verified the rebuilt `maccatalyst` Mac app exits through the user-facing Quit menu item; System Events reported no remaining `com.apesdk.sim-mac` process afterward.
- Decision: The user replaced `/Applications/Simulated Ape.app` with the mentioned build, but future process testing should avoid the `/Applications` copy until the app is 100% production/release compilable.
- Decision: Keep the inherited `sim-mac` target and scheme name for now to preserve traceability to the known-good Mac baseline. Revisit renaming after the mobile target is more usable.
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

New `maccatalyst` iOS/iPadOS baseline command:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath /private/tmp/apesdk-maccatalyst-ios-derived CODE_SIGNING_ALLOWED=NO build
```

Result on 2026-07-06: `BUILD SUCCEEDED`.

Built product:

```text
/private/tmp/apesdk-maccatalyst-ios-derived/Build/Products/Debug-iphonesimulator/ApeSim-iOS.app
```

Post-change Mac verification:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-mac-derived build
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-sim-mac-derived build
```

Result on 2026-07-06: both commands produced `BUILD SUCCEEDED`.

New `maccatalyst` Mac app smoke test on 2026-07-06:

```sh
open /private/tmp/apesdk-maccatalyst-mac-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e 'tell application "System Events" to tell process "Simulated Ape" to get name of windows'
osascript -e 'tell application "System Events" to tell process "Simulated Ape" to tell menu bar 1 to get name of menu bar items'
```

Observed windows: `Control`, `Terrain`, `View`.

Observed top-level menus: `Apple`, `Simulated Ape`, `File`, `Edit`, `View`, `Control`, `Online`, `Window`, `Help`.

Observed File menu commands included: `New Simulation`, `Close`, `Close All`, `Open...`, `Open Script...`, `Save As...`.

Observed Control menu commands included: `Pause`, `Previous Ape`, `Next Ape`, `Clear Errors`, `Territory`, `No Weather`, `No Brain`, `Braincode`, `Command Line`.

Observed Online menu commands: `Manual`, `Simulation Page`.

New `maccatalyst` iPad Simulator smoke test on 2026-07-06:

```sh
xcrun simctl boot 28E193E7-623B-4E28-8C37-A90E044D8CBE
xcrun simctl bootstatus 28E193E7-623B-4E28-8C37-A90E044D8CBE -b
xcrun simctl install 28E193E7-623B-4E28-8C37-A90E044D8CBE /private/tmp/apesdk-maccatalyst-ios-derived/Build/Products/Debug-iphonesimulator/ApeSim-iOS.app
xcrun simctl launch 28E193E7-623B-4E28-8C37-A90E044D8CBE com.apesdk.sim-ios
xcrun simctl io 28E193E7-623B-4E28-8C37-A90E044D8CBE screenshot /private/tmp/apesdk-ipad-smoke.png
```

Observed device: iPad Air 13-inch (M4), iOS 26.5.

Result: app launched and showed a full-screen terrain simulation with the compact command overlay.

iPad layout pass command:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath /private/tmp/apesdk-maccatalyst-ios-derived CODE_SIGNING_ALLOWED=NO build
```

Result on 2026-07-06: `BUILD SUCCEEDED`.

iPad layout smoke test on 2026-07-06:

```sh
xcrun simctl terminate 28E193E7-623B-4E28-8C37-A90E044D8CBE com.apesdk.sim-ios
xcrun simctl install 28E193E7-623B-4E28-8C37-A90E044D8CBE /private/tmp/apesdk-maccatalyst-ios-derived/Build/Products/Debug-iphonesimulator/ApeSim-iOS.app
xcrun simctl launch 28E193E7-623B-4E28-8C37-A90E044D8CBE com.apesdk.sim-ios
xcrun simctl io 28E193E7-623B-4E28-8C37-A90E044D8CBE screenshot /private/tmp/apesdk-ipad-layout-smoke.png
```

Result: app launched as `com.apesdk.sim-ios: 63959`. Screenshot showed visible terrain and the regular-width iPad command panel placed on the trailing edge with larger controls.

Touch automation note:

```sh
xcrun simctl io 28E193E7-623B-4E28-8C37-A90E044D8CBE tap ...
```

Result: unavailable with this `simctl io`; supported actions reported by the tool are screenshot/video/display-oriented. Touch routing was verified through the Simulator UI route recorded below instead.

Touch and overlay command pass on 2026-07-06:

```sh
xcrun simctl spawn 28E193E7-623B-4E28-8C37-A90E044D8CBE log stream --style compact --predicate 'process == "ApeSim-iOS" OR eventMessage CONTAINS "Touch moved"'
```

Computer Use actions:

- Dragged on the terrain canvas inside the iPad Simulator.
- Clicked the accessibility-exposed `Next Ape`, `Previous Ape`, and `New Simulation` buttons.

Observed log evidence before the coordinate fix: the running `ApeSim-iOS` process received UIKit touch dispatch events for the terrain drag.

Touch coordinate fix:

- `ios/ApeSimApp.swift` now computes touch scale from the current render buffer width (`oldDimensionX / bounds.width`) instead of `UIScreen.main.scale`.
- This keeps `shared_mouseReceived` coordinates aligned with the buffer produced by `shared_draw_ios`.

Post-fix build command:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath /private/tmp/apesdk-maccatalyst-ios-derived CODE_SIGNING_ALLOWED=NO build
```

Result on 2026-07-06: `BUILD SUCCEEDED`.

Post-fix iPad Simulator smoke test:

```sh
xcrun simctl terminate 28E193E7-623B-4E28-8C37-A90E044D8CBE com.apesdk.sim-ios
xcrun simctl install 28E193E7-623B-4E28-8C37-A90E044D8CBE /private/tmp/apesdk-maccatalyst-ios-derived/Build/Products/Debug-iphonesimulator/ApeSim-iOS.app
xcrun simctl launch 28E193E7-623B-4E28-8C37-A90E044D8CBE com.apesdk.sim-ios
xcrun simctl spawn 28E193E7-623B-4E28-8C37-A90E044D8CBE log stream --style compact --predicate 'process == "ApeSim-iOS" OR eventMessage CONTAINS "Touch moved"'
xcrun simctl io 28E193E7-623B-4E28-8C37-A90E044D8CBE screenshot /private/tmp/apesdk-ipad-touch-buttons-rebuild-smoke.png
```

Result: app launched as `com.apesdk.sim-ios: 85673`. A terrain drag produced UIKit `UIEvent` dispatch logs in `ApeSim-iOS`. `Next Ape`, `Previous Ape`, and `New Simulation` accepted Simulator UI clicks; `New Simulation` visibly changed the terrain. Final screenshot path: `/private/tmp/apesdk-ipad-touch-buttons-rebuild-smoke.png`.

Manual-shutdown crash report received on 2026-07-06:

```text
Process: Simulated Ape [93679]
Path: /Applications/Simulated Ape.app/Contents/MacOS/Simulated Ape
Identifier: com.apesdk.Simulated-Ape
Exception Type: EXC_CRASH (SIGABRT)
Application Specific Information: stack buffer overflow
Top app frames: draw_cycle -> shared_draw -> -[ApeMTKRenderer updateRenderedView:] -> -[ApeMTKRenderer drawInMTKView:]
```

Triage notes:

- The report points to the installed `/Applications` app and an older MetalKit renderer path.
- The current `maccatalyst` Mac target produces `/private/tmp/apesdk-maccatalyst-mac-derived/Build/Products/Debug/Simulated Ape.app` with bundle id `com.apesdk.sim-mac`, and it uses the source-linked AppKit/SwiftUI `CustomDrawingView` path.
- Even though the report is not from the current `maccatalyst` product, the same shared simulation/draw shutdown boundary is relevant to the transition.

Shutdown hardening added:

- `gui/shared.c`: `shared_close()` returns immediately if the simulation is already closed.
- `toolchains/sim-mac/sim-mac/AppDelegate.swift`: `applicationWillTerminate` sets `AppDelegate.isTerminating` and calls `shared_close()` when needed.
- `toolchains/sim-mac/sim-mac/CustomDrawingView.swift`: `draw(_:)` exits during termination and queued redraws stop once termination starts.
- `mac/AppDelegate.swift`: `applicationWillTerminate` closes and clears the shared controller.
- `mac/SimulationBindings.swift`: `SimulationShared.close()` is idempotent, draw/cycle/blit paths return while closing, and redraw scheduling stops while closing.

Post-shutdown-hardening build commands:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-mac-derived build
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-sim-mac-derived build
xcodebuild -project toolchains/planet/planet.xcodeproj -scheme planet -configuration Debug -derivedDataPath /private/tmp/apesdk-planet-derived build
```

Result on 2026-07-06: all three commands produced `BUILD SUCCEEDED`.

Post-shutdown-hardening quit smoke test:

```sh
open -n /private/tmp/apesdk-maccatalyst-mac-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac") to get {name, name of windows}'
osascript -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac")' \
  -e 'set frontmost to true' \
  -e 'click menu item "Quit Simulated Ape" of menu "Simulated Ape" of menu bar item "Simulated Ape" of menu bar 1' \
  -e 'end tell' \
  -e 'delay 3' \
  -e 'tell application "System Events" to exists (first application process whose bundle identifier is "com.apesdk.sim-mac")'
```

Result: the launched app reported windows `Control`, `Terrain`, `View`. The menu-based quit returned `false` for the remaining `com.apesdk.sim-mac` process check, so the rebuilt `maccatalyst` app exited cleanly through the manual Quit path.

## Canonical Source Map

- Current Mac app baseline:
  - `toolchains/sim-mac/sim-mac.xcodeproj`
  - `toolchains/sim-mac/sim-mac/SimMacApp.swift`
  - `toolchains/sim-mac/sim-mac/AppDelegate.swift`
  - `toolchains/sim-mac/sim-mac/AppDelegate.swift` now guards app termination and closes the shared simulation.
  - `toolchains/sim-mac/sim-mac/ViewWrapper.swift`
  - `toolchains/sim-mac/sim-mac/CustomDrawingView.swift`
  - `toolchains/sim-mac/sim-mac/CustomDrawingView.swift` now stops drawing/redraw scheduling after termination begins.
  - `toolchains/sim-mac/sim-mac/InitialTutorialController.swift`
  - `toolchains/sim-mac/render/sim-mac-Bridging-Header.h`
- Current compact iOS app source:
  - `ios/ApeSimApp.swift`
  - Initializes through `NUM_CONTROL` so the shared simulation starts before the terrain view draws.
  - Uses an adaptive SwiftUI overlay: compact width keeps the small bottom-trailing controls, regular iPad width uses a wider trailing command panel.
  - Maps touch locations to the render buffer scale before passing them to `shared_mouseReceived`.
- New `maccatalyst` project metadata:
  - `maccatalyst/maccatalyst.xcodeproj`
  - Currently references existing Mac source through `maccatalyst/source-links/sim-mac`.
  - Currently references existing iOS source through `maccatalyst/source-links/ios`.
  - Current schemes: `sim-mac`, `ApeSim-iOS`.
- Shared simulation/rendering/bridge code already used by the Mac target:
  - `gui/shared.c`
  - `gui/shared.c` now defines `shared_draw_ios` and `shared_cycle_ios` for the compact mobile shell.
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
- Older shared Mac wrapper affected by the supplied crash report:
  - `mac/AppDelegate.swift`
  - `mac/SimulationBindings.swift`
  - This wrapper is not the current `maccatalyst` Mac shell, but it is used by at least `toolchains/planet` and now has the same idempotent close/draw-stop hardening.

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
- Manual app shutdown without queued redraw entering shared drawing after close.

## iOS/iPadOS Minimum Checklist

The mobile build should at least provide:

- Done: A visible simulation view.
- Done: Initialization through `shared_init`.
- Done: Drawing through `shared_draw_ios` or a documented shared replacement.
- Done: Cycle updates through `shared_cycle_ios` or a documented shared replacement.
- Done with note: Touch input routed to the shared mouse/input API. Verification used Simulator UI drag plus UIKit event logs because this `simctl io` lacks synthetic taps.
- Done: New Simulation, Next Ape, and Previous Ape commands are present in the overlay.
- Done: Idle timer behavior appropriate for a running simulation.
- Done: iPad layout that can grow beyond the single compact iPhone overlay.

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

Status: Done.

- Done: Create a `maccatalyst` Xcode project.
- Done: Add a Mac build target by repointing the existing Mac project metadata.
- Done: Add an iOS/iPadOS build target.
- Done: Reference existing shared C sources and headers.
- Done: Reference existing Mac assets where possible.
- Done: Build the Mac target from `maccatalyst/`.
- Done: Build the iOS/iPadOS target from `maccatalyst/`.
- Done: Decide to keep the inherited `sim-mac` target/scheme name for now.

### Phase 2: Mac Parity

Status: In progress.

- In progress: The `maccatalyst` Mac target references the same Swift, assets, entitlements, and shared C sources as `toolchains/sim-mac`.
- Done: Build the `maccatalyst` Mac target after adding the iOS wrappers.
- Done: Launch and smoke-test the `maccatalyst` Mac app against the parity checklist before considering it a replacement.
- Done: Validate the three-window Mac experience exists at launch.
- Partial: Validate menu command presence. Command behavior still needs exercising.
- Done: Validate manual Quit for the rebuilt `maccatalyst` app after shutdown hardening.
- Next: Recreate or validate keyboard shortcuts.
- Next: Recreate or validate file open/save behavior.
- Next: Recreate or validate initial tutorial behavior or document a platform-appropriate replacement.

### Phase 3: iOS/iPadOS Usability

Status: In progress.

- Done: Validate a mobile build from the new project.
- Done: Keep the current compact iOS overlay working.
- Done: Improve iPad layout for larger screens.
- Done: Confirm touch, simulation cycling, and redraw behavior at smoke-test level.
- Done with note: Touch automation used Computer Use against the Simulator UI; `simctl io tap` is unavailable in the current toolchain.
- Done: Exercise the mobile overlay commands (`New Simulation`, `Next Ape`, `Previous Ape`) through the Simulator UI.
- Next: Investigate optional iPad multi-window scene behavior.

### Phase 4: Transition

Status: Not started.

- Compare the new Mac build against `toolchains/sim-mac`.
- Keep both builds until parity is proven.
- Ask the user before removing, archiving, or physically moving old source.

## Immediate Next Steps

1. Continue Mac parity checks beyond launch presence: keyboard shortcuts, mouse/gesture input, file open/save, and initial tutorial behavior.
2. Investigate optional iPad multi-window scene behavior.
3. Decide when to add a true Mac Catalyst target in addition to the native `sim-mac` target and `ApeSim-iOS` target.

## Open Questions

- None blocking right now.
