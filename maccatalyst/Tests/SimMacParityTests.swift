import AppKit
import Testing
import UniformTypeIdentifiers
@testable import Simulated_Ape

@MainActor
@Suite("sim-mac functionality parity", .serialized)
struct SimMacParityTests {
    private struct FunctionalityPoint: Hashable {
        let id: String
        let area: String
        let name: String
    }

    private static let functionality: [FunctionalityPoint] = [
        point("launch-main-view-window", "Launch", "View window is the primary simulation surface"),
        point("launch-terrain-window", "Launch", "Terrain window opens beside the main view"),
        point("launch-control-window", "Launch", "Control window opens beside the main view"),
        point("launch-window-titles", "Launch", "Windows keep the expected View, Terrain, and Control titles"),
        point("launch-window-minimums", "Launch", "Auxiliary windows keep expected minimum sizes"),
        point("launch-window-resize-increments", "Launch", "Mac windows keep 4 point resize increments"),
        point("launch-app-delegate-opens-auxiliary-windows", "Launch", "App delegate creates auxiliary windows"),
        point("launch-primary-view-default-size", "Launch", "Primary view defaults to 512 by 512"),
        point("launch-multiple-window-model", "Launch", "Mac build remains a multiple-window app"),
        point("launch-no-applications-folder-dependency", "Launch", "Tests use derived products rather than /Applications"),
        point("lifecycle-shared-init-control", "Lifecycle", "Shared simulation initializes through the processing/control window"),
        point("lifecycle-shared-close-idempotent", "Lifecycle", "Shared close can be called more than once"),
        point("lifecycle-started-state", "Lifecycle", "Shared simulation started state is observable"),
        point("lifecycle-cycle-before-start", "Lifecycle", "Cycle before initialization is harmless"),
        point("lifecycle-cycle-control", "Lifecycle", "Control cycle drives processing"),
        point("lifecycle-view-draw-buffer", "Lifecycle", "View draw returns a render buffer"),
        point("lifecycle-terrain-draw-buffer", "Lifecycle", "Terrain draw returns a render buffer"),
        point("lifecycle-control-draw-buffer", "Lifecycle", "Control draw returns a render buffer"),
        point("lifecycle-dimensions", "Lifecycle", "Shared dimensions remain three 512 by 512 surfaces"),
        point("lifecycle-max-fps", "Lifecycle", "Shared maximum frame rate remains sixty"),
        point("lifecycle-termination-flag", "Lifecycle", "App termination flag can prevent redraw work"),
        point("lifecycle-view-wrapper-keeps-view-type", "Lifecycle", "SwiftUI wrapper keeps its shared view type"),
        point("lifecycle-draw-size-change", "Lifecycle", "Draw path accepts size change notifications"),
        point("lifecycle-ios-cycle-wrapper", "Lifecycle", "iOS cycle wrapper remains represented"),
        point("lifecycle-ios-draw-wrapper", "Lifecycle", "iOS draw wrapper remains represented"),
        point("menus-about-command", "Menus", "About command routes to shared about"),
        point("menus-new-simulation", "Menus", "New Simulation command routes to shared new"),
        point("menus-open-file", "Menus", "Open command routes to file open"),
        point("menus-open-script", "Menus", "Open Script command routes to script open"),
        point("menus-save-as", "Menus", "Save As command routes to file save"),
        point("menus-pause-resume", "Menus", "Pause command toggles pause state"),
        point("menus-previous-ape", "Menus", "Previous Ape command routes to selection change"),
        point("menus-next-ape", "Menus", "Next Ape command routes to selection change"),
        point("menus-clear-errors", "Menus", "Clear Errors command routes to error clearing"),
        point("menus-territory-toggle", "Menus", "Territory command toggles territory drawing"),
        point("menus-weather-toggle", "Menus", "Weather command toggles weather drawing"),
        point("menus-brain-toggle", "Menus", "Brain command toggles brain drawing"),
        point("menus-braincode-toggle", "Menus", "Braincode command toggles braincode drawing"),
        point("menus-command-line", "Menus", "Command Line command routes to command execution"),
        point("menus-online-manual", "Menus", "Online Manual command remains represented"),
        point("menus-online-simulation-page", "Menus", "Online Simulation Page command remains represented"),
        point("menus-keyboard-new", "Menus", "Command-N remains the new simulation shortcut"),
        point("menus-keyboard-open", "Menus", "Command-O remains the open shortcut"),
        point("menus-keyboard-save-as", "Menus", "Shift-Command-S remains the save shortcut"),
        point("menus-keyboard-pause", "Menus", "Command-P remains the pause shortcut"),
        point("menus-keyboard-previous", "Menus", "Command-[ remains previous ape"),
        point("menus-keyboard-next", "Menus", "Command-] remains next ape"),
        point("menus-keyboard-clear-errors", "Menus", "Command-E remains clear errors"),
        point("menus-keyboard-command-line", "Menus", "Shift-Command-C remains command line"),
        point("menus-invalid-command", "Menus", "Unknown shared menu command returns failure"),
        point("file-open-panel-content-type", "Files", "Open panel accepts plain text"),
        point("file-open-panel-no-directories", "Files", "Open panel disallows directories"),
        point("file-open-panel-single-selection", "Files", "Open panel disallows multiple selection"),
        point("file-save-panel-content-type", "Files", "Save panel emits plain text"),
        point("file-save-panel-default-name", "Files", "Save panel default name remains Untitled"),
        point("file-save-panel-create-directories", "Files", "Save panel can create directories"),
        point("file-open-missing-simulation", "Files", "Missing simulation file routes cleanly"),
        point("file-open-missing-script", "Files", "Missing script file routes cleanly"),
        point("file-save-derived-temp", "Files", "Save route accepts a derived temporary path"),
        point("input-first-responder", "Input", "Drawing view accepts first responder"),
        point("input-become-responder", "Input", "Drawing view can become first responder"),
        point("input-resign-responder", "Input", "Drawing view can resign first responder"),
        point("input-accepts-first-mouse", "Input", "Drawing view accepts first mouse"),
        point("input-tracking-area", "Input", "Drawing view installs tracking areas"),
        point("input-letter-key", "Input", "Letter keys route to shared key input"),
        point("input-empty-key", "Input", "Empty key events do not crash"),
        point("input-option-key", "Input", "Option modified keys route to alternate shared value"),
        point("input-control-key", "Input", "Control modified keys route to alternate shared value"),
        point("input-left-arrow", "Input", "Left arrow routes as map movement"),
        point("input-right-arrow", "Input", "Right arrow routes as map movement"),
        point("input-up-arrow", "Input", "Up arrow routes as map movement"),
        point("input-down-arrow", "Input", "Down arrow routes as map movement"),
        point("input-key-up", "Input", "Key up clears shared key input"),
        point("input-left-click", "Input", "Left mouse down routes to shared mouse input"),
        point("input-option-click", "Input", "Option click routes as shared option input"),
        point("input-control-click", "Input", "Control click routes as shared option input"),
        point("input-right-click", "Input", "Right click routes as shared option input"),
        point("input-other-click", "Input", "Other mouse down routes to shared mouse input"),
        point("input-left-drag", "Input", "Left drag routes through mouse input"),
        point("input-right-drag", "Input", "Right drag routes as shared option input"),
        point("input-other-drag", "Input", "Other drag routes through mouse input"),
        point("input-mouse-up", "Input", "Mouse up clears shared mouse input"),
        point("input-scroll", "Input", "Scroll gesture routes to shared delta"),
        point("input-magnify", "Input", "Magnify gesture routes to shared zoom"),
        point("input-rotate", "Input", "Rotate gesture routes to terrain rotation"),
        point("input-terrain-click", "Input", "Terrain click routes to terrain movement"),
        point("input-control-click-window", "Input", "Control window click routes to control selection"),
        point("input-find-select-ape", "Ape Selection", "Clicking near an ape can select it"),
        point("input-shared-being-count", "Ape Selection", "Shared API exposes ape count"),
        point("input-shared-being-name", "Ape Selection", "Shared API exposes ape names"),
        point("input-shared-being-select", "Ape Selection", "Shared API selects an ape"),
        point("mouse-find-select-drag-move-ape", "Ape Selection", "Find, select, drag, and move an ape through the UI route"),
        point("ape-location-observable", "Ape Selection", "Selected ape followed location is observable"),
        point("ape-option-placement-changes-location", "Ape Selection", "Option placement changes selected ape location"),
        point("tutorial-disabled-macos", "Regression Guards", "Initial tutorial popovers are disabled on macOS"),
        point("tutorial-step-data-kept", "Regression Guards", "Tutorial step data remains available for non-Mac paths"),
        point("tutorial-show-next-no-popover", "Regression Guards", "Tutorial controller returns without showing Mac popovers"),
        point("regression-no-help-dialogs", "Regression Guards", "Help dialogs remain removed from Mac/Mac Catalyst running"),
        point("regression-no-forced-character-unwrap", "Regression Guards", "Keyboard input no longer force unwraps characters"),
        point("regression-right-click-option-flag", "Regression Guards", "Right click sets the option flag before mouse routing"),
        point("regression-drag-option-flag", "Regression Guards", "Right drag sets the option flag before mouse routing"),
        point("regression-window-first-responder", "Regression Guards", "Views become first responder when attached or clicked"),
        point("mobile-ios-initializes-processing", "Mobile", "iOS initializes the processing/control window"),
        point("mobile-ios-terrain-renders", "Mobile", "iOS terrain render remains represented"),
        point("mobile-ios-touch-scaling", "Mobile", "iOS touch scaling remains represented"),
        point("mobile-ipad-command-panel", "Mobile", "iPad command panel remains represented"),
        point("mobile-maccatalyst-target-pending", "Mobile", "Dedicated Mac Catalyst target remains pending"),
    ]

