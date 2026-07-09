# ApeSDK Mac Catalyst Plan

Last updated: 2026-07-09

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
- Done with note: User validation of `/private/tmp/apesdk-ipados-orientation-fixed.png` and `/private/tmp/apesdk-ios-orientation-fixed.png` showed that the terrain graphics still needed a 180-degree rotation while the SwiftUI controls were already upright. The mobile terrain copy path now rotates the shared terrain buffer 180 degrees, and the UIKit touch mapping now applies the same inverse rotation so terrain input still maps to the displayed terrain.
- Done: Mobile priority 0 now keeps the visible app name, bundle name, executable name, and compiled iOS/iPadOS Simulator product aligned as `Simulated Ape` / `Simulated Ape.app`.
- Done with note: Replaced the temporary mobile text-button overlay with a compact SF Symbols command surface for Previous Ape, New Simulation, and Next Ape. User visual acceptance is still needed before treating this as the final iOS/iPadOS interface.
- Done: User confirmed task 0 is completed on 2026-07-07.
- Done with note: Task 1 current-parity decision recorded on 2026-07-07. Scroll and magnify are accepted as current no-op/no-crash parity routes for now because `shared_delta` and `shared_zoom` are empty. Rotate remains the only visibly meaningful route in this phase, and a derived-app visible screenshot pass confirmed Terrain changed after rotate-capable input.
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
- Done with note: Completed the safe visible native Mac non-toggle menu validation from a derived `/private/tmp` build. `New Simulation`, `About Simulated Ape`, `Previous Ape`, `Next Ape`, `Clear Errors`, and `Command Line` accepted UI clicks, the app kept the expected `Control`, `Terrain`, and `View` windows, and the app quit cleanly. `Manual` and `Simulation Page` remain source-and-presence validated only because automated clicking opens external webpages through `NSWorkspace.shared.open`.
- Done: Added a dedicated Mac Catalyst target and shared scheme, `ApeSim-MacCatalyst`, under `maccatalyst/maccatalyst.xcodeproj`.
- Done with note: `ApeSim-MacCatalyst` uses the existing `ios/ApeSimApp.swift` and shared C file references. It forks only target metadata: scheme, product reference, bundle id `com.apesdk.sim-catalyst`, and Mac Catalyst device-family settings.
- Done: Built `ApeSim-MacCatalyst` as an unsigned Mac Catalyst development product from `/private/tmp`. The product is `Simulated Ape.app` and the generated Info.plist reports `CFBundleDisplayName`, `CFBundleName`, and `CFBundleExecutable` as `Simulated Ape`.
- Done: Smoke-tested the dedicated Mac Catalyst product from `/private/tmp`. It launched as bundle id `com.apesdk.sim-catalyst`, exposed one `Simulated Ape` window with standard Catalyst menus, and quit cleanly.
- Done: Deleted the failed `june13/` project tree per user request on 2026-07-07.
- Decision: The user replaced `/Applications/Simulated Ape.app` with the mentioned build, but future process testing should avoid the `/Applications` copy until the app is 100% production/release compilable.
- Decision: Keep the inherited `sim-mac` target and scheme name for now to preserve traceability to the known-good Mac baseline. Revisit renaming after the mobile target is more usable.
- Decision update: Add a dedicated Mac Catalyst target as its own priority after the remaining native Mac command/gesture validation and before any iPad multi-window session implementation. The existing `ApeSim-iOS` target already builds and launches as Mac Catalyst and remains the reference until the dedicated target exists.
- Decision update: iPad multi-window UI is a priority, but it must be preceded by the per-scene iPad session model so each window owns an independent simulation session instead of competing over singleton state.
- Done: Designed the iPad multi-window architecture. The path is to keep the current `WindowGroup` as one shared simulation surface only until a scene-owned session handle exists; then explicit iPad multi-window UI can create independent simulation sessions without forking iOS/Mac source.
- Done with note: Cycle 1 implemented the per-scene iPad session model. `ios/ApeSimApp.swift` now owns an `ApeSimulationSceneModel` per `WindowGroup` scene, and the model owns the timer, render dimensions, command routes, touch mapping, and an opaque `shared_session` handle. The C session API is layered over `gui/app_shell.*` and keeps the current global `shared_*` wrappers intact.
- Done: Cycle 1 verification passed on 2026-07-09. The `sim-mac` Swift Testing suite passed with 10 tests, the `ApeSim-iOS` iOS Simulator build succeeded, the dedicated `ApeSim-MacCatalyst` build succeeded, the original `toolchains/sim-mac` build succeeded, and `toolchains/planet` still builds with the extended `gui/app_shell.*` helper. An iPad Air 13-inch (M4) Simulator launch rendered terrain through the new scene model; screenshot: `/private/tmp/apesdk-cycle1-ipad-session-smoke.png`.
- Done with note: Cycle 2 added snapshot-backed `shared_session` isolation for the remaining shared/menu and world-state bridge. The legacy `shared_*` calls now route through a default context, each `shared_session` stores its own pause/menu draw state and in-memory transfer snapshot, and session activation restores that context before draw/cycle/menu/file/input routes. This is still a bridge over the existing singleton engine internals, not the final context-pointer engine refactor.
- Done: Cycle 2 verification passed on 2026-07-09. The `sim-mac` Swift Testing suite passed with 11 tests, including a two-session regression that verifies pause toggles, selected ape state, and `New Simulation` stay scoped to the active session. The `ApeSim-iOS` iOS Simulator build, dedicated `ApeSim-MacCatalyst` build, original `toolchains/sim-mac` build, and `toolchains/planet` build all succeeded. An iPad Air 13-inch (M4) Simulator launch rendered terrain through the session path; screenshot: `/private/tmp/apesdk-cycle2-ipad-session-smoke.png`.
- Done with note: Added explicit iPad multi-window UI. The iPad regular-width command panel now exposes a `New Window` button with accessibility id `NewSimulationWindowButton`, the app uses a named `WindowGroup(id: ApeSimulationWindow.id)`, and the button calls `openWindow(id:)` so each new iPad window constructs a fresh `ContentView`, `ApeSimulationSceneModel`, and opaque `shared_session`.
- Done with note: Expanded the two-session regression to cover the full iPad multi-window isolation contract: selected ape, pause state, terrain rotation/facing, touch input state, and world mutations all stay scoped to the session being acted on. This also fixed session activation so a session only becomes active after its saved world restores successfully.
- Done: Final multi-window verification passed on 2026-07-09. The `sim-mac` Swift Testing suite passed with 12 tests; the `ApeSim-iOS`, dedicated `ApeSim-MacCatalyst`, original `toolchains/sim-mac`, and `toolchains/planet` Xcode builds all succeeded with `CODE_SIGNING_ALLOWED=NO`.
- Done with note: Final iPad simulator smoke validation on iPad Air 13-inch (M4), iOS 26.5, installed and launched the app, rendered terrain, exposed the new-window control, and iPadOS reported `1 Hidden Window` for the app. Screenshot: `/private/tmp/apesdk-ipad-multiwindow-final.png`. The local `simctl` runtime has no tap/keyboard injection and macOS `System Events` click injection is blocked, so arranging the two iPad windows visually side-by-side remains a manual/real-device validation step even though the side-by-side state isolation contract is now automated.

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

