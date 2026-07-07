# ApeSDK Mac Catalyst Plan

Last updated: 2026-07-07

## Goal

Create a `maccatalyst` home for the next Apple-platform simulation app:

- A full Mac build with the current `sim-mac` functionality, including multiple windows, menu commands, file open/save, and input handling. Initial help/tutorial popovers are intentionally disabled on macOS and Mac Catalyst.
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
- Done: Disabled initial help/tutorial popovers on macOS and future Mac Catalyst paths. The original `sim-mac` project and the `maccatalyst` Mac project no longer define `INITIAL_TUTORIAL_ON`, and `InitialTutorialController` is guarded off on macOS/Mac Catalyst.
- Done: Improved Mac keyboard routing. `CustomDrawingView` now becomes first responder when attached to a window and when clicked, and its letter-key path no longer force-unwraps empty character events.
- Done: Smoke-tested keyboard shortcut routing in the rebuilt `maccatalyst` Mac app. `Command-P` changed the Control menu from `Pause` to `Resume`; letter and left/right arrow key events were sent to the focused View window without a crash.
- Done with note: Improved Mac mouse routing. `CustomDrawingView` now uses one helper for left, modifier, right, other-button, and drag mouse-down paths so the option/right-click flag is set before `shared_mouseReceived`.
- Done with note: Smoke-tested rebuilt `maccatalyst` Mac mouse input with normal click and option-click against the View window. System Events in this environment cannot synthesize drag events, so drag remains source-covered and build-covered but not UI-automation-covered.
- Done: Added a hosted Swift Testing target, `SimMacTests`, to `maccatalyst/maccatalyst.xcodeproj`.
- Done: Added one grouped Swift Testing source file, `maccatalyst/Tests/SimMacParityTests.swift`, covering the Mac functionality inventory with grouped behavioral checks for lifecycle/drawing, windows, panels, menu constants, file routes, keyboard input, mouse/drag/gesture routing, disabled tutorial popovers, and selected-ape find/select/drag/move through the UI event route.
- Done: Expanded the grouped Swift Testing source with tutorial-data checks, all three wrapper view types, mobile draw/cycle wrapper coverage, new simulation/new apes reset coverage, and sandbox temporary save-file creation.
- Done: Stabilized the AppKit window test harness after two attached crash reports from derived `/private/tmp/.../Simulated Ape.app` test runs showed the same main-thread AppKit `_NSWindowTransformAnimation` release crash. The test now inspects launch windows instead of creating and closing extra Terrain/Control windows.
- Done: Added read-only shared test/diagnostic accessors for selected ape followed location and actual selected being location.
- Done: Fixed `command_base_open` so failed/missing opens clear `command_file_interaction`; this unblocked later save calls after a missing-file route.
- Done: Updated the Mac app sandbox entitlement from user-selected read-only to user-selected read-write so the Save As panel is valid for the app's advertised save behavior.
- Done: Performed a visible File menu panel smoke test from the derived `maccatalyst` Mac build. Save As opened a `Save` dialog, Open opened an `Open` dialog, Open Script opened an `Open` dialog, and the app quit cleanly afterward.
- Done: Expanded the sandbox save Swift Testing route into a save/open round trip. The test now saves, mutates the world, reopens the saved file, and verifies the selected ape's location is restored.
- Done with note: Performed a visible user-selected File panel save/open/save pass from a derived `maccatalyst` Mac build. Save As created a non-empty file in `/private/tmp`, Open selected that file through the Open panel, a second Save As created another non-empty file, and the app quit cleanly. The two files were not byte-identical because the running simulation can advance between panel operations; the deterministic read-back assertion remains covered by Swift Testing.
- Done with note: Investigated stronger automated scroll/magnify/rotate gesture event tests. The local AppKit SDK exposes the responder methods and event properties, but synthetic gesture `NSEvent` construction is not reliable in this environment; this remains covered at shared-route smoke level and still needs real-device/manual or better UI automation validation.
- Done: Rebuilt `ApeSim-iOS` for a generic iOS Simulator destination from the `maccatalyst` project. The generated Info.plist has `UIApplicationSupportsMultipleScenes = true` in `UIApplicationSceneManifest`.
- Done: Built the existing `ApeSim-iOS` scheme as Mac Catalyst into `Debug-maccatalyst` from the `maccatalyst` project. The generated Catalyst Info.plist has `UIDeviceFamily = [6]` and `UIApplicationSupportsMultipleScenes = true`.
- Done: Smoke-tested the unsigned Mac Catalyst `ApeSim-iOS` product from `/private/tmp`. It launched as bundle id `com.apesdk.sim-ios`, exposed one `Simulated Ape` window with standard Catalyst menus, and quit cleanly. A signed generic Catalyst build still requires a development team.
- Done: Exercised native Mac Control menu toggle behavior from a derived `/private/tmp` build. `Pause`, `Territory`, `No Weather`, `No Brain`, and `Braincode` accepted clicks and updated to their inverse labels.
- Decision: The user replaced `/Applications/Simulated Ape.app` with the mentioned build, but future process testing should avoid the `/Applications` copy until the app is 100% production/release compilable.
- Decision: Keep the inherited `sim-mac` target and scheme name for now to preserve traceability to the known-good Mac baseline. Revisit renaming after the mobile target is more usable.
- Decision update: Add a dedicated Mac Catalyst target as its own priority after the remaining native Mac command/gesture validation and before any iPad multi-window session implementation. The existing `ApeSim-iOS` target already builds and launches as Mac Catalyst and remains the reference until the dedicated target exists.
- Decision: Do not expose explicit iPad multi-window controls yet. SwiftUI `WindowGroup` and the generated scene manifest already support multiple scenes, but the shared simulation engine is still singleton state; visible iPad multi-window UX should wait for scene-instance simulation/app-shell design or a deliberate decision to share one global simulation across windows.
- Done: Designed the iPad multi-window architecture. The path is to keep the current `WindowGroup` as one shared simulation surface until a scene-owned session handle exists; then explicit iPad multi-window UI can create independent simulation sessions without forking iOS/Mac source.
- Not done: No separate Mac Catalyst-specific target has been added yet; this is now an explicit queued phase before iPad session implementation and final transition work.

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