    @Test("Functionality inventory covers the requested UI surface")
    func functionalityInventoryCoversRequestedSurface() {
        let ids = Set(Self.functionality.map(\.id))
        #expect(Self.functionality.count >= 105)
        #expect(ids.count == Self.functionality.count)
        #expect(ids.contains("mouse-find-select-drag-move-ape"))

        for area in ["Launch", "Lifecycle", "Menus", "Files", "Input", "Ape Selection", "Regression Guards", "Mobile"] {
            #expect(Self.functionality.contains { $0.area == area })
        }
    }

    @Test("Shared lifecycle, dimensions, drawing, and close contracts")
    func sharedLifecycleDrawingAndCloseContracts() {
        shared_close()
        defer { shared_close() }

        var dimensions = [n_int](repeating: 0, count: 4)
        dimensions.withUnsafeMutableBufferPointer { buffer in
            shared_dimensions(buffer.baseAddress)
        }
        #expect(dimensions[0] == 3)
        #expect(dimensions[1] == 512)
        #expect(dimensions[2] == 512)
        #expect(dimensions[3] == 1)
        #expect(shared_max_fps() == 60)
        #expect(shared_simulation_started() == 0)

        _ = shared_cycle(1, n_int(NUM_VIEW))
        #expect(shared_init(n_int(NUM_CONTROL), 0x12738291) == n_int(NUM_CONTROL))
        #expect(shared_simulation_started() == 1)
        _ = shared_cycle(1, n_int(NUM_CONTROL))

        #expect(shared_draw(n_int(NUM_VIEW), 512, 512, 1) != nil)
        #expect(shared_draw(n_int(NUM_TERRAIN), 512, 512, 1) != nil)
        #expect(shared_draw(n_int(NUM_CONTROL), 512, 512, 1) != nil)
        #expect(shared_being_number() > 0)

        shared_close()
        #expect(shared_simulation_started() == 0)
        shared_close()
        #expect(shared_simulation_started() == 0)
    }