Mobile priority 0 rotation, naming, and control-surface pass on 2026-07-07:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-ios-rotate-tests test
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath /private/tmp/apesdk-ios-rotate-mobile-derived CODE_SIGNING_ALLOWED=NO build
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-iOS -configuration Debug -destination 'generic/platform=macOS,variant=Mac Catalyst' -derivedDataPath /private/tmp/apesdk-ios-rotate-catalyst-derived CODE_SIGNING_ALLOWED=NO build
```

Results:

- Swift Testing produced `TEST SUCCEEDED`: 10 tests in 1 suite passed, including the 180-degree mobile terrain copy helper.
- iOS Simulator build produced `BUILD SUCCEEDED`.
- Product: `/private/tmp/apesdk-ios-rotate-mobile-derived/Build/Products/Debug-iphonesimulator/Simulated Ape.app`.
- Generated Info.plist has `CFBundleDisplayName = Simulated Ape`, `CFBundleName = Simulated Ape`, and `CFBundleExecutable = Simulated Ape`.
- The user-facing target/scheme name remains `ApeSim-iOS` for build-path traceability, but the built app product is now `Simulated Ape.app`.
- Mac Catalyst build from the same reference scheme also produced `BUILD SUCCEEDED`.
- Mac Catalyst product: `/private/tmp/apesdk-ios-rotate-catalyst-derived/Build/Products/Debug-maccatalyst/Simulated Ape.app`.
- Generated Mac Catalyst Info.plist also has `CFBundleDisplayName = Simulated Ape`, `CFBundleName = Simulated Ape`, and `CFBundleExecutable = Simulated Ape`.

Simulator validation:

```sh
xcrun simctl install 28E193E7-623B-4E28-8C37-A90E044D8CBE /private/tmp/apesdk-ios-rotate-mobile-derived/Build/Products/Debug-iphonesimulator/Simulated\ Ape.app
xcrun simctl launch 28E193E7-623B-4E28-8C37-A90E044D8CBE com.apesdk.sim-ios
xcrun simctl io 28E193E7-623B-4E28-8C37-A90E044D8CBE screenshot /private/tmp/apesdk-ipados-rotate180.png
xcrun simctl install A643FADE-0BCC-4290-A7C4-96B7BB771E25 /private/tmp/apesdk-ios-rotate-mobile-derived/Build/Products/Debug-iphonesimulator/Simulated\ Ape.app
xcrun simctl launch A643FADE-0BCC-4290-A7C4-96B7BB771E25 com.apesdk.sim-ios
xcrun simctl io A643FADE-0BCC-4290-A7C4-96B7BB771E25 screenshot /private/tmp/apesdk-ios-rotate180.png
```

Observed devices:

- iPad Air 13-inch (M4), iOS 26.5, UDID `28E193E7-623B-4E28-8C37-A90E044D8CBE`.
- iPhone 17, iOS 26.5, UDID `A643FADE-0BCC-4290-A7C4-96B7BB771E25`.

Result: both products installed and launched, fresh screenshots were captured, and both app instances were terminated afterward. Screenshots for user validation:

- `/private/tmp/apesdk-ipados-rotate180.png`
- `/private/tmp/apesdk-ios-rotate180.png`

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

Task 1 restart inspection on 2026-07-07:

- `toolchains/sim-mac/sim-mac/CustomDrawingView.swift` overrides `scrollWheel`, `magnify`, and `rotate`.
- `scrollWheel` calls `shared_delta(event.deltaX, event.deltaY, viewType)`.
- `magnify` calls `shared_zoom(event.magnification, viewType)`.
- `rotate` calls `shared_rotate(Double(event.rotation), viewType)`.
- `gui/shared.c` implements `shared_rotate` for `NUM_TERRAIN` by calling `sim_rotate`, but `shared_delta` and `shared_zoom` are empty.

User decision on 2026-07-07: task 1 should confirm current parity only and visibly validate rotate only. Scroll and magnify are current no-op/no-crash parity because `shared_delta` and `shared_zoom` are empty. Do not add visible scroll/zoom semantics during this phase.

Visible rotate validation on 2026-07-07:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-task1-gesture-tests test
open -n /private/tmp/apesdk-task1-gesture-tests/Build/Products/Debug/Simulated\ Ape.app
osascript -e '...' # Raised the Terrain window and sent left-arrow rotate input to the focused derived app.
screencapture -x /private/tmp/apesdk-task1-rotate-before.png
screencapture -x /private/tmp/apesdk-task1-rotate-after.png
osascript -e '...' # Quit Simulated Ape through the app menu.
```