Initial help/tutorial popover removal:

- `toolchains/sim-mac/sim-mac.xcodeproj` no longer defines `INITIAL_TUTORIAL_ON` in Debug or Release.
- `maccatalyst/maccatalyst.xcodeproj` no longer defines `INITIAL_TUTORIAL_ON` in Debug or Release.
- `toolchains/sim-mac/sim-mac/InitialTutorialController.swift` now returns disabled on macOS and Mac Catalyst so pointer movement cannot show the tutorial popovers.

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

Post-help-popover-removal build commands:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-mac-derived build
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-sim-mac-derived build
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath /private/tmp/apesdk-maccatalyst-ios-derived CODE_SIGNING_ALLOWED=NO build
```

Result on 2026-07-06: all three commands produced `BUILD SUCCEEDED`.

Post-help-popover-removal launch/quit smoke test:

```sh
open -n /private/tmp/apesdk-maccatalyst-mac-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e 'delay 2' -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac") to get {name, name of windows}'
osascript -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac")' \
  -e 'set frontmost to true' \
  -e 'click menu item "Quit Simulated Ape" of menu "Simulated Ape" of menu bar item "Simulated Ape" of menu bar 1' \
  -e 'end tell' \
  -e 'delay 3' \
  -e 'tell application "System Events" to exists (first application process whose bundle identifier is "com.apesdk.sim-mac")'
```

Result: the launched app reported windows `Control`, `Terrain`, `View`. The menu-based quit returned `false` for the remaining `com.apesdk.sim-mac` process check.

Mac keyboard routing pass:

- `toolchains/sim-mac/sim-mac/CustomDrawingView.swift` now calls `window?.makeFirstResponder(self)` when the view moves to a window and when it receives mouse down.
- `CustomDrawingView` now explicitly accepts becoming and resigning first responder.
- `CustomDrawingView.keyDown(with:)` now safely unwraps letter key characters before calling `shared_keyReceived`.

Post-keyboard-routing build commands:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-mac-derived build
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-sim-mac-derived build
```

Result on 2026-07-06: both commands produced `BUILD SUCCEEDED`.

Post-keyboard-routing smoke test:

```sh
open -n /private/tmp/apesdk-maccatalyst-mac-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e 'delay 2' -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac") to get {name, position, size} of windows'
osascript -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac")' \
  -e 'set frontmost to true' \
  -e 'tell window "View" to set {x, y} to position' \
  -e 'tell window "View" to set {w, h} to size' \
  -e 'click at {x + (w div 2), y + (h div 2)}' \
  -e 'delay 0.5' \
  -e 'keystroke "p" using command down' \
  -e 'delay 0.5' \
  -e 'set controlMenuNames to name of menu items of menu "Control" of menu bar item "Control" of menu bar 1' \
  -e 'keystroke "a"' \
  -e 'key code 124' \
  -e 'key code 123' \
  -e 'return controlMenuNames' \
  -e 'end tell'
osascript -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac")' \
  -e 'set frontmost to true' \
  -e 'click menu item "Quit Simulated Ape" of menu "Simulated Ape" of menu bar item "Simulated Ape" of menu bar 1' \
  -e 'end tell' \
  -e 'delay 3' \
  -e 'tell application "System Events" to exists (first application process whose bundle identifier is "com.apesdk.sim-mac")'
```

Observed windows: `Control`, `Terrain`, `View`.

Observed after `Command-P`: the Control menu first item was `Resume`, confirming the pause shortcut fired.

Observed after quit: remaining `com.apesdk.sim-mac` process check returned `false`.

Mac mouse routing pass:

- `toolchains/sim-mac/sim-mac/CustomDrawingView.swift` now routes mouse-down-style events through one helper.
- Normal left click uses the event modifier flags.
- Option/control-click, right-click, other-button click, and their drag variants force the shared option flag before calling `shared_mouseReceived`.
- Mouse-up handling still clears the shared mouse-down/drag state through `shared_mouseUp`.

Post-mouse-routing build commands:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-mac-derived build
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-sim-mac-derived build
```

Result on 2026-07-06: both commands produced `BUILD SUCCEEDED`.

Post-mouse-routing smoke test:

```sh
open -n /private/tmp/apesdk-maccatalyst-mac-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e 'delay 2' -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac") to get {name, position, size} of windows'
osascript -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac")' \
  -e 'set frontmost to true' \
  -e 'tell window "View" to set {x, y} to position' \
  -e 'tell window "View" to set {w, h} to size' \
  -e 'click at {x + (w div 2), y + (h div 2)}' \
  -e 'delay 0.3' \
  -e 'key down option' \
  -e 'click at {x + (w div 2) + 40, y + (h div 2)}' \
  -e 'key up option' \
  -e 'delay 0.3' \
  -e 'keystroke "p" using command down' \
  -e 'delay 0.5' \
  -e 'set controlMenuNames to name of menu items of menu "Control" of menu bar item "Control" of menu bar 1' \
  -e 'return controlMenuNames' \
  -e 'end tell'
osascript -e 'tell application "System Events" to tell (first application process whose bundle identifier is "com.apesdk.sim-mac")' \
  -e 'set frontmost to true' \
  -e 'click menu item "Quit Simulated Ape" of menu "Simulated Ape" of menu bar item "Simulated Ape" of menu bar 1' \
  -e 'end tell' \
  -e 'delay 3' \
  -e 'tell application "System Events" to exists (first application process whose bundle identifier is "com.apesdk.sim-mac")'
```

Observed windows: `Control`, `Terrain`, `View`.

Observed after normal click and option-click: `Command-P` still changed the Control menu first item to `Resume`, confirming the app remained responsive after mouse input.

Observed after quit: remaining `com.apesdk.sim-mac` process check returned `false`.

Drag automation note:

```text
System Events got an error: application process ... does not understand the "drag" message. (-1708)
```

There is no `cliclick` binary available in the current environment. Drag/right-click paths are build-covered by the shared helper, but still need manual or better automation coverage.

Swift Testing parity pass:

- `maccatalyst/maccatalyst.xcodeproj` now has a hosted `SimMacTests` target.
- `maccatalyst/maccatalyst.xcodeproj/xcshareddata/xcschemes/sim-mac.xcscheme` now runs `SimMacTests` from the `sim-mac` scheme Test action.
- `maccatalyst/Tests/SimMacParityTests.swift` is intentionally the only Swift Testing source file for this pass; it groups the roughly hundred-point functionality inventory into six Swift Testing tests.

Command:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-tests-derived test
```