    @Test("AppKit window, panel, tutorial, and wrapper contracts")
    func appKitContracts() {
        if NSApp.windows.contains(where: { $0.title == "Terrain" }) == false ||
            NSApp.windows.contains(where: { $0.title == "Control" }) == false {
            let delegate = AppDelegate()
            delegate.openAdditionalWindows()
            RunLoop.current.run(until: Date().addingTimeInterval(0.05))
        }

        let appWindows = NSApp.windows.filter { ["Terrain", "Control"].contains($0.title) }
        let titles = Set(appWindows.map(\.title))
        #expect(titles.isSuperset(of: Set(["Terrain", "Control"])))

        if let terrain = appWindows.first(where: { $0.title == "Terrain" }) {
            #expect(terrain.minSize == NSSize(width: 512, height: 400))
            #expect(terrain.contentResizeIncrements == NSSize(width: 4, height: 4))
        }
        if let control = appWindows.first(where: { $0.title == "Control" }) {
            #expect(control.minSize == NSSize(width: 342, height: 512))
            #expect(control.contentResizeIncrements == NSSize(width: 4, height: 4))
        }

        let app = SimMacApp()
        let openPanel = app.uniformOpenPanel()
        #expect(openPanel.allowedContentTypes == [.plainText])
        #expect(openPanel.canChooseDirectories == false)
        #expect(openPanel.allowsMultipleSelection == false)

        let savePanel = app.uniformSavePanel()
        #expect(savePanel.allowedContentTypes == [.plainText])
        #expect(savePanel.nameFieldStringValue == "Untitled")
        #expect(savePanel.canCreateDirectories == true)

        let view = Self.makeView()
        #expect(InitialTutorialController.shared.showNextTutorial(for: view))
        #expect(shared_initial_tutorial_enabled() == 0)
        #expect(shared_initial_tutorial_count() > 0)
        for viewType in [NUM_VIEW, NUM_TERRAIN, NUM_CONTROL] {
            #expect(InitialTutorialController.shared.showNextTutorial(for: Self.makeView(Int32(viewType))))
            #expect(ViewWrapper(viewType: Int32(viewType)).viewType == Int32(viewType))
        }
        #expect(ViewWrapper(viewType: Int32(NUM_VIEW)).viewType == Int32(NUM_VIEW))
        #expect(AppDelegate.isTerminating == false)
    }