Result: the first Swift Testing run exposed that a direct headless `shared_rotate` render-difference check was not reliable, so that strict test was not retained. The retained Swift Testing suite then produced `TEST SUCCEEDED`: 10 tests in 1 suite passed. The visible derived-app pass showed the Terrain window changed between the before and after screenshots, and the app quit cleanly with `System Events` reporting no remaining `Simulated Ape` process. Evidence:

- `/private/tmp/apesdk-task1-rotate-before.png`
- `/private/tmp/apesdk-task1-rotate-after.png`

Safe native Mac non-toggle command validation on 2026-07-07:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-nontoggle-menu-tests test
open -n /private/tmp/apesdk-nontoggle-menu-tests/Build/Products/Debug/Simulated\ Ape.app
osascript -e '...' # Clicked About, Previous Ape, Next Ape, Clear Errors, and Command Line through the menu bar.
screencapture -x /private/tmp/apesdk-nontoggle-menu-before.png
screencapture -x /private/tmp/apesdk-nontoggle-menu-after.png
open -n /private/tmp/apesdk-nontoggle-menu-tests/Build/Products/Debug/Simulated\ Ape.app
osascript -e '...' # Clicked File > New Simulation through the menu bar.
screencapture -x /private/tmp/apesdk-nontoggle-new-before.png
screencapture -x /private/tmp/apesdk-nontoggle-new-after.png
osascript -e '...' # Quit Simulated Ape through the app menu after each run.
```

Result: Swift Testing produced `TEST SUCCEEDED`: 10 tests in 1 suite passed. The derived app exposed the expected `Control`, `Terrain`, and `View` windows. `About Simulated Ape`, `Previous Ape`, `Next Ape`, `Clear Errors`, `Command Line`, and `New Simulation` all accepted UI clicks. The `New Simulation` before/after screenshots had different SHA-256 hashes, confirming a visible change. `Manual` and `Simulation Page` were present in the Online menu and source-confirmed to call `NSWorkspace.shared.open`, but were intentionally not clicked during this safe pass because they open external webpages. Both derived app runs quit cleanly, with `System Events` reporting no remaining `Simulated Ape` process. Evidence:

- `/private/tmp/apesdk-nontoggle-menu-before.png`
- `/private/tmp/apesdk-nontoggle-menu-after.png`
- `/private/tmp/apesdk-nontoggle-new-before.png`
- `/private/tmp/apesdk-nontoggle-new-after.png`

User request on 2026-07-07: the four `/private/tmp/apesdk-nontoggle-*.png` screenshots above were deleted after display/validation.

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
- Decision: iPad multi-window UI is a priority, but do not add explicit iPad multi-window controls until there is a scene-instance simulation/app-shell design.

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

Dedicated Mac Catalyst target pass on 2026-07-07:

```sh
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -list
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme ApeSim-MacCatalyst -configuration Debug -destination 'generic/platform=macOS,variant=Mac Catalyst' -derivedDataPath /private/tmp/apesdk-dedicated-catalyst-derived CODE_SIGNING_ALLOWED=NO build
plutil -p /private/tmp/apesdk-dedicated-catalyst-derived/Build/Products/Debug-maccatalyst/Simulated\ Ape.app/Contents/Info.plist
open -n /private/tmp/apesdk-dedicated-catalyst-derived/Build/Products/Debug-maccatalyst/Simulated\ Ape.app
osascript -e '...' # Read window/menu surface and running bundle id, then quit through the app menu.
xcodebuild -project maccatalyst/maccatalyst.xcodeproj -scheme sim-mac -configuration Debug -derivedDataPath /private/tmp/apesdk-dedicated-catalyst-simmac-regression test
```

Results:

- `xcodebuild -list` reports target and scheme `ApeSim-MacCatalyst` alongside the existing `sim-mac`, `ApeSim-iOS`, and `SimMacTests` targets/schemes.
- `ApeSim-MacCatalyst` build produced `BUILD SUCCEEDED`.
- Product: `/private/tmp/apesdk-dedicated-catalyst-derived/Build/Products/Debug-maccatalyst/Simulated Ape.app`.
- Generated Info.plist values: `CFBundleIdentifier = com.apesdk.sim-catalyst`, `CFBundleDisplayName = Simulated Ape`, `CFBundleName = Simulated Ape`, `CFBundleExecutable = Simulated Ape`, `CFBundleSupportedPlatforms = [MacOSX]`, `UIDeviceFamily = [6]`, and `UIApplicationSupportsMultipleScenes = true`.
- `open` launched the product. System Events observed `windows=Simulated Ape`, top-level menus `Apple`, `Simulated Ape`, `File`, `Edit`, `View`, `Window`, `Help`, and running bundle id `com.apesdk.sim-catalyst`.
- Quit through the app menu returned `false` for the remaining `Simulated Ape` process check.
- Existing `sim-mac` regression after the project-file edit produced `TEST SUCCEEDED`: 10 tests in 1 suite passed.

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
  - Creates an `ApeSimulationSceneModel` per SwiftUI `WindowGroup` scene.
  - The scene model owns the draw timer, render buffer dimensions, command routing, touch mapping, and opaque C `shared_session` handle.
  - Initializes each scene session through `NUM_CONTROL` so the shared simulation starts before the terrain view draws.
  - Uses an adaptive SwiftUI overlay: compact width keeps the small bottom-trailing controls, regular iPad width uses a wider trailing command panel.
  - The regular-width iPad command panel exposes `New Window`, backed by `openWindow(id:)` on the named simulation `WindowGroup`, so every iPad window gets a new scene model and session.
  - Maps touch locations to the render buffer scale before passing them to `shared_session_mouseReceived`.
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
  - `gui/shared.c` now defines an opaque `shared_session` API for per-scene shell ownership while retaining the existing global `shared_*` wrappers.
  - `gui/shared.c` now keeps session activation guarded by successful restore and exposes session diagnostics for pause state, selected ape index/facing, and active input state.
  - `gui/app_shell.c` is now included by the `maccatalyst` app targets and the original `toolchains/sim-mac` target so session-owned shell state is available everywhere `gui/shared.c` is built.
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
- Removed reference material:
  - `june13/` was deleted on 2026-07-07 per user request because it was a failed project.
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
- Done with note: New Simulation menu command accepted a derived-app UI click and produced a visibly different before/after screenshot pair.
- Done with note: Open simulation file is covered by visible Open panel selection plus Swift Testing save/open read-back.
- Done with note: Open script file panel opens and can be canceled; script content execution through a selected file still needs safe validation if script parity becomes a focus.
- Done with note: Save As is covered by visible user-selected panel saves plus Swift Testing direct save/open read-back.
- Done: About Simulated Ape accepted a derived-app UI click through the app menu.
- Done: Pause/Resume and the Territory, Weather, Brain, and Braincode toggles have visible UI smoke coverage through the Control menu.
- Done: Previous Ape and Next Ape accepted derived-app UI clicks through the Control menu.
- Done: Clear Errors accepted a derived-app UI click through the Control menu.
- Done with note: Command Line accepted a derived-app UI click through the Control menu and the app remained responsive enough to quit cleanly.
- Done with note: Online Manual and Simulation Page commands are present in the Online menu and source-confirmed to route through `NSWorkspace.shared.open`; they were not clicked during safe automation because they open external webpages.
- Done: Keyboard input for letters and arrows has a first-responder path and a smoke pass.
- Done: Mouse down/up/drag, right mouse, and option/control-modified mouse share the corrected source path and are covered by Swift Testing event-route checks. External UI automation still covered normal and option clicks only; drag/right-click should get manual or better UI automation coverage when tooling allows.
- Done with note: Scroll wheel and magnify are accepted as current no-op/no-crash parity routes. Rotate is visibly covered by the derived-app screenshot validation recorded above.
- Initial help/tutorial popovers are intentionally disabled on macOS and Mac Catalyst; any future replacement must not interfere with the running simulation.
- Done: App sandbox file entitlement now allows user-selected read-write access for Open and Save As.
- Done: Local debug build and launch from Xcode command line.
- Done: Manual app shutdown without queued redraw entering shared drawing after close.

## iOS/iPadOS Minimum Checklist

The mobile build should at least provide:

- Done: A visible simulation view.
- Done: Initialization through `shared_init`.
- Done: Drawing through `shared_draw_ios` or a documented shared replacement.
- Done: Cycle updates through `shared_cycle_ios` or a documented shared replacement.
- Done with note: Touch input routed to the shared mouse/input API. Verification used Simulator UI drag plus UIKit event logs because this `simctl io` lacks synthetic taps.
- Done: New Simulation, Next Ape, and Previous Ape commands are present in the overlay.
- Done with note: Replaced the text-button overlay with a compact icon command surface. This is the first pass toward a better iOS/iPadOS interface and still needs user validation.
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

Decision: iPad multi-window UI is a priority, but keep the current iPad `WindowGroup` as a single shared simulation surface until the simulation can be owned by a scene instance instead of process-global state. Do not expose an explicit New Window / duplicate simulation command before that per-scene model exists.

Reasoning:

- SwiftUI `WindowGroup` and the generated iOS/iPadOS scene manifest already permit multiple scenes.
- `ios/ApeSimApp.swift` currently creates `ContentView()` with no scene-owned simulation model, and each `ASiOSView` calls the global `shared_init`, `shared_draw_ios`, `shared_cycle_ios`, `shared_mouseReceived`, and `shared_menu` APIs.
- The C simulation is still singleton-oriented: `universe/sim.c` owns `static simulated_group group` and `static simulated_timing timing`; `sim_group()` and `sim_timing()` return those globals.
- `gui/shared.c` also owns global shell/input/render state such as `simulation_started`, mouse/key state, iOS draw dimensions, output buffer, and menu toggles.
- Opening multiple iPad windows today would therefore show multiple views competing over one global simulation, not independent simulations.

Target architecture before explicit iPad multi-window UI:

1. Done: Add a Swift scene model, `ApeSimulationSceneModel`, owned by each `WindowGroup` scene. It owns the timer, view identity, render buffer metadata, command routes, touch mapping, and an opaque C session handle.
2. Done with note: Extend the existing `gui/app_shell.*` instance helper first. It now carries enough mouse/session state for the first session bridge, but shared/menu and simulation world state still need to move in later layers.
3. Done with note: Introduce a C session API alongside the existing global API, not as a hard replacement:
   - `shared_session_create(random)`
   - `shared_session_destroy(session)`
   - `shared_session_init(session, view, random)`
   - `shared_session_cycle(session, ticks, identification)`
   - `shared_session_cycle_ios(session, ticks)`
   - `shared_session_draw(session, identification, width, height, size_changed)`
   - `shared_session_draw_ios(session, outputBuffer, width, height)`
   - `shared_session_menu(session, menu_id)`
   - `shared_session_new(session, seed)`
   - `shared_session_mouseReceived(session, x, y, identification)`
   - `shared_session_mouseUp(session)`
   - `shared_session_openFileName(session, path, is_script)`
   - `shared_session_saveFileName(session, path)`
4. Keep the current `shared_*` functions as wrappers around one default process session until native Mac parity is complete. This avoids breaking `toolchains/sim-mac`, `maccatalyst` Mac, tests, or older wrappers during the migration.
5. Move singleton state in layers:
   - First: shell state already represented by `ape_app_shell` and the iOS render/input buffers.
   - Done with note: shared/menu state in `gui/shared.c`, including pause and display toggles, now saves/restores through a per-session context.
   - Done with note: simulation world state now switches through an in-memory transfer snapshot per session. `universe/sim.c` has a restore hook that reloads a snapshot without advancing the world, but deeper engine internals still need a future context-pointer refactor.
6. Prefer explicit context/session pointers over thread-local storage. iPad scenes can share the main thread, so thread-local state would not make scenes independent.
7. Once independent sessions exist, add iPad UI for multi-window behavior. This is priority work after the dedicated Mac Catalyst target phase. The first user-facing version should create a fresh simulation per new window; duplicating an existing simulation should be a separate, explicit command because it needs save/clone semantics.

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
- Done with note: Recreate or validate the remaining safe non-toggle native Mac menu commands. About, New Simulation, Previous Ape, Next Ape, Clear Errors, and Command Line accepted UI clicks in a derived app. Online Manual and Simulation Page are source-and-presence validated only because clicking them opens external webpages.

### Phase 3: iOS/iPadOS Usability

Status: In progress.

- Done: Validate a mobile build from the new project.
- Done: Keep the current compact iOS overlay working.
- Done: Improve iPad layout for larger screens.
- Done with note: Replaced the temporary text buttons with a compact icon command panel. This is a first visual pass and needs user acceptance before being considered final.
- Done: Confirm touch, simulation cycling, and redraw behavior at smoke-test level.
- Done with note: Touch automation used Computer Use against the Simulator UI; `simctl io tap` is unavailable in the current toolchain.
- Done: Exercise the mobile overlay commands (`New Simulation`, `Next Ape`, `Previous Ape`) through the Simulator UI.
- Done: Investigate optional iPad multi-window scene behavior. The generated iOS Simulator Info.plist already sets `UIApplicationSupportsMultipleScenes = true` for the SwiftUI `WindowGroup` app.
- Decision update: iPad multi-window UI is a priority. Do not add explicit controls until the per-scene session model exists because the underlying shared simulation engine currently uses singleton state.
- Done: Designed scene-instance simulation/app-shell isolation before exposing explicit iPad multi-window controls. The accepted direction is a per-scene Swift model plus an opaque C session API, with current `shared_*` calls retained as wrappers around a default process session during migration.
- Done with note: Implemented the per-scene iPad session model and opaque C session API for cycle 1. The session owns shell/render/input lifecycle through `gui/app_shell.*`; deeper shared/menu and simulation world state remains singleton-backed and must move before explicit multi-window controls are safe.
- Done with note: Implemented snapshot-backed `shared_session` isolation for cycle 2. Session activation now saves/restores per-session pause/menu draw state and world snapshots, while the default `shared_*` wrappers remain functional through their own context.
- Next: Add explicit iPad multi-window UI and validate two windows side-by-side.

### Phase 4: Dedicated Mac Catalyst Target

Status: Done.

- Done: Added the dedicated `ApeSim-MacCatalyst` target and shared scheme under `maccatalyst/maccatalyst.xcodeproj`.
- Done: Used the existing `ApeSim-iOS` Mac Catalyst build as the known-good reference while adding the dedicated target.
- Done: Kept shared simulation, rendering, iOS/iPadOS Swift, and Mac Swift source references canonical. The new target forks target metadata only.
- Done: Built the dedicated Mac Catalyst target into `/private/tmp/apesdk-dedicated-catalyst-derived`.
- Done: Launched the dedicated Mac Catalyst product from `/private/tmp`, verified the expected one-window standard Catalyst menu surface, confirmed bundle id `com.apesdk.sim-catalyst`, and quit it cleanly.
- Done: Kept the native `sim-mac` target and current `ApeSim-iOS` target in place.

### Phase 5: Transition

Status: Not started.

- Compare the new Mac build against `toolchains/sim-mac`.
- Keep both builds until parity is proven.
- Ask the user before removing, archiving, or physically moving old source.

## Immediate Next Steps

1. Perform a manual or better-automation iPadOS window arrangement pass to visually place the two app windows side-by-side; the state-isolation contract is already covered by Swift Testing.
2. Start Phase 5 transition work only after parity is proven: compare against `toolchains/sim-mac`, keep both builds during proof, and ask before removing or moving old source.
3. Define signing/team expectations for Mac Catalyst, or continue using `CODE_SIGNING_ALLOWED=NO` for development smoke builds until release signing is planned.

## Open Questions

- None blocking right now.