Result on 2026-07-06: `TEST SUCCEEDED`.

Observed Swift Testing results: 6 tests in 1 suite passed, including the selected ape test that finds an ape, selects it, sends mouse down/drag/up events through `CustomDrawingView`, sends an option-modified placement click through the same UI route, and confirms the selected ape's actual coordinates changed.

Entitlement note: this pass changed `toolchains/sim-mac/sim-mac/sim_mac.entitlements` from `com.apple.security.files.user-selected.read-only` to `com.apple.security.files.user-selected.read-write`; without that, the Save As panel could not be configured under the sandboxed app host.

Post-Swift-Testing sanity builds:

```sh
xcodebuild -project toolchains/sim-mac/sim-mac.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-sim-mac-tests-sanity-derived build
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath /private/tmp/apesdk-maccatalyst-ios-tests-sanity-derived CODE_SIGNING_ALLOWED=NO build
```

Result on 2026-07-06: both commands produced `BUILD SUCCEEDED`.

Visible File menu panel smoke test:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-maccatalyst-ui-cycle-derived build
open -n /private/tmp/apesdk-maccatalyst-ui-cycle-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e '...' # System Events clicked File > Save As…, File > Open…, File > Open Script…, then Quit Simulated Ape.
```

Result on 2026-07-07: build produced `BUILD SUCCEEDED`. System Events observed:

```text
windows=ControlTerrainView
fileMenu=New Simulation...Open…Open Script…Save As…
afterSaveAs.windows=SaveControlTerrainView
afterOpen.windows=OpenControlTerrainView
afterOpenScript.windows=OpenControlTerrainView
remaining=false
```

Interpretation: the user-facing File menu items are present, their panels can be shown and canceled in the derived build, and the process exits cleanly through the app menu. This remains a panel smoke test, not an end-to-end file read/write content validation.

Gesture automation investigation:

```sh
xcrun swift -module-cache-path /private/tmp/apesdk-swift-probe-modules -e '...NSEvent scroll/magnify/rotate construction probes...'
```

Result on 2026-07-07: `NSEvent.scrollWheel(...)` is not available under that Swift name in the local SDK, and synthetic `.magnify` / `.rotate` events through `NSEvent.otherEvent(...)` abort with an AppKit internal assertion. Keep gesture behavior covered by the existing shared-route Swift Testing smoke checks until real gesture UI automation or manual validation is available.

Four-cycle pass on 2026-07-07:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-plan-four-cycles-tests test
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -destination 'generic/platform=iOS Simulator' -derivedDataPath /private/tmp/apesdk-plan-four-cycles-iossim build
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -destination 'generic/platform=macOS,variant=Mac Catalyst' -derivedDataPath /private/tmp/apesdk-plan-four-cycles-catalyst CODE_SIGNING_ALLOWED=NO build
```

Results:

- `sim-mac` Swift Testing produced `TEST SUCCEEDED`: 9 tests in 1 suite passed.
- The two crash reports attached during this pass were both from derived `/private/tmp/.../Simulated Ape.app` test runs and had the same main-thread AppKit stack: `objc_release` -> `-[_NSWindowTransformAnimation dealloc]`. This was addressed by changing the AppKit test to inspect the launch Terrain/Control windows rather than creating and closing extra test windows; the full suite passed afterward.
- `ApeSim-iOS` iOS Simulator build produced `BUILD SUCCEEDED`. Product: `/private/tmp/apesdk-plan-four-cycles-iossim/Build/Products/Debug-iphonesimulator/ApeSim-iOS.app`.
- The generated iOS Simulator Info.plist includes `UIApplicationSceneManifest` with `UIApplicationSupportsMultipleScenes = true`, and `UIDeviceFamily = [1, 2]`.
- `ApeSim-iOS` Mac Catalyst build produced `BUILD SUCCEEDED`. Product: `/private/tmp/apesdk-plan-four-cycles-catalyst/Build/Products/Debug-maccatalyst/ApeSim-iOS.app`.
- The generated Catalyst Info.plist includes `UIApplicationSceneManifest` with `UIApplicationSupportsMultipleScenes = true`, and `UIDeviceFamily = [6]`.
- The save route now writes a non-empty sandbox temporary file under test. Root cause found and fixed: `command_base_open` left `command_file_interaction` set after missing-file opens, causing later saves to no-op.