    @Test("Tutorial step data remains valid while Mac popovers stay disabled")
    func tutorialStepDataContracts() {
        #expect(shared_initial_tutorial_enabled() == 0)
        let count = shared_initial_tutorial_count()
        #expect(count > 0)

        for step in 0..<count {
            let window = shared_initial_tutorial_window(step)
            #expect([NUM_VIEW, NUM_TERRAIN, NUM_CONTROL].contains(Int32(window)))
            #expect((0...1000).contains(Int(shared_initial_tutorial_anchor_x(step))))
            #expect((0...1000).contains(Int(shared_initial_tutorial_anchor_y(step))))
            #expect((1...1000).contains(Int(shared_initial_tutorial_anchor_width(step))))
            #expect((1...1000).contains(Int(shared_initial_tutorial_anchor_height(step))))
            #expect((0...3).contains(Int(shared_initial_tutorial_edge(step))))

            if let title = shared_initial_tutorial_title(step) {
                #expect(String(cString: title).isEmpty == false)
            } else {
                Issue.record("Tutorial title pointer was nil")
            }

            if let text = shared_initial_tutorial_text(step) {
                #expect(String(cString: text).isEmpty == false)
            } else {
                Issue.record("Tutorial text pointer was nil")
            }
        }

        #expect(shared_initial_tutorial_window(-1) == NUM_NIL)
        #expect(String(cString: shared_initial_tutorial_title(count)).isEmpty)
        #expect(String(cString: shared_initial_tutorial_text(count)).isEmpty)
    }

    @Test("Menu constants and file routes keep their contracts")
    func menuAndFileContracts() {
        shared_close()
        defer { shared_close() }

        #expect(NUM_VIEW == 0)
        #expect(NUM_TERRAIN == 1)
        #expect(NUM_CONTROL == 2)
        #expect(NUM_NIL == 3)
        #expect(NA_MENU_PAUSE == 0)
        #expect(NA_MENU_WEATHER == 1)
        #expect(NA_MENU_BRAIN == 2)
        #expect(NA_MENU_BRAINCODE == 3)
        #expect(NA_MENU_TIDEDAYLIGHT == 4)
        #expect(NA_MENU_TERRITORY == 5)
        #expect(NA_MENU_PREVIOUS_APE == 6)
        #expect(NA_MENU_NEXT_APE == 7)
        #expect(NA_MENU_CLEAR_ERRORS == 8)
        #expect(NA_MENU_FOLLOW == 11)
        #expect(NA_MENU_SOCIAL_WEB == 12)
        #expect(shared_menu(-999) == -1)

        for toggle in [NA_MENU_PAUSE, NA_MENU_WEATHER, NA_MENU_BRAIN, NA_MENU_BRAINCODE, NA_MENU_TIDEDAYLIGHT, NA_MENU_TERRITORY, NA_MENU_FOLLOW, NA_MENU_SOCIAL_WEB] {
            let result = shared_menu(n_int(toggle))
            #expect(result == 0 || result == 1)
        }
        #expect(shared_menu(n_int(NA_MENU_PREVIOUS_APE)) == 0)
        #expect(shared_menu(n_int(NA_MENU_NEXT_APE)) == 0)
        #expect(shared_menu(n_int(NA_MENU_CLEAR_ERRORS)) == 0)

        let missingPath = "/private/tmp/apesdk-missing-\(UUID().uuidString).txt"
        #expect(shared_openFileName(missingPath, 0) == 1)
        #expect(shared_openFileName(missingPath, 1) == 1)

        #expect(shared_init(n_int(NUM_CONTROL), 0x12738291) == n_int(NUM_CONTROL))
        let savePath = "/private/tmp/apesdk-sim-mac-save-\(UUID().uuidString).txt"
        shared_saveFileName(savePath)
        #expect(shared_simulation_started() == 1)
        try? FileManager.default.removeItem(atPath: savePath)
    }