Four-cycle continuation on 2026-07-07:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-plan-cycle-file-roundtrip test
```

Result: `TEST SUCCEEDED`. Observed Swift Testing results: 9 tests in 1 suite passed. The sandbox file test now performs a save/open round trip: it records the selected ape location, saves to a temporary file, mutates the world, reopens the saved file, and verifies the selected ape location is restored.

iPad multi-window decision evidence:

- `ios/ApeSimApp.swift` uses SwiftUI `WindowGroup`.
- The generated iOS Simulator Info.plist from `/private/tmp/apesdk-plan-four-cycles-iossim` sets `UIApplicationSupportsMultipleScenes = true` and `UIDeviceFamily = [1, 2]`.
- The shared engine remains singleton-oriented: `universe/sim.c` has a static `simulated_group group`, and `gui/shared.c` has process-global shared simulation state such as `simulation_started`.
- Decision: do not add explicit iPad multi-window controls until there is a scene-instance simulation/app-shell design, unless the intended UX is explicitly to show the same global simulation in multiple windows.

Mac Catalyst signing and launch check:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -destination 'generic/platform=macOS,variant=Mac Catalyst' -derivedDataPath /private/tmp/apesdk-plan-cycle-catalyst-launch build
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -destination 'generic/platform=macOS,variant=Mac Catalyst' -derivedDataPath /private/tmp/apesdk-plan-cycle-catalyst-launch CODE_SIGNING_ALLOWED=NO build
open -n /private/tmp/apesdk-plan-cycle-catalyst-launch/Build/Products/Debug-maccatalyst/ApeSim-iOS.app
osascript -e 'delay 2' -e 'tell application "System Events" to ...'
osascript -e 'tell application id "com.apesdk.sim-ios" to quit' ...
```

Results:

- Signed generic Mac Catalyst build without an explicit team failed with: `Signing for "ApeSim-iOS" requires a development team`.
- Unsigned generic Mac Catalyst build produced `BUILD SUCCEEDED`.
- Product: `/private/tmp/apesdk-plan-cycle-catalyst-launch/Build/Products/Debug-maccatalyst/ApeSim-iOS.app`.
- `open` launched the product. System Events observed `windows=Simulated Ape` and top-level menus `Apple`, `ApeSim-iOS`, `File`, `Edit`, `View`, `Window`, `Help`.
- Quit by bundle id returned `false` for the remaining `com.apesdk.sim-ios` process check.

Native Mac Control menu behavior smoke test:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-plan-cycle-menu-derived build
open -n /private/tmp/apesdk-plan-cycle-menu-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e '...' # clicked Control menu toggle items
osascript -e 'tell application id "com.apesdk.sim-mac" to quit' ...
```

Result: build produced `BUILD SUCCEEDED`. System Events first observed Control menu items including `Pause`, `Territory`, `No Weather`, `No Brain`, and `Braincode`. Clicking those reversible commands updated the visible labels to `Resume`, `No Territory`, `Weather`, `Brain`, and `No Braincode`. Quit by bundle id returned `false` for the remaining `com.apesdk.sim-mac` process check.

Visible user-selected File panel save/open/save pass on 2026-07-07:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-plan-panel-roundtrip-derived build
open -n /private/tmp/apesdk-plan-panel-roundtrip-derived/Build/Products/Debug/Simulated\ Ape.app
osascript -e '...' # Pause, File > Save As…, select /private/tmp/apesdk-panel-roundtrip.QUGBMk/panel-a.txt
osascript -e '...' # File > New Simulation, File > Open…, select panel-a.txt, File > Save As…, select panel-b.txt
osascript -e 'tell application id "com.apesdk.sim-mac" to quit' ...
```

Result: build produced `BUILD SUCCEEDED`. The first Save As panel created `/private/tmp/apesdk-panel-roundtrip.QUGBMk/panel-a.txt` with size `11948671` bytes. Open selected that file through the visible Open panel and returned to the normal `Terrain`, `Control`, and `View` windows. The second Save As panel created `/private/tmp/apesdk-panel-roundtrip.QUGBMk/panel-b.txt.txt` with size `12121048` bytes. The app quit cleanly, with the remaining `com.apesdk.sim-mac` process check returning `false`.

Interpretation: user-selected Save As, Open, and subsequent Save As work through real panels in the sandboxed derived app. The two saved files were not byte-identical because the simulation can advance between panel operations; the deterministic state read-back check is the Swift Testing save/open round trip, which pauses the world through direct shared calls and verifies the selected ape location after reopen.

Post-architecture-plan verification:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-plan-up-to-ipad-architecture-tests test
```

Result on 2026-07-07: `TEST SUCCEEDED`. Observed Swift Testing results: 9 tests in 1 suite passed.

## Canonical Source Map

- Current Mac app baseline:
  - `toolchains/sim-mac/sim-mac.xcodeproj`
  - `toolchains/sim-mac/sim-mac/SimMacApp.swift`
  - `toolchains/sim-mac/sim-mac/AppDelegate.swift`
  - `toolchains/sim-mac/sim-mac/AppDelegate.swift` now guards app termination and closes the shared simulation.
  - `toolchains/sim-mac/sim-mac/ViewWrapper.swift`
  - `toolchains/sim-mac/sim-mac/CustomDrawingView.swift`
  - `toolchains/sim-mac/sim-mac/CustomDrawingView.swift` now stops drawing/redraw scheduling after termination begins.
  - `toolchains/sim-mac/sim-mac/CustomDrawingView.swift` now claims first responder status on window attach and click so keyboard events route to the simulation view.
  - `toolchains/sim-mac/sim-mac/CustomDrawingView.swift` now sets shared mouse option state before delivering left/modifier/right/other/drag mouse-down events.
  - `toolchains/sim-mac/sim-mac/InitialTutorialController.swift` now disables the initial help/tutorial popovers on macOS and Mac Catalyst.
  - `toolchains/sim-mac/sim-mac/sim_mac.entitlements` now grants user-selected read-write access for Open and Save As panel behavior.
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
  - `sim-mac` has a shared scheme file that includes the hosted `SimMacTests` target in its Test action.
  - `maccatalyst/Tests/SimMacParityTests.swift` is the compact Swift Testing coverage source for Mac parity.
- Shared simulation/rendering/bridge code already used by the Mac target:
  - `gui/shared.c`
  - `gui/shared.c` now defines `shared_draw_ios` and `shared_cycle_ios` for the compact mobile shell.
  - `gui/shared.c` now exposes selected ape followed and actual selected-being location accessors for tests and diagnostics.
  - `gui/draw.c`
  - `gui/gui.h`
  - `sim/`
  - `universe/`
  - `entity/`
  - `toolkit/`
  - `script/`
  - `render/`
  - `shared.h`
  - `universe/command.c` now clears `command_file_interaction` after missing or failed opens so later save/open routes are not blocked.
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
- Done with note: Open simulation file is covered by visible Open panel selection plus Swift Testing save/open read-back.
- Done with note: Open script file panel opens and can be canceled; script content execution through a selected file still needs safe validation if script parity becomes a focus.
- Done with note: Save As is covered by visible user-selected panel saves plus Swift Testing direct save/open read-back.
- About Simulated Ape.
- Done: Pause/Resume and the Territory, Weather, Brain, and Braincode toggles have visible UI smoke coverage through the Control menu.
- Previous Ape and Next Ape.
- Clear Errors.
- Command Line command.
- Online Manual and Simulation Page commands.
- Done: Keyboard input for letters and arrows has a first-responder path and a smoke pass.
- Done: Mouse down/up/drag, right mouse, and option/control-modified mouse share the corrected source path and are covered by Swift Testing event-route checks. External UI automation still covered normal and option clicks only; drag/right-click should get manual or better UI automation coverage when tooling allows.
- Done with note: Scroll wheel, magnify, and rotate shared gesture routes are covered at smoke-test level by Swift Testing. Full human-visible gesture behavior still needs manual or UI automation validation.
- Initial help/tutorial popovers are intentionally disabled on macOS and Mac Catalyst; any future replacement must not interfere with the running simulation.
- Done: App sandbox file entitlement now allows user-selected read-write access for Open and Save As.
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

## iPad Multi-Window Architecture

Decision: keep the current iPad `WindowGroup` as a single shared simulation surface for now. Do not expose an explicit New Window / duplicate simulation command until the simulation can be owned by a scene instance instead of process-global state.

Reasoning:

- SwiftUI `WindowGroup` and the generated iOS/iPadOS scene manifest already permit multiple scenes.
- `ios/ApeSimApp.swift` currently creates `ContentView()` with no scene-owned simulation model, and each `ASiOSView` calls the global `shared_init`, `shared_draw_ios`, `shared_cycle_ios`, `shared_mouseReceived`, and `shared_menu` APIs.
- The C simulation is still singleton-oriented: `universe/sim.c` owns `static simulated_group group` and `static simulated_timing timing`; `sim_group()` and `sim_timing()` return those globals.
- `gui/shared.c` also owns global shell/input/render state such as `simulation_started`, mouse/key state, iOS draw dimensions, output buffer, and menu toggles.
- Opening multiple iPad windows today would therefore show multiple views competing over one global simulation, not independent simulations.

Target architecture before explicit iPad multi-window UI:

1. Add a Swift scene model, tentatively `ApeSimulationSceneModel`, owned by each `WindowGroup` scene. It should own the timer, view identity, render buffer metadata, selected command state, and an opaque C session handle.
2. Extend the existing `gui/app_shell.*` instance helper first. It already models per-shell lifecycle, input, and output-buffer state, so it is the safest bridge between the current global app shell and a later per-simulation session.
3. Introduce a C session API alongside the existing global API, not as a hard replacement:
   - `shared_session_create(random)`
   - `shared_session_destroy(session)`
   - `shared_session_init(session, view, random)`
   - `shared_session_cycle(session, ticks, identification)`
   - `shared_session_draw(session, identification, width, height, size_changed)`
   - `shared_session_menu(session, menu_id)`
   - `shared_session_mouse_received(session, x, y, identification)`
   - `shared_session_mouse_up(session)`
   - `shared_session_open_file(session, path, is_script)`
   - `shared_session_save_file(session, path)`
4. Keep the current `shared_*` functions as wrappers around one default process session until native Mac parity is complete. This avoids breaking `toolchains/sim-mac`, `maccatalyst` Mac, tests, or older wrappers during the migration.
5. Move singleton state in layers:
   - First: shell state already represented by `ape_app_shell` and the iOS render/input buffers.
   - Second: shared/menu state in `gui/shared.c`, including pause and display toggles.
   - Third: simulation core state in `universe/sim.c`, especially `simulated_group group`, `simulated_timing timing`, and any interpreter/session-adjacent state.
6. Prefer explicit context/session pointers over thread-local storage. iPad scenes can share the main thread, so thread-local state would not make scenes independent.
7. Once independent sessions exist, add iPad UI for multi-window behavior. The first user-facing version should create a fresh simulation per new window; duplicating an existing simulation should be a separate, explicit command because it needs save/clone semantics.

Acceptance criteria for exposing explicit iPad multi-window controls:

- Two iPad windows can run side-by-side without sharing selected ape, pause state, terrain rotation, input state, or simulation world mutations.
- Closing one scene releases its shell/session resources without calling `shared_close()` on another scene.
- `New Simulation`, `Next Ape`, and `Previous Ape` affect only the active scene.
- The existing global `shared_*` wrappers continue to pass `SimMacTests` while the transition is underway.
- No canonical source is copied solely to make the iPad fork work.

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
- Done with note: Validate menu command presence and reversible Control menu toggle behavior. Non-reversible commands, dialog commands, and Online commands still need safe visible behavior validation where Swift Testing and existing panel smoke tests do not cover them.
- Done: Validate manual Quit for the rebuilt `maccatalyst` app after shutdown hardening.
- Done by decision: Disable initial help/tutorial popovers on macOS and Mac Catalyst per user request.
- Done: Recreate or validate keyboard shortcuts and focused letter/arrow key routing.
- Done with note: Recreate or validate mouse down/up/drag/right/option routing. Source and build are covered; drag/right-click UI automation remains limited by available tooling.
- Done with note: Add initial Swift Testing coverage for the Mac parity checklist in one grouped test file. This now covers panel construction, tutorial data, command constants, file-route smoke checks, save/open round trip, input/event routing, mobile wrappers, reset commands, and ape selection/drag movement, but does not replace manual UI validation for every visible command.
- Done with note: Recreate or validate file open/save behavior at panel/entitlement and shared-route smoke level. Visible Save As/Open/Open Script panels are UI-smoke-tested, visible user-selected Save As/Open/Save As completed in a derived app, and direct sandbox save/open read-back is Swift-tested. Byte-identical UI-level replay is not treated as a reliable assertion while the simulation is live.
- Done with note: Recreate or validate scroll wheel, magnify, and rotate gestures at shared-route smoke level. Synthetic AppKit gesture event construction is unreliable in this environment, so full visible gesture behavior still needs manual or better UI automation validation.

### Phase 3: iOS/iPadOS Usability

Status: In progress.

- Done: Validate a mobile build from the new project.
- Done: Keep the current compact iOS overlay working.
- Done: Improve iPad layout for larger screens.
- Done: Confirm touch, simulation cycling, and redraw behavior at smoke-test level.
- Done with note: Touch automation used Computer Use against the Simulator UI; `simctl io tap` is unavailable in the current toolchain.
- Done: Exercise the mobile overlay commands (`New Simulation`, `Next Ape`, `Previous Ape`) through the Simulator UI.
- Done: Investigate optional iPad multi-window scene behavior. The generated iOS Simulator Info.plist already sets `UIApplicationSupportsMultipleScenes = true` for the SwiftUI `WindowGroup` app.
- Decision: Do not add explicit iPad multi-window controls yet. The underlying scene manifest supports multiple scenes, but the shared simulation engine currently uses singleton state.
- Done: Designed scene-instance simulation/app-shell isolation before exposing explicit iPad multi-window controls. The accepted direction is a per-scene Swift model plus an opaque C session API, with current `shared_*` calls retained as wrappers around a default process session during migration.
- Next: Keep the current single shared simulation surface until the dedicated Mac Catalyst target phase is complete and explicit iPad multi-window UI becomes a priority.

### Phase 4: Dedicated Mac Catalyst Target

Status: Not started.

- Add a dedicated Mac Catalyst target and scheme under `maccatalyst/maccatalyst.xcodeproj`.
- Use the existing `ApeSim-iOS` Mac Catalyst build as the known-good reference while adding the dedicated target.
- Keep shared simulation, rendering, iOS/iPadOS Swift, and Mac Swift source references canonical; fork only target settings, app-shell files, menus, entitlements, or platform behavior that genuinely must differ for Mac Catalyst.
- Build the dedicated Mac Catalyst target into an explicit `/private/tmp` derived data location.
- Launch the dedicated Mac Catalyst product from `/private/tmp`, verify the expected window/menu surface, and quit it cleanly.
- Do not replace the native `sim-mac` target or the current `ApeSim-iOS` target while this target is being brought up.

### Phase 5: Transition

Status: Not started.

- Compare the new Mac build against `toolchains/sim-mac`.
- Keep both builds until parity is proven.
- Ask the user before removing, archiving, or physically moving old source.

## Immediate Next Steps

1. Add manual or stronger UI automation validation for visible scroll/magnify/rotate behavior.
2. Continue safe visible command validation for non-toggle menu commands and expand `SimMacTests` only where new feedback or regressions identify missing parity assertions.
3. Add the dedicated Mac Catalyst target and scheme as its own phase, keeping existing iOS/iPadOS and native Mac targets in place.
4. If explicit iPad multi-window UI becomes a priority, implement the documented per-scene model and C session API before adding user-facing window controls.
5. Start Phase 5 transition work only after parity is proven: compare against `toolchains/sim-mac`, keep both builds during proof, and ask before removing or moving old source.
6. Define signing/team expectations for Mac Catalyst, or continue using `CODE_SIGNING_ALLOWED=NO` for development smoke builds until release signing is planned.

## Open Questions

- None blocking right now.