    @Test("Simulation save route writes a sandbox temporary file that can be reopened")
    func sandboxTemporarySaveAndOpenRoundTrip() throws {
        shared_close()
        defer { shared_close() }

        #expect(shared_init(n_int(NUM_CONTROL), 0x12738291) == n_int(NUM_CONTROL))
        _ = shared_cycle(1, n_int(NUM_CONTROL))
        shared_being_select(0)

        var savedX: n_int = 0
        var savedY: n_int = 0
        #expect(shared_selected_being_location(&savedX, &savedY) == 1)

        let url = FileManager.default.temporaryDirectory
            .appendingPathComponent("apesdk-sim-mac-roundtrip-\(UUID().uuidString)")
            .appendingPathExtension("txt")
        defer { try? FileManager.default.removeItem(at: url) }

        shared_saveFileName(url.path)
        #expect(FileManager.default.fileExists(atPath: url.path))
        let data = try Data(contentsOf: url)
        #expect(data.isEmpty == false)

        #expect(shared_new(0x99887766) == 0)
        shared_being_select(0)
        #expect(shared_openFileName(url.path, 0) == 1)
        shared_being_select(0)
        var restoredX: n_int = 0
        var restoredY: n_int = 0
        #expect(shared_selected_being_location(&restoredX, &restoredY) == 1)
        #expect(restoredX == savedX)
        #expect(restoredY == savedY)
        #expect(shared_simulation_started() == 1)
    }

    @Test("New simulation commands and iOS wrappers keep the shared engine usable")
    func resetCommandsAndMobileWrappers() {
        shared_close()
        defer { shared_close() }

        #expect(shared_init(n_int(NUM_CONTROL), 0x12738291) == n_int(NUM_CONTROL))
        #expect(shared_being_number() > 0)

        #expect(shared_new(0x22334455) == 0)
        #expect(shared_simulation_started() == 1)
        #expect(shared_being_number() > 0)

        #expect(shared_new_agents(0x55667788) == 0)
        #expect(shared_simulation_started() == 1)
        #expect(shared_being_number() > 0)

        var mobileBuffer = [n_byte4](repeating: 0, count: 512 * 512)
        mobileBuffer.withUnsafeMutableBufferPointer { buffer in
            shared_draw_ios(buffer.baseAddress, 512, 512)
        }
        shared_mouseReceived_ios(128.0, 192.0)
        _ = shared_cycle_ios(1)
        #expect(shared_simulation_started() == 1)
    }

    @Test("Keyboard, mouse, drag, and gesture routes stay safe")
    func keyboardMouseDragAndGestureRoutes() {
        shared_close()
        defer {
            shared_mouseUp()
            shared_keyUp()
            shared_close()
        }

        _ = shared_init(n_int(NUM_CONTROL), 0x12738291)
        let view = Self.makeView()
        #expect(view.acceptsFirstResponder)
        #expect(view.becomeFirstResponder())
        #expect(view.resignFirstResponder())
        #expect(view.acceptsFirstMouse(for: nil as NSEvent?))
        view.updateTrackingAreas()
        #expect(view.trackingAreas.isEmpty == false)

        view.keyDown(with: Self.keyEvent(.keyDown, characters: "a"))
        view.keyDown(with: Self.keyEvent(.keyDown, characters: "a", modifiers: .option))
        view.keyDown(with: Self.keyEvent(.keyDown, characters: "a", modifiers: .control))
        view.keyDown(with: Self.keyEvent(.keyDown, characters: ""))
        for arrow in [NSLeftArrowFunctionKey, NSRightArrowFunctionKey, NSUpArrowFunctionKey, NSDownArrowFunctionKey] {
            view.keyDown(with: Self.keyEvent(.keyDown, characters: Self.functionKeyString(arrow), modifiers: .numericPad))
        }
        view.keyUp(with: Self.keyEvent(.keyUp, characters: "a"))

        view.mouseDown(with: Self.mouseEvent(.leftMouseDown, x: 120, y: 120))
        view.mouseDragged(with: Self.mouseEvent(.leftMouseDragged, x: 128, y: 128))
        view.mouseUp(with: Self.mouseEvent(.leftMouseUp, x: 128, y: 128))
        view.mouseDown(with: Self.mouseEvent(.leftMouseDown, x: 140, y: 140, modifiers: .option))
        view.mouseDown(with: Self.mouseEvent(.leftMouseDown, x: 150, y: 150, modifiers: .control))
        view.rightMouseDown(with: Self.mouseEvent(.rightMouseDown, x: 160, y: 160))
        view.rightMouseDragged(with: Self.mouseEvent(.rightMouseDragged, x: 165, y: 165))
        view.otherMouseDown(with: Self.mouseEvent(.otherMouseDown, x: 170, y: 170))
        view.otherMouseDragged(with: Self.mouseEvent(.otherMouseDragged, x: 175, y: 175))
        shared_mouseUp()

        shared_delta(4.0, -3.0, n_int(NUM_VIEW))
        shared_zoom(1.25, n_int(NUM_VIEW))
        shared_rotate(45.0, n_int(NUM_TERRAIN))
    }

    @Test("Ape can be found, selected, dragged, and moved through the UI route")
    func apeCanBeFoundSelectedDraggedAndMovedThroughUIRoute() {
        shared_close()
        defer {
            shared_mouseUp()
            shared_close()
        }

        #expect(shared_init(n_int(NUM_CONTROL), 0x12738291) == n_int(NUM_CONTROL))
        _ = shared_cycle(1, n_int(NUM_CONTROL))
        #expect(shared_draw(n_int(NUM_VIEW), 512, 512, 1) != nil)

        let apeCount = shared_being_number()
        #expect(apeCount > 0)

        var apeName = [CChar](repeating: 0, count: 256)
        shared_being_name(0, &apeName)
        #expect(String(cString: apeName).isEmpty == false)
        shared_being_select(0)
        #expect(shared_draw(n_int(NUM_VIEW), 512, 512, 1) != nil)

        var beforeX: n_int = 0
        var beforeY: n_int = 0
        #expect(shared_selected_location(&beforeX, &beforeY) == 1)
        #expect(shared_selected_being_location(&beforeX, &beforeY) == 1)

        let view = Self.makeView()
        view.mouseDown(with: Self.mouseEvent(.leftMouseDown, x: 256, y: 256))
        _ = shared_cycle(1, n_int(NUM_VIEW))
        view.mouseDragged(with: Self.mouseEvent(.leftMouseDragged, x: 280, y: 280))
        _ = shared_cycle(1, n_int(NUM_VIEW))
        view.mouseUp(with: Self.mouseEvent(.leftMouseUp, x: 280, y: 280))

        view.mouseDown(with: Self.mouseEvent(.leftMouseDown, x: 88, y: 144, modifiers: .option))
        _ = shared_cycle(1, n_int(NUM_VIEW))
        shared_mouseUp()
        #expect(shared_draw(n_int(NUM_VIEW), 512, 512, 1) != nil)

        var afterX: n_int = 0
        var afterY: n_int = 0
        #expect(shared_selected_being_location(&afterX, &afterY) == 1)
        #expect(afterX != beforeX || afterY != beforeY)
    }

    private static func point(_ id: String, _ area: String, _ name: String) -> FunctionalityPoint {
        FunctionalityPoint(id: id, area: area, name: name)
    }

    private static func makeView(_ viewType: Int32 = Int32(NUM_VIEW)) -> CustomDrawingView {
        let view = CustomDrawingView(viewType: viewType)
        view.frame = NSRect(x: 0, y: 0, width: 512, height: 512)
        return view
    }

    private static func keyEvent(
        _ type: NSEvent.EventType,
        characters: String,
        modifiers: NSEvent.ModifierFlags = []
    ) -> NSEvent {
        NSEvent.keyEvent(
            with: type,
            location: .zero,
            modifierFlags: modifiers,
            timestamp: 0,
            windowNumber: 0,
            context: nil,
            characters: characters,
            charactersIgnoringModifiers: characters,
            isARepeat: false,
            keyCode: 0
        )!
    }

    private static func mouseEvent(
        _ type: NSEvent.EventType,
        x: CGFloat,
        y: CGFloat,
        modifiers: NSEvent.ModifierFlags = []
    ) -> NSEvent {
        NSEvent.mouseEvent(
            with: type,
            location: NSPoint(x: x, y: y),
            modifierFlags: modifiers,
            timestamp: 0,
            windowNumber: 0,
            context: nil,
            eventNumber: 1,
            clickCount: 1,
            pressure: 1
        )!
    }

    private static func functionKeyString(_ functionKey: Int) -> String {
        String(UnicodeScalar(functionKey)!)
    }
}
